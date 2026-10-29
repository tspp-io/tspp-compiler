# Security Policy

## Supported versions

tspp is an early prototype (0.1.x) with no releases yet. Security fixes go to
the `master` branch.

## Reporting a vulnerability

Please **do not open a public issue**. Report it privately through GitHub:

**[Report a vulnerability](https://github.com/tspp-io/tspp-compiler/security/advisories/new)**
(the repository's *Security* tab → *Report a vulnerability*).

Include what you can of:

- a description of the problem and its impact
- steps or a minimal `.tspp` file / command line to reproduce it
- the commit you tested, your OS, and your LLVM version (`tspp --version`,
  `clang --version`)
- a suggested fix, if you have one

This is a small volunteer project. We aim to acknowledge a report within a
week, agree on a fix and a disclosure date with you, and credit you in the
advisory unless you prefer otherwise. Please give us a reasonable chance to
fix the issue before disclosing it publicly.

## Scope

In scope:

- the `tspp` compiler and driver (`src/`), for example a crafted source file
  or file name that makes the compiler execute commands, write outside its
  temp directory or output path, or crash in an exploitable way
- the runtime linked into compiled programs (`src/core/syscall/`,
  `libcore.a`)
- the build and install scripts in this repository

Out of scope:

- bugs in programs written in tspp (tspp is not memory-safe yet: arrays are
  not bounds-checked, and `__builtin_syscall` / `#asm` do what they say)
- vulnerabilities in LLVM, clang, Boehm GC or libcurl; report those upstream
- `scripts/install_deps_ubuntu.sh` piping apt.llvm.org's installer into a root
  shell: that is a documented trade-off; read the script before running it

## What the compiler does on your machine

`tspp file.tspp` compiles **and runs** the program. Compiling an untrusted
`.tspp` file is safe in the sense that the compiler itself does not execute
it, but the default mode then runs it with your privileges. Use
`tspp -o out file.tspp` (or `--emit-llvm`) to build without running.

The driver starts `llvm-as`, `clang` and the program directly
(`llvm::sys::ExecuteAndWait` with an argument vector), never through a shell,
and writes intermediates to a private temporary directory.

## Past issues

- **Shell injection in the driver** (fixed by commit `d920727`, branch
  `fix/safe-driver`, before any release). The driver built `sed`, `llvm-as`,
  `clang` and `./temp_exec` command lines by string concatenation and ran
  them with `std::system()`, so a source file name containing shell
  metacharacters (`x.tspp; touch pwned`) executed arbitrary commands; a
  stale or planted `./temp_exec` in the working directory could also be run.
  Regression test: `tests/driver/shell_injection.sh`.
