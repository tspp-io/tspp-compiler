#!/bin/bash
set -euo pipefail

# Install dependencies for building TSPP on Ubuntu/Debian-based systems
# Usage: sudo ./scripts/install_deps_ubuntu.sh

if [ "$EUID" -ne 0 ]; then
  echo "This script must be run as root (sudo)." >&2
  exit 1
fi

echo "Installing packages for TSPP development (Ubuntu/Debian)..."

# Optional: add official LLVM apt repository for the latest LLVM-17 packages
echo "Adding apt.llvm.org repository for LLVM 17 (recommended)..."
if ! command -v curl >/dev/null 2>&1; then
  apt-get update
  apt-get install -y curl
fi

# The apt.llvm.org helper script will add the appropriate repo entries for a given version
curl -sSL https://apt.llvm.org/llvm.sh | bash -s 17

apt-get update

# Install common build tools and LLVM 17
apt-get install -y --no-install-recommends \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  git \
  clang-17 \
  lld-17 \
  llvm-17 \
  llvm-17-dev \
  libclang-17-dev \
  libgc-dev \
  ca-certificates

echo "Installation complete. You may need to log out/in for PATH changes to take effect."
echo "Verify llvm-config is available: llvm-config-17 --version"
