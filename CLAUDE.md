# CLAUDE.md — working on the tspp compiler

## What tspp is

tspp is a **compiler** from a TypeScript-like syntax to LLVM IR, written in C++17
against the LLVM C++ API. Positioning: *TypeScript syntax with C++-level memory
control, compiled to native via LLVM. Safe by default, unsafe when you ask for it.*
The driver emits LLVM IR into a private temp dir, checks it with `llvm-as`, links
it with `clang` against the runtime (`libcore.a` next to the `tspp` binary), Boehm
GC (`-lgc`) and libcurl, then runs the binary and exits with its status.
It is **not an interpreter** and has no JIT. The REPL (`tspp` with no args,
`runRepl` in `src/main.cpp`) compiles, links and runs each chunk (a line, or lines
until braces balance) as a separate program through the normal driver path;
nothing carries over between chunks (H2). Docs say LLVM 17+; the local toolchain is **LLVM 23.1.1** (`/usr/bin`)
and must keep building. Status of features: README "Works today" (each row links a passing test),
`docs/KNOWLEDGE_TRANSFER.md` §1.2. Bug IDs (C1, H3, …): KNOWLEDGE_TRANSFER §9
(new ones in §9.7). What comes next: `ROADMAP.md`. The finished stabilization
plan: `docs/STABILIZATION_PLAN.md`.

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
./build.sh                         # Debug (ASan) build in build/; incremental
./build.sh --release               # Release (no sanitizers) in build-release/
                                   # --clean: wipe that dir first; -- -DTSPP_ENABLE_UBSAN=ON
./build/src/tspp file.tspp         # compile + link + run from any directory; exits
                                   # with the program's status
./build/src/tspp -o prog file.tspp # build only;  --emit-llvm [-o f.ll]: IR only
./build/src/tspp --help            # also --keep-temps, --asan, `-- args` for the program
./test.sh file.tspp                # thin wrapper around build/src/tspp
```

Tests (see `tests/README.md`):

```bash
tests/run.sh                       # full suite; prints PASS/FAIL/XFAIL/XPASS summary,
                                   # exits non-zero on FAIL or XPASS
tests/run.sh regression            # only tests whose path contains "regression"
(cd build && ctest --output-on-failure)  # same suite via CTest
TSPP=build-release/src/tspp tests/run.sh # suite against the Release compiler
TSPP_LEAK_CHECK=1 tests/run.sh     # also fail if the (Debug/ASan) compiler leaks
```

A test is `<name>.tspp` + `<name>.expected` (stdout; optional `// exit: N`), or has a
`// test: parse-only` / `// test: compile-error` header. `tests/driver/*.sh` are
driver tests (shell injection, other CWD, link failure, CLI). `<name>.xfail` (reason + bug ID) marks a known
bug; when your fix makes it pass (XPASS), delete the `.xfail` file.

`lsan.supp` holds only narrow, commented third-party leak suppressions (currently
none); never add blanket malloc/new entries.

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
  Primitive type names live in one table, `src/tokens/primitive_types.h`
  (lexer keywords, semantic `resolveType`, `getLLVMType`); `boolean` and `bool`
  are both i1 and canonicalize to `"bool"` (H4 fixed).
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
- **Build types and sanitizers**: `CMAKE_BUILD_TYPE` defaults to Debug when unset.
  `TSPP_ENABLE_ASAN` (default ON for Debug, OFF otherwise) and `TSPP_ENABLE_UBSAN`
  (default OFF; fatal on UB) instrument the compiler only; values are cached
  per build dir. Compiled programs are PIE and not instrumented unless
  `tspp --asan`; `libcore` (the runtime) is built with `-fno-sanitize=all` for that.
  `compile_commands.json` is generated (build.sh symlinks it to the root; gitignored).
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
- Docs were made truthful (H7 fixed). Keep them so: list a feature as working
  only with a passing test; fence code samples ```` ```tspp ```` (compiled by
  `scripts/check_doc_samples.sh`, run by the suite as `tests/driver/doc_samples.sh`)
  or ```` ```tspp-planned ```` for syntax that doesn't work yet. When a fix lands,
  update the README tables, `ROADMAP.md` and KNOWLEDGE_TRANSFER §9 status.
