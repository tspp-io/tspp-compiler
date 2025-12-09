#!/bin/bash
set -euo pipefail

# Install dependencies for building TSPP on macOS (Homebrew)
# Usage: ./scripts/install_deps_macos.sh  (run as a user; installs via brew)

if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew not found. Installing Homebrew..."
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  echo "Please follow any instructions printed by the Homebrew installer, then re-run this script."
  exit 0
fi

echo "Updating Homebrew..."
brew update

echo "Installing required packages via Homebrew..."
brew install cmake llvm@17 bdw-gc ninja pkg-config git

echo "Homebrew installs LLVM keg-only. To use it, you may need to add it to your PATH and set LLVM_DIR:" \
     "\n  export PATH=$(brew --prefix llvm@17)/bin:$PATH" \
     "\n  export LLVM_DIR=$(brew --prefix llvm@17)/lib/cmake/llvm"

echo "Installation complete."
