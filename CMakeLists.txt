cmake_minimum_required(VERSION 3.10)
project(tspp_interpreter)
set(CMAKE_CXX_STANDARD 17)

add_executable(tspp_interpreter
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/ast/ast.cpp
    src/types/types.cpp
    src/interpreter/interpreter.cpp
    src/common/common.cpp
    src/repl/repl.cpp
)

include_directories(
    src/lexer
    src/parser
    src/ast
    src/types
    src/interpreter
    src/common
    src/repl
)
