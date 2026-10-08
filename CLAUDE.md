# CLAUDE.md — working on the tspp compiler

## What tspp is

tspp is a **compiler** from a TypeScript-like syntax to LLVM IR, written in C++17
against the LLVM C++ API. Positioning: *TypeScript syntax with C++-level memory
control, compiled to native via LLVM. Safe by default, unsafe when you ask for it.*
The driver emits LLVM IR into a private temp dir, checks it with `llvm-as`, links
it with `clang` against the runtime (`libcore.a` next to the `tspp` binary), Boehm
GC (`-lgc`) and libcurl, then runs the binary and exits with its status.
It is **not an interpreter**. The REPL (`tspp` with no args) recompiles each chunk
as a fresh program; the `JITInterpreter` it constructs is never used — it is not a
working JIT. Docs say LLVM 17+; the local toolchain is **LLVM 23.1.1** (`/usr/bin`)
and must keep building. Status of features: `docs/KNOWLEDGE_TRANSFER.md` §1.2 and
`docs/MISSING_FEATURES_REPORT.md` (accurate). Bug IDs (C1, H3, …): KNOWLEDGE_TRANSFER §9–10.
The work plan is `docs/STABILIZATION_PLAN.md`.

## Pipeline (where each stage lives)

| Stage | Code |
|---|---|
| Lexer | `src/lexer/` (`lexer.cpp`, `scanner/token_scanner.cpp`, keywords + `#attr` in `scanner/specialized/identifier_scanner.cpp`); tokens in `src/tokens/` |
| Parser / AST builders | `src/parser/parser.cpp` → `visitors/ast_builder/ast_builder_visitor.cpp` (top level) → `ast_builders/{Declaration,Statement,Expression,Type}Builder.cpp`; nodes in `src/parser/nodes/` |
| Imports | `src/parser/module/ModuleResolver.cpp` (textual AST merge, no namespaces) |
| Semantic | `src/parser/visitors/semantic/SemanticAnalyzerVisitor.cpp` (+ `SemanticScope`) |
| Codegen | `src/codegen/LLVMCodeGenerator.cpp` (~5k lines), `ConstantExpressionEvaluator.cpp` |
| Driver | `src/main.cpp` (CLI; runs `llvm-as`, `clang`, the program via `llvm::sys::ExecuteAndWait`, no shell) |
| Diagnostics | `src/core/diagnostics/error_reporter.*`: the single sink `core::diagnostics()` used by every stage (GCC-style `file:line:col: error: ...`); parser helpers in `ast_builders/ParseDiagnostics.h` |
| Runtime | `src/core/syscall/syscall_wrapper.cpp` (`extern "C" tspp_*`), built into `libcore.a` |

## Build and run (verified)

Prerequisites: CMake ≥ 3.14, C++17 compiler, LLVM dev package (`LLVMConfig.cmake`,
shared `libLLVM`), `clang` + `llvm-as` on `PATH`, Boehm GC (`libgc`), libcurl dev.

```bash
./build.sh                         # rm -rf build; configure; build; tspp --version
./build/src/tspp file.tspp         # compile + link + run from any directory; exits
                                   # with the program's status
./build/src/tspp -o prog file.tspp # build only;  --emit-llvm [-o f.ll]: IR only
./build/src/tspp --help            # also --keep-temps, --asan, `-- args` for the program
./test.sh file.tspp                # thin wrapper around build/src/tspp
git checkout compile_commands.json # build.sh overwrites the committed copy; don't commit it
```

Tests (see `tests/README.md`):

```bash
tests/run.sh                       # full suite; prints PASS/FAIL/XFAIL/XPASS summary,
                                   # exits non-zero on FAIL or XPASS
tests/run.sh regression            # only tests whose path contains "regression"
(cd build && ctest --output-on-failure)  # same suite via CTest
```

A test is `<name>.tspp` + `<name>.expected` (stdout; optional `// exit: N`), or has a
`// test: parse-only` / `// test: compile-error` header. `tests/driver/*.sh` are
driver tests (shell injection, other CWD, link failure, CLI). `<name>.xfail` (reason + bug ID) marks a known
bug; when your fix makes it pass (XPASS), delete the `.xfail` file.

Silence leak noise from the ASan-built compiler/programs:
`export LSAN_OPTIONS="suppressions=$(pwd)/lsan.supp:report_objects=0:exitcode=0"`

The driver writes nothing into the CWD (except `-o` / `--emit-llvm` output),
finds `clang`/`llvm-as` on PATH (overrides: `TSPP_CLANG`, `TSPP_LLVM_AS`) and the
runtime via `TSPP_RUNTIME_LIB` or next to its own executable. Exit codes: 0 ok,
1 compile/build failure (the failing stage is named on stderr), 2 usage error; in
run mode, the program's own status.

## Working rules for Claude sessions

- Read the relevant files before editing. **Diagnose before fixing**: state the
  root cause with `file:line` evidence first.
- **One phase** of `docs/STABILIZATION_PLAN.md` per session/branch. Do not fix
  unrelated issues you notice; list them at the end instead.
- Never edit a test's expected output to make it pass unless the expected output
  is provably wrong; say so explicitly if you do.
- `LLVMCodeGenerator.cpp` is ~5k lines (`visit(BinaryExpr&)` and `visit(CallExpr&)`
  are ~1.1k each): `grep -n` for the function, read only that range, keep diffs minimal.
- Every fix needs a test under `tests/` that fails before and passes after.
- Run the full test suite (`tests/run.sh`) before declaring done.
- Don't commit build artifacts (`build/`, `*.ll`).

## Known traps

- **Types are inferred by string-matching names in codegen** (`"string"`,
  `"ClassName.method"`, `Symbol::typeName` strings). Changes ripple unpredictably.
  `boolean` is not mapped in `getLLVMType` and falls back to `ptr` (H4); `bool` works.
- Codegen (`emitEntryPoint`) renames a parameterless user `main` to
  `__tspp_user_main` and emits an `i32 main()` that calls `tspp_runtime_init()`
  (GC_INIT), then `__tspp_global_ctors()` (if there is startup code), then the
  user main (void → exit 0); a missing `main` gets one that only does the two
  init calls and returns 0; a `main` with parameters gets both calls at the top
  of its entry block. Prefer `function main(): int` in tests.
- Startup code (`emitModuleInit`, C2 fixed): non-constant global initializers,
  class `#static` field initializers and `static {}` blocks, and top-level
  statements all run in `__tspp_global_ctors`, in source order; imported
  modules' code comes first (ModuleResolver prepends them). Constant
  initializers are folded into the global. Order of top-level items is kept in
  `ProgramNode::topLevel`.
- **The build forces Debug + ASan** (root `CMakeLists.txt` sets
  `CMAKE_BUILD_TYPE Debug` and `-fsanitize=address`; `-DCMAKE_BUILD_TYPE=...` is
  overridden) of the compiler. Compiled programs are PIE and not ASan-instrumented
  unless `tspp --asan`; `libcore` is built with `-fno-sanitize=address` for that.
  Expect LeakSanitizer output from the compiler on stderr; compare stdout only.
- Every `const char*` the runtime returns to tspp code is a fresh GC-owned copy
  (`GC_MALLOC_ATOMIC`; C1/C1b fixed). Never free it; `tspp_free_string` is a no-op.
  A string pointer stored only in non-GC memory (e.g. `sys.malloc`) is invisible to
  the collector.
- Diagnostics (H1/H6 fixed): lexer, parser, import, semantic and codegen errors
  all go through `core::diagnostics()` with a location; the driver stops after
  the first stage that reported an error and exits 1. The parser reports
  "expected X, found Y" and resynchronizes at the next statement (`finishListItem`,
  `synchronize`). Report new errors through the sink, never `std::cerr`.
  After codegen `llvm::verifyModule` runs; invalid IR is an *internal compiler
  error* (a codegen bug), never patched up. Each function must terminate its own
  blocks (`finishFunction`). Codegen still has silent `lastValue = nullptr`
  paths, so "exit 0" proves little — check the output.
- The lexer inserts a `;` token (lexeme `"\n"`, see `isImplicitSemicolon`) at line
  ends, except after a token that cannot end a statement (`,` `(` operators) and
  before a line starting with `)` `]` `.` or a binary operator.
- Operator precedence (H3 fixed) is the TS/C table, named levels in
  `ExpressionBuilder.h`; `docs/grammar.md` and
  `tests/regression/h3_precedence_table.tspp` follow it.
- Many docs (README, `docs/architecture.md`, `docs/language_spec.md`) overstate
  features (H7). Trust the code, `KNOWLEDGE_TRANSFER.md`, and `MISSING_FEATURES_REPORT.md`.
