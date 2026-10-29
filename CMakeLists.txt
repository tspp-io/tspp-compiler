cmake_minimum_required(VERSION 3.14)
project(tspp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# compile_commands.json for clangd & co. lives in the build directory;
# build.sh symlinks it to the repo root (the root copy is gitignored).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# -fPIE/-fPIC everywhere: the compiler links as a PIE, and libcore.a (the
# runtime) is linked into PIE programs by the driver.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build type: Debug unless the caller chose one (-DCMAKE_BUILD_TYPE=Release,
# or ./build.sh --release). Multi-config generators pick it at build time.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()
message(STATUS "tspp build type: ${CMAKE_BUILD_TYPE}")

# Sanitizers for the compiler itself (not for compiled tspp programs: those
# get ASan only with `tspp --asan`). The defaults are taken from the build
# type on the first configure of a build directory; afterwards the cached
# values win, so pass -DTSPP_ENABLE_ASAN=... explicitly to change them.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_tspp_asan_default ON)
else()
    set(_tspp_asan_default OFF)
endif()
option(TSPP_ENABLE_ASAN "Build the compiler with AddressSanitizer (+LeakSanitizer)"
       ${_tspp_asan_default})
option(TSPP_ENABLE_UBSAN "Build the compiler with UndefinedBehaviorSanitizer" OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(TSPP_SANITIZE_FLAGS "")
if(TSPP_ENABLE_ASAN)
    list(APPEND TSPP_SANITIZE_FLAGS -fsanitize=address)
endif()
if(TSPP_ENABLE_UBSAN)
    # Fatal, so UB fails the test suite instead of printing to (ignored) stderr.
    list(APPEND TSPP_SANITIZE_FLAGS -fsanitize=undefined -fno-sanitize-recover=undefined)
endif()
if(TSPP_SANITIZE_FLAGS)
    if(MSVC)
        message(FATAL_ERROR "TSPP_ENABLE_ASAN/TSPP_ENABLE_UBSAN are only supported with GCC/Clang")
    endif()
    add_compile_options(${TSPP_SANITIZE_FLAGS} -fno-omit-frame-pointer)
    add_link_options(${TSPP_SANITIZE_FLAGS})
endif()
message(STATUS "tspp sanitizers: ASan=${TSPP_ENABLE_ASAN} UBSan=${TSPP_ENABLE_UBSAN}")

# Add source directory
add_subdirectory(src)

# Golden-output test suite (tests/run.sh); run with `ctest` from the build dir.
enable_testing()
add_test(NAME tspp_golden_tests COMMAND ${CMAKE_SOURCE_DIR}/tests/run.sh)
set_tests_properties(tspp_golden_tests PROPERTIES
    ENVIRONMENT "TSPP=$<TARGET_FILE:tspp>;TSPP_BUILD_DIR=${CMAKE_BINARY_DIR}")
