#!/bin/bash
set -euo pipefail

# Small helper to detect llvm-config and print environment exports you can add to your shell.
# Usage: ./scripts/setup_env.sh

if command -v llvm-config-17 >/dev/null 2>&1; then
  LC=llvm-config-17
elif command -v llvm-config >/dev/null 2>&1; then
  LC=llvm-config
else
  echo "No llvm-config found on PATH. Please install LLVM 17 and ensure llvm-config or llvm-config-17 is available."
  exit 1
fi

LLVM_PREFIX=$($LC --prefix)
LLVM_CMAKE_DIR="$LLVM_PREFIX/lib/cmake/llvm"

echo "Detected llvm-config: $LC"
echo "LLVM prefix: $LLVM_PREFIX"
echo
echo "Add the following to your shell profile (e.g. ~/.profile or ~/.bashrc):"
echo
echo "export PATH=\"$LLVM_PREFIX/bin:\$PATH\""
echo "export LLVM_DIR=\"$LLVM_CMAKE_DIR\""

echo
echo "You can also run (current session):"
echo "  export PATH=\"$LLVM_PREFIX/bin:\$PATH\""
echo "  export LLVM_DIR=\"$LLVM_CMAKE_DIR\""
