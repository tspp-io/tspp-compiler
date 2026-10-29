<!-- Workspace instructions for GitHub Copilot. The full version, kept current, is CLAUDE.md at the repo root. -->

This repository is **tspp**, a **compiler** (not an interpreter) from a TypeScript-like language to native code via LLVM: TypeScript syntax with C++-level memory control, safe by default, unsafe when you ask for it. C++17, CMake, LLVM 17+ C++ API, Boehm GC.

Pipeline: lexer (`src/lexer`, `src/tokens`) → parser/AST builders (`src/parser/visitors/ast_builder`) → imports (`src/parser/module`) → `SemanticAnalyzerVisitor` (`src/parser/visitors/semantic`) → `LLVMCodeGenerator` (`src/codegen`) → driver (`src/main.cpp`: `llvm-as` + `clang` link with the runtime `libcore.a` from `src/core/syscall`). There is no JIT; the REPL compiles and runs each input.

Build with `./build.sh`, test with `tests/run.sh` (see `tests/README.md`). Report errors through `core::diagnostics()` with a source location. Every fix needs a test under `tests/`. Read `CLAUDE.md` for working rules and known traps.
