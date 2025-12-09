#!/bin/bash
set -euo pipefail

# Install dependencies for building TSPP on Arch Linux / Manjaro
# Usage: sudo ./scripts/install_deps_arch.sh

if [ "$EUID" -ne 0 ]; then
  echo "This script must be run as root (sudo)." >&2
  exit 1
fi

if ! command -v pacman >/dev/null 2>&1; then
  echo "pacman not found. This script is for Arch Linux / derivatives." >&2
  exit 1
fi

echo "Updating package database and installing required packages (Arch)..."
pacman -Syu --noconfirm --needed \
  base-devel \
  cmake \
  ninja \
  pkgconf \
  git \
  llvm \
  clang \
  lld \
  bdw-gc

echo "Installed packages. Verify llvm-config is available: llvm-config --version"
echo "If llvm-config shows a version >= 17 you're set; otherwise consider installing a specific LLVM release or using the distro package manager to upgrade."
