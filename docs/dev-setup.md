# Development environment setup

This document contains quick instructions and scripts to set up a development environment for the TSPP++ compiler on common platforms.

Scripts
- `./scripts/install_deps_ubuntu.sh` — Debian/Ubuntu installer (adds apt.llvm.org entry and installs LLVM 17 + build tools). Run with `sudo`.
- `./scripts/install_deps_fedora.sh` — Fedora/RHEL installer. Run with `sudo`.
- `./scripts/install_deps_macos.sh` — macOS installer using Homebrew.
- `./scripts/setup_env.sh` — Helper to detect an installed LLVM and print environment variables to export.

Usage (Ubuntu/Debian)

1. Run the installer as root:

```sh
sudo ./scripts/install_deps_ubuntu.sh
```

2. After install, either re-open your shell or run the helper to get the correct LLVM variables:

```sh
./scripts/setup_env.sh
# then copy the suggested exports into your ~/.bashrc or ~/.profile
```

Usage (macOS)

1. Run the macOS script as a normal user (it will install Homebrew first if necessary):

```sh
./scripts/install_deps_macos.sh
```

2. Homebrew installs `llvm@17` as a "keg-only" package. Add the printed PATH/LLVM_DIR to your shell.

Usage (Fedora)

```sh
sudo ./scripts/install_deps_fedora.sh
```

Notes and caveats

- The Ubuntu script uses `curl https://apt.llvm.org/llvm.sh` to add the LLVM apt repository for the requested version. Inspect the script before running in restricted environments.
- Some distributions provide older LLVM packages in their default repos. If you need LLVM 17 specifically, prefer using the apt.llvm.org repo (Ubuntu) or official packages for your distro.
- After installing LLVM you may need to point CMake to `LLVM_DIR`. Typical values:
  - `/usr/lib/llvm-17/lib/cmake/llvm`
  - `/usr/local/opt/llvm/lib/cmake/llvm` (macOS Homebrew)

If you'd like, I can attempt to run the appropriate installer on this machine now (I will need `sudo` permission to install system packages). Alternatively, I can run only the environment helper and help you point CMake at an existing LLVM installation.
