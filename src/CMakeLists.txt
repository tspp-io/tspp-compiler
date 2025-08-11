add_library(core
    core/diagnostics/error_reporter.cpp
    core/utils/file_utils.cpp
    core/utils/log_utils.cpp
    core/utils/string_utils.cpp
)

add_library(tokens
    tokens/stream/token_stream.cpp
    tokens/token.cpp
)

add_library(lexer
    lexer/lexer.cpp
    lexer/scanner/base/scanner_base.cpp
    lexer/scanner/specialized/identifier_scanner.cpp
    lexer/scanner/specialized/number_scanner.cpp
    lexer/scanner/specialized/operator_scanner.cpp
    lexer/scanner/specialized/string_scanner.cpp
    lexer/scanner/token_scanner.cpp
    lexer/state/lexer_state.cpp
)

add_library(parser
    parser/parser.cpp
    parser/visitors/ast_builder/ast_builder_visitor.cpp
    parser/visitors/ast_builder/ast_builders/DeclarationBuilder.cpp
    parser/visitors/ast_builder/ast_builders/StatementBuilder.cpp
    parser/visitors/ast_builder/ast_builders/ExpressionBuilder.cpp
    parser/visitors/ast_builder/ast_builders/TypeBuilder.cpp
    parser/visitors/semantic/SemanticAnalyzerVisitor.cpp
    parser/visitors/semantic/SemanticScope.cpp
)


# Only add tspp_codegen once, and link it with LLVM and all dependencies
add_library(tspp_codegen
    codegen/LLVMCodeGenerator.cpp
    codegen/ConstantExpressionEvaluator.cpp
)
target_link_libraries(tspp_codegen 
    PUBLIC 
        core 
        parser
        LLVM
)


# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

# Find Boehm GC
find_library(BOEHM_GC_LIB gc)
if(BOEHM_GC_LIB)
    message(STATUS "Found Boehm GC: ${BOEHM_GC_LIB}")
else()
    message(FATAL_ERROR "Boehm GC library not found. Please install libgc-dev.")
endif()

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Link dependencies
target_link_libraries(core PUBLIC tokens)
target_link_libraries(tokens PUBLIC core)
target_link_libraries(lexer PUBLIC core tokens)
target_link_libraries(parser PUBLIC core tokens)
target_link_libraries(tspp_codegen PUBLIC core parser LLVM ${BOEHM_GC_LIB})


# Include directories
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(tokens PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(tspp_codegen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Main executable
add_executable(tspp main.cpp)

target_link_libraries(tspp
    PRIVATE
        core
        tokens
        lexer
        parser
        tspp_codegen
)

# Add TSPP standard library
add_subdirectory(packages/tspp_std)

target_link_libraries(tspp PRIVATE tspp_std)