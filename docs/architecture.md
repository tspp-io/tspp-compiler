# tspp Compiler Architecture

This document provides a high-level overview of the tspp compiler architecture and its compilation pipeline.

## Overview

The tspp compiler is a modular system written in C++17 that compiles tspp source code into LLVM IR, which is then compiled to native machine code using Clang/LLVM.

## Compilation Pipeline

The compilation process follows a standard pipeline:

1.  **Lexical Analysis (Lexer)**
2.  **Parsing (Parser & AST Construction)**
3.  **Semantic Analysis**
4.  **Code Generation (LLVM IR)**
5.  **Native Compilation (Clang)**

### 1. Lexical Analysis (`src/lexer`, `src/tokens`)

The **Lexer** (`lexer::Lexer`) takes the raw source code string and converts it into a stream of **Tokens**.
-   **Tokens** (`tokens::Token`) represent atomic units like keywords, identifiers, literals, and operators.
-   The lexer handles comments, whitespace, and pre-processing of attributes (e.g., `#stack`, `#heap`).
-   It uses specialized scanners (`IdentifierScanner`, `NumberScanner`, etc.) for different token types.

### 2. Parsing (`src/parser`)

The **Parser** (`parser::Parser`) consumes the stream of tokens and builds an **Abstract Syntax Tree (AST)**.
-   The parser uses a recursive descent approach.
-   **AST Nodes** (`src/parser/nodes`) represent the syntactic structure of the program (e.g., `FunctionDecl`, `VarDecl`, `BinaryExpr`).
-   The `ASTBuilder` visitors (`src/parser/visitors/ast_builder`) are responsible for constructing specific parts of the AST.

### 3. Semantic Analysis (`src/parser/visitors/semantic`)

Before code generation, the **Semantic Analyzer** (`ast::SemanticAnalyzerVisitor`) traverses the AST to ensure correctness.
-   **Type Checking**: Verifies that types match (e.g., assigning a string to an int).
-   **Scope Resolution**: Resolves variable and function names to their declarations.
-   **Symbol Table**: Manages scopes and symbol definitions.
-   **Error Reporting**: Collects and reports semantic errors (e.g., "undefined variable").

### 4. Code Generation (`src/codegen`)

The **Code Generator** (`codegen::LLVMCodeGenerator`) traverses the validated AST and generates **LLVM IR** (Intermediate Representation).
-   It uses the LLVM C++ API to build instructions.
-   **Memory Management**: Handles `#stack` (alloca), `#heap` (malloc/GC), and `#static` (global variables) allocations.
-   **Control Flow**: Generates IR for loops (`for`, `while`), conditionals (`if`, `switch`), and function calls.
-   **Features**: Implements SIMD operations, smart pointers, and other language features.

### 5. Native Compilation

The generated LLVM IR (`.ll` file) is passed to `llvm-as` to create a bitcode file (`.bc`), which is then linked and compiled by `clang` to produce the final executable.
-   **Runtime Library**: The compiler links against `libcore.a` (tspp runtime) and `libgc` (Boehm GC).

## Project Structure

```
src/
├── ast/            # AST node definitions and printers
├── codegen/        # LLVM code generation logic
├── core/           # Common utilities, diagnostics, and syscall wrappers
├── interpreter/    # JIT interpreter (REPL)
├── lexer/          # Lexical analysis
├── parser/         # Parsing and AST construction
├── tokens/         # Token definitions
└── main.cpp        # Compiler entry point
```

## Key Classes

-   `lexer::Lexer`: Main entry point for lexing.
-   `parser::Parser`: Main entry point for parsing.
-   `ast::BaseNode`: Base class for all AST nodes.
-   `ast::SemanticAnalyzerVisitor`: Performs semantic checks.
-   `codegen::LLVMCodeGenerator`: Generates LLVM IR.
-   `interpreter::JITInterpreter`: Handles REPL execution.

## Dependencies

-   **LLVM**: Backend for code generation.
-   **Clang**: Used for linking and final binary creation.
-   **Boehm GC**: Conservative garbage collector for managed memory.
