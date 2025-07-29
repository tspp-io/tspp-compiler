# TSPP++ Interpreter

A C++ interpreter for the custom TSPP++ language, featuring TypeScript-like syntax and Go-style semantics.

## Structure
- lexer: Tokenizes input source
- parser: Parses tokens into AST
- ast: Abstract Syntax Tree classes
- types: Type system
- interpreter: Executes AST
- common: Shared utilities
- repl: Read-Eval-Print Loop
- tests: Test scripts and cases

## Build
```sh
mkdir build && cd build
cmake ..
make
```

## Run
```sh
./tspp_interpreter path/to/file.tspp
```
