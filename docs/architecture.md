# tspp Compiler Architecture

A high-level map of the compiler. For the long version (data flow, runtime
ABI, known issues), read [KNOWLEDGE_TRANSFER.md](KNOWLEDGE_TRANSFER.md).

## Overview

`tspp` is a single C++17 program built against the LLVM C++ API. It compiles a
`.tspp` file to LLVM IR, has `llvm-as` check the IR, links it with `clang`
against the tspp runtime (`libcore.a`), Boehm GC and libcurl, and by default
runs the result. There is no interpreter and no JIT; the REPL compiles and
runs each input as its own program.

## Compilation pipeline

| # | Stage | Code | Output |
|---|---|---|---|
| 1 | Lexer | `src/lexer/` (specialized scanners in `scanner/specialized/`), tokens in `src/tokens/` | token vector; inserts `;` at line ends |
| 2 | Parser | `src/parser/parser.cpp` → `visitors/ast_builder/` (`parseTopLevel`) → `ast_builders/{Declaration,Statement,Expression,Type}Builder.cpp`; nodes in `src/parser/nodes/` | `ast::ProgramNode` |
| 3 | Imports | `src/parser/module/ModuleResolver.cpp` | imported files' ASTs merged in front (no namespaces) |
| 4 | Semantic analysis | `src/parser/visitors/semantic/SemanticAnalyzerVisitor.cpp`, `SemanticScope.cpp` | name/arity/const checks, default arguments filled in |
| 5 | Code generation | `src/codegen/LLVMCodeGenerator.cpp`, `ConstantExpressionEvaluator.cpp` | `llvm::Module`, checked with `llvm::verifyModule` |
| 6 | Driver | `src/main.cpp` | IR in a private temp dir → `llvm-as` → `clang -O3` link → run |
| — | Diagnostics | `src/core/diagnostics/error_reporter.*` (`core::diagnostics()`) | `file:line:col: error: ...` from every stage; the driver stops after the first failing stage |
| — | Runtime | `src/core/syscall/syscall_wrapper.cpp` (`extern "C" tspp_*`), built into `libcore.a` | linked into every compiled program |

Details that matter when changing things:

- **Primitive types** come from one table, `src/tokens/primitive_types.h`
  (lexer keywords, semantic types, `getLLVMType`).
- **Types in codegen** are often inferred by matching type-name strings
  (`"string"`, `"ClassName.method"`). Changes there ripple.
- **Program startup**: codegen emits an `i32 main()` that calls
  `tspp_runtime_init()` (GC init), then `__tspp_global_ctors()` (global
  initializers, class static initializers and blocks, top-level statements,
  in source order), then the user's `main`. See
  [language_spec.md](language_spec.md#program-startup-and-initialization-order).
- **Strings** returned by the runtime are fresh GC-owned buffers.
- **Child processes** are started with `llvm::sys::ExecuteAndWait` and an argv
  vector, never through a shell. `clang`/`llvm-as` are found on `PATH`
  (`TSPP_CLANG`, `TSPP_LLVM_AS` override); `libcore.a` next to the `tspp`
  executable (`TSPP_RUNTIME_LIB` overrides).
- **Optimization** is `clang -O3` on the IR; the compiler runs no LLVM passes
  of its own.

Not implemented in codegen (parsed only): `#heap`/`#stack` storage, pointer
and smart-pointer semantics, `#simd` and the other performance attributes
(recorded as LLVM string attributes only). See [ROADMAP.md](../ROADMAP.md).

## Source tree

```
src/
├── main.cpp        # driver + REPL
├── tokens/         # Token, TokenType, primitive_types.h
├── lexer/          # Lexer and scanners
├── parser/         # AST nodes, builders, semantic analysis, imports
├── codegen/        # LLVMCodeGenerator (~5k lines), constant folding
├── core/           # diagnostics, file/string utils, runtime (syscall/)
├── ast/            # AST printers; not used by the build
└── types/          # placeholder; not built
```

## Build targets

`src/CMakeLists.txt`: libraries `diagnostics`, `core` (utilities + runtime;
`libcore.a` is also the runtime archive, built without sanitizers), `tokens`,
`lexer`, `parser`, `tspp_codegen`, and the `tspp` executable. Build types and
sanitizer options (`TSPP_ENABLE_ASAN`, `TSPP_ENABLE_UBSAN`) are described in
[dev-setup.md](dev-setup.md).

## Dependencies

- **LLVM 17 or newer**: IR generation (C++ API), plus the `llvm-as` and
  `clang` tools at run time.
- **Boehm GC**: allocation of objects, arrays and strings in compiled programs.
- **libcurl**: the runtime's `sys.http_get` call.
