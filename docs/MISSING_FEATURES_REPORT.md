# Missing Features Report

This document outlines features that are defined in the tspp grammar and documentation but are currently missing implementation in the compiler (specifically the LLVM code generator).

## 1. Smart Pointers (`#shared<T>`, `#unique<T>`)

**Current Status:**
- **Lexer/Parser:** Supported. `SmartPointerTypeNode` exists.
- **Codegen:** **Missing**. `LLVMCodeGenerator::visit(ast::SmartPointerTypeNode&)` contains only TODOs.

**Required Implementation:**
- Implement reference counting logic for `#shared<T>`.
- Implement unique ownership semantics and destructors for `#unique<T>`.
- Update `LLVMCodeGenerator.cpp` to generate appropriate LLVM IR for allocation and deallocation.

## 2. Memory Safety Controls (`#heap`, `#stack`)

**Current Status:**
- **Lexer/Parser:** Supported. `VarDecl` captures storage qualifiers.
- **Codegen:** **Missing**. `LLVMCodeGenerator::visit(ast::VarDecl&)` currently ignores `#heap` and allocates everything on the stack via `CreateAlloca`.

**Required Implementation:**
- Add logic in `visit(ast::VarDecl&)` to check for `ast::StorageQualifier::Heap`.
- For `#heap`, generate `malloc`/`gc_malloc` calls instead of `alloca`.
- Ensure proper pointer types are used for heap-allocated variables.

## 3. Safety Annotations (`@unsafe`)

**Current Status:**
- **Lexer/Parser:** Supported. `PointerTypeNode` captures qualifiers.
- **Codegen:** **Missing**. `LLVMCodeGenerator::visit(ast::PointerTypeNode&)` contains TODOs regarding unsafe vs. aligned pointer semantics.

**Required Implementation:**
- Implement logic to bypass safety checks (e.g., bounds checking, null checks) when `@unsafe` is present.
- Handle pointer arithmetic generation for unsafe pointers.

## 4. Concurrency (Go Semantics)

**Current Status:**
- **Lexer/Parser:** Keywords like `go`, `chan` might be reserved, but no AST nodes or parsing logic found for `go` statements or channel operations.
- **Codegen:** **Completely Missing**. No runtime support for goroutines or channels.

**Required Implementation:**
- Add AST nodes for `GoStmt` and channel types.
- Implement a runtime library (or link against one) for green threads/coroutines.
- Implement `go` keyword handling in codegen to spawn coroutines.

## 5. SIMD Operations (`#simd`)

**Current Status:**
- **Lexer/Parser:** Supported. `FunctionModifier::Simd` exists.
- **Codegen:** **Partial**. The compiler adds a `"tspp.simd"` string attribute to functions, but generates no vectorized IR.

**Required Implementation:**
- Implement an optimization pass or codegen logic to utilize LLVM vector types (`<4 x float>`, etc.) when `#simd` is present.
- Map array operations to LLVM vector intrinsics.

## 6. Zero-Cost Abstractions (`#zerocast`)

**Current Status:**
- **Lexer/Parser:** Supported. `FunctionModifier::Zerocast` exists.
- **Codegen:** **Partial**. Adds `"tspp.zerocast"` attribute, but no specific optimization logic exists.

**Required Implementation:**
- Define the exact semantics of `#zerocast` (e.g., forced inlining, monomorphization).
- Implement the corresponding logic in the codegen or optimization pipeline.
