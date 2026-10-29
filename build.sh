#!/usr/bin/env bash
# Configure and build tspp.
#
# Usage: ./build.sh [--release] [--clean] [-- extra cmake configure args]
#   (default)   Debug build in build/          (compiler built with ASan)
#   --release   Release build in build-release/ (no sanitizers)
#   --clean     delete that build directory first
#   -- ARGS     passed to the cmake configure step, e.g.
#               ./build.sh -- -DTSPP_ENABLE_UBSAN=ON
#
# Builds are incremental. Sanitizer options are cached per build directory,
# so use --clean (or pass -DTSPP_ENABLE_ASAN=...) to change them.
# The compiler ends up at <build dir>/src/tspp; run the tests against it with
# TSPP=<build dir>/src/tspp tests/run.sh.
set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

handle_error() {
    echo -e "${RED}Error: $1${NC}" >&2
    exit 1
}

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_TYPE=Debug
BUILD_DIR="$ROOT/build"
CLEAN=0
EXTRA_ARGS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --release) BUILD_TYPE=Release; BUILD_DIR="$ROOT/build-release" ;;
        --debug)   BUILD_TYPE=Debug;   BUILD_DIR="$ROOT/build" ;;
        --clean)   CLEAN=1 ;;
        -h|--help) sed -n '2,14p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 0 ;;
        --)        shift; EXTRA_ARGS=("$@"); break ;;
        *)         handle_error "unknown option '$1' (see ./build.sh --help)" ;;
    esac
    shift
done

echo -e "${BLUE}TSPP Build Script${NC} (${BUILD_TYPE}, ${BUILD_DIR#"$ROOT"/})"

if [[ $CLEAN -eq 1 && -d "$BUILD_DIR" ]]; then
    echo -e "${YELLOW}Removing old build directory...${NC}"
    rm -rf "$BUILD_DIR" || handle_error "Failed to remove $BUILD_DIR"
fi

echo -e "${YELLOW}Configuring CMake...${NC}"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    ${EXTRA_ARGS[@]+"${EXTRA_ARGS[@]}"} || handle_error "CMake configuration failed"

if [[ "$(uname)" == "Darwin" ]]; then
    NUM_CORES=$(sysctl -n hw.ncpu)
else
    NUM_CORES=$(nproc)
fi

echo -e "${YELLOW}Building project using ${NUM_CORES} cores...${NC}"
cmake --build "$BUILD_DIR" -j "${NUM_CORES}" || handle_error "Build failed"

echo -e "${GREEN}Build completed successfully!${NC}"
"$BUILD_DIR/src/tspp" --version || handle_error "tspp --version failed"

# Point the repo-root compile_commands.json (gitignored) at this build for
# clangd and other tools.
if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
    ln -sfn "${BUILD_DIR#"$ROOT"/}/compile_commands.json" "$ROOT/compile_commands.json" \
        || handle_error "Failed to link compile_commands.json"
fi

echo -e "${GREEN}All done! Binary located at ${BUILD_DIR#"$ROOT"/}/src/tspp${NC}"
