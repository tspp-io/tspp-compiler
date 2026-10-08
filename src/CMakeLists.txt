# Diagnostics sink of the compiler (not part of the runtime; built with the
# compiler's sanitizers, unlike libcore).
add_library(diagnostics
    core/diagnostics/error_reporter.cpp
)

# core: file/string utilities used by the compiler, plus the runtime
# (syscall_wrapper.cpp). Depends on no other tspp library.
add_library(core
    core/utils/file_utils.cpp
    core/utils/string_utils.cpp
    core/syscall/syscall_wrapper.cpp
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
    parser/module/ModuleResolver.cpp
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

# Find libcurl for HTTP support in runtime syscalls
find_package(CURL REQUIRED)
if(CURL_FOUND)
    message(STATUS "Found libcurl: ${CURL_VERSION_STRING}")
else()
    message(FATAL_ERROR "libcurl not found. Please install libcurl and development headers.")
endif()

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Link dependencies
target_link_libraries(core PUBLIC CURL::libcurl)
# Runtime strings are allocated with Boehm GC (GC_MALLOC_ATOMIC).
target_link_libraries(core PUBLIC ${BOEHM_GC_LIB})
# tokens uses only the header core/common/common_types.h (via the shared src/
# include directory), not the core library.
target_link_libraries(lexer PUBLIC core tokens diagnostics)
target_link_libraries(parser PUBLIC core tokens diagnostics)
target_link_libraries(tspp_codegen PUBLIC core parser diagnostics LLVM ${BOEHM_GC_LIB})


# libcore.a doubles as the runtime linked into compiled tspp programs.
# Programs are not sanitizer-instrumented by default (tspp --asan opts in), so
# the runtime must not reference any sanitizer runtime (TSPP_ENABLE_ASAN /
# TSPP_ENABLE_UBSAN apply to the compiler only), and it must be linkable into
# a PIE executable.
if(NOT MSVC)
    target_compile_options(core PRIVATE -fno-sanitize=all)
endif()
set_target_properties(core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Include directories
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(diagnostics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(tokens PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lexer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(tspp_codegen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Main executable
add_executable(tspp main.cpp)

# Version and LLVM tool directory for the driver (--version, finding clang /
# llvm-as when they are not on PATH).
target_compile_definitions(tspp PRIVATE
    TSPP_VERSION="${PROJECT_VERSION}"
    TSPP_LLVM_TOOLS_DIR="${LLVM_TOOLS_BINARY_DIR}"
)

target_link_libraries(tspp
    PRIVATE
        core
        diagnostics
        tokens
        lexer
        parser
        tspp_codegen
        LLVM
)