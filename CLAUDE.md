# CLAUDE.md — working on the tspp compiler

## What tspp is

tspp is a **compiler** from a TypeScript-like syntax to LLVM IR, written in C++17
against the LLVM C++ API. Positioning: *TypeScript syntax with C++-level memory
control, compiled to native via LLVM. Safe by default, unsafe when you ask for it.*
The driver emits `<file>.tspp.ll`, links it with `clang` against the runtime
(`build/src/libcore.a`), Boehm GC (`-lgc`) and libcurl, then runs the binary.
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
| Driver | `src/main.cpp` (shells out via `std::system`: `sed`, `llvm-as`, `clang`, `./temp_exec`) |
| Runtime | `src/core/syscall/syscall_wrapper.cpp` (`extern "C" tspp_*`), built into `libcore.a` |

## Build and run (verified)

Prerequisites: CMake ≥ 3.14, C++17 compiler, LLVM dev package (`LLVMConfig.cmake`,
shared `libLLVM`), `clang` + `llvm-as` on `PATH`, Boehm GC (`libgc`), libcurl dev.

```bash
./build.sh                         # rm -rf build; configure; build. Ends with
                                   # "Could not open file: --version" — harmless (L4).
./build/src/tspp path/to/file.tspp # compile + link + run. MUST be run from the
                                   # repo root (links ./build/src/libcore.a by CWD).
./test.sh path/to/file.tspp        # same, plus IR patching (see traps) and ASan filtering
git checkout compile_commands.json # build.sh overwrites the committed copy; don't commit it
```

Tests (see `tests/README.md`):

```bash
tests/run.sh                       # full suite; prints PASS/FAIL/XFAIL/XPASS summary,
                                   # exits non-zero on FAIL or XPASS
tests/run.sh regression            # only tests whose path contains "regression"
(cd build && ctest --output-on-failure)  # same suite via CTest
```

A test is `<name>.tspp` + `<name>.expected` (stdout), or has a `// test: parse-only`
/ `// test: compile-error` header. `<name>.xfail` (reason + bug ID) marks a known
bug; when your fix makes it pass (XPASS), delete the `.xfail` file.

Silence leak noise from the ASan-built compiler/programs:
`export LSAN_OPTIONS="suppressions=$(pwd)/lsan.supp:report_objects=0:exitcode=0"`

The driver writes `<input>.ll`, `temp.bc`, `temp_exec` into the **current
directory**, prints `Output:` on stdout before running the program, and returns 0
even if linking or the program fails (until Phase 3 of the plan fixes it).

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
- Don't commit build artifacts (`build/`, `*.ll`, `temp.bc`, `temp_exec`).

## Known traps

- **Types are inferred by string-matching names in codegen** (`"string"`,
  `"ClassName.method"`, `Symbol::typeName` strings). Changes ripple unpredictably.
  `boolean` is not mapped in `getLLVMType` and falls back to `ptr` (H4); `bool` works.
- **`test.sh` patches IR that the driver does not**: it renames a `void @main` to
  `@main_tspp` and appends an `i32 main()` wrapper (or a trivial `main` if none).
  The driver only prints `[ERROR] User main() must return i32.` to stderr for
  `function main(): void` (LLVMCodeGenerator.cpp ~4752) and links/runs anyway.
  Prefer `function main(): int` in tests.
- **The build forces Debug + ASan** (root `CMakeLists.txt` sets
  `CMAKE_BUILD_TYPE Debug` and `-fsanitize=address`; `-DCMAKE_BUILD_TYPE=...` is
  overridden). Compiled programs are also linked with `-fsanitize=address -no-pie`.
  Expect LeakSanitizer output on stderr; compare stdout only.
- Runtime string helpers return a shared `static std::string` buffer (C1): a string
  from `+`/to-string is overwritten by the next such call.
- Non-constant global initializers never run (C2). Top-level expression statements
  are silently dropped by the parser (H1): put code in `function main(): int`.
- The parser silently skips tokens it doesn't understand and lexer errors are
  ignored (H1); codegen errors go to stderr but IR is still written and run, with no
  `verifyModule` (H6). "Exit 0" proves little — check the output.
- Operator precedence differs from `docs/grammar.md` (H3): `& | ^` bind like
  comparisons, shifts like `+ -`.
- Many docs (README, `docs/architecture.md`, `docs/language_spec.md`) overstate
  features (H7). Trust the code, `KNOWLEDGE_TRANSFER.md`, and `MISSING_FEATURES_REPORT.md`.
