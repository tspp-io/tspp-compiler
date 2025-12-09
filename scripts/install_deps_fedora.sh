#!/bin/bash
set -euo pipefail

# Install dependencies for building TSPP on Fedora/RHEL-like systems
# Usage: sudo ./scripts/install_deps_fedora.sh

if [ "$EUID" -ne 0 ]; then
  echo "This script must be run as root (sudo)." >&2
  exit 1
fi

echo "Installing packages for TSPP development (Fedora/RHEL)..."

# Fedora
if command -v dnf >/dev/null 2>&1; then
  dnf install -y @development-tools cmake ninja-build pkgconfig git llvm llvm-devel clang lld libclang-devel boehm-gc-devel
else
  echo "dnf not found; please install packages for your distribution manually."
  exit 1
fi

echo "Installation complete. Verify llvm-config --version or llvm-config-17 --version"
