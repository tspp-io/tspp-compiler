# Development environment setup

## Requirements

- C++17 compiler (GCC or Clang) and CMake 3.14+
- **LLVM 17 or newer** with development files (`LLVMConfig.cmake` and the
  shared `libLLVM`), and the matching `clang` and `llvm-as` on `PATH`.
  Verified with LLVM 17, 18, 22 and 23; CI builds with LLVM 18 and 22.
- Boehm GC (`libgc`) and libcurl, with development headers

## Install scripts

Read a script before running it.

| Platform | Command | Notes |
|---|---|---|
| Debian/Ubuntu | `sudo ./scripts/install_deps_ubuntu.sh` | Adds apt.llvm.org (pipes its `llvm.sh` into bash as root) and installs the LLVM 17 packages. Any LLVM ≥ 17 from apt.llvm.org or your distro works. |
| Fedora/RHEL | `sudo ./scripts/install_deps_fedora.sh` | Distro `llvm-devel`, `clang`, `boehm-gc-devel`, `libcurl-devel` |
| Arch | `sudo ./scripts/install_deps_arch.sh` | Distro `llvm`, `clang`, `bdw-gc`, `curl` |
| macOS | `./scripts/install_deps_macos.sh` | Homebrew `llvm@17` (keg-only: add its `bin/` to `PATH` and set `LLVM_DIR` as the script prints) |

`./scripts/setup_env.sh` finds `llvm-config` and prints the `PATH` and
`LLVM_DIR` exports for it.

If CMake cannot find LLVM, pass its CMake directory, for example:

```bash
./build.sh -- -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm      # Debian/Ubuntu
./build.sh -- -DLLVM_DIR="$(brew --prefix llvm)/lib/cmake/llvm" # macOS
```

## Build

```bash
./build.sh                 # Debug build in build/ (the compiler is built with ASan)
./build.sh --release       # Release build in build-release/ (no sanitizers)
./build.sh --clean         # wipe the build directory first
./build.sh -- -DTSPP_ENABLE_UBSAN=ON   # extra CMake configure arguments
```

CMake options (cached per build directory; use `--clean` to change them):

| Option | Default | Effect |
|---|---|---|
| `CMAKE_BUILD_TYPE` | `Debug` when unset | |
| `TSPP_ENABLE_ASAN` | ON for Debug, OFF otherwise | AddressSanitizer for the compiler (not the runtime, not compiled programs) |
| `TSPP_ENABLE_UBSAN` | OFF | UndefinedBehaviorSanitizer for the compiler, fatal on UB |

Compiled tspp programs are built without sanitizers unless you pass
`tspp --asan`. `compile_commands.json` is generated in the build directory
(`build.sh` symlinks it to the repo root for editors).

## Test

```bash
tests/run.sh                          # full suite
tests/run.sh regression               # tests whose path contains "regression"
TSPP=build-release/src/tspp tests/run.sh   # against the Release build
TSPP_LEAK_CHECK=1 tests/run.sh        # also fail if the Debug compiler leaks
scripts/check_doc_samples.sh          # compile the ```tspp samples in the docs
```

See [tests/README.md](../tests/README.md) for test kinds and how to add one.
