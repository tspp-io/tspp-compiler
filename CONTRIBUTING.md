# Contributing to tspp

Thank you for your interest in contributing to tspp! This document provides guidelines and information about how to contribute to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [How to Contribute](#how-to-contribute)
- [Development Setup](#development-setup)
- [Coding Guidelines](#coding-guidelines)
- [Commit Guidelines](#commit-guidelines)
- [Pull Request Process](#pull-request-process)
- [Issue Guidelines](#issue-guidelines)
- [Community and Support](#community-and-support)

## Code of Conduct

This project adheres to the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

## Getting Started

### Prerequisites

Before contributing, ensure you have:

- **C++ Compiler**: GCC 9+ or Clang 10+
- **CMake**: Version 3.14 or higher
- **LLVM**: 17 or newer, with development files, plus `clang` and `llvm-as` on `PATH`
- **Boehm GC**: For garbage collection support
- **libcurl** (with development headers): Used by the runtime's HTTP syscalls
- **Git**: For version control

### Development Setup

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/tspp-compiler.git
   cd tspp-compiler
   ```

3. **Install dependencies** with the script for your platform
   (`scripts/install_deps_{ubuntu,fedora,arch,macos}.sh`), or with your
   package manager: CMake, an LLVM ≥ 17 dev package and the matching clang,
   Boehm GC and libcurl dev packages. See [docs/dev-setup.md](docs/dev-setup.md).

4. **Build the project**:
   ```bash
   ./build.sh
   ```

5. **Run tests**:
   ```bash
   tests/run.sh
   ```

6. **Read [CLAUDE.md](CLAUDE.md)** (working rules and known traps; written for
   AI assistants, useful for humans too) and [tests/README.md](tests/README.md).

## How to Contribute

### Types of Contributions

We welcome several types of contributions:

- 🐛 **Bug fixes**
- ✨ **New features**
- 📚 **Documentation improvements**
- 🧪 **Tests and testing improvements**
- 🔧 **Build system and tooling**
- 🎨 **Examples and tutorials**

### Areas Needing Help

Check our [issues](https://github.com/tspp-io/tspp-compiler/issues) and the
[roadmap](ROADMAP.md) (which lists open bugs with IDs) for areas where we need help:

- Issues labeled `good first issue` for newcomers
- Issues labeled `help wanted` for general contributions
- Issues labeled `documentation` for docs improvements

## Development Setup

### Project Structure

```
tspp-compiler/
├── src/
│   ├── main.cpp           # Driver (CLI, toolchain invocation) and REPL
│   ├── tokens/            # Token types; primitive_types.h (the type-name table)
│   ├── lexer/             # Tokenization
│   ├── parser/            # AST nodes, builders (parser), semantic analysis, imports
│   ├── codegen/           # LLVM IR generation
│   ├── core/              # Diagnostics, utilities, and the runtime (core/syscall/)
│   ├── ast/               # AST printers (not used by the build)
│   └── types/             # Placeholder (not built)
├── lib/system.tspp        # Experimental tspp-level stdlib (does not compile yet)
├── tests/                 # Test suite: tests/run.sh, see tests/README.md
├── scripts/               # Dependency installers, check_doc_samples.sh
├── docs/                  # Language and compiler docs
├── build.sh               # Configure + build (Debug or --release)
└── test.sh                # Compile and run one file (wrapper around the driver)
```

### Building Components

```bash
# Debug build (compiler built with ASan) in build/
./build.sh

# Release build (no sanitizers) in build-release/
./build.sh --release

# Plain CMake; options: TSPP_ENABLE_ASAN (default ON for Debug),
# TSPP_ENABLE_UBSAN (default OFF)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Test suite (TSPP=build-release/src/tspp to test the Release build)
tests/run.sh

# One file: compile and run
./build/src/tspp tests/examples/hello.tspp
```

## Coding Guidelines

### C++ Style

We use **clang-format** for code formatting. The configuration is in `.clang-format`.

```bash
# Format your code before committing
clang-format -i src/**/*.cpp src/**/*.h
```

### Key Principles

1. **Correctness first**: the compiler must fail loudly (a located error, or
   an internal compiler error) rather than emit wrong code
2. **Diagnose before fixing**: find the root cause, with `file:line` evidence
3. **Clarity**: Write self-documenting code with meaningful names
4. **Testing**: Include tests for new functionality
5. **Documentation**: Update documentation for user-facing changes

### Code Style Rules

- Use **camelCase** for variables and functions
- Use **PascalCase** for classes and types
- Use **UPPER_CASE** for constants and macros
- Prefer `std::unique_ptr` over raw pointers
- Use `const` wherever possible
- Keep functions focused and small
- Add comments for complex algorithms

### Example Code Style

```cpp
// Good
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(Visitor& visitor) const = 0;
    
private:
    std::string name_;
    std::unique_ptr<Type> type_;
};

// Avoid
class astnode {  // Wrong naming
public:
    Type* type;  // Raw pointer, not const
    void DoSomething();  // Unclear function name
};
```

## Commit Guidelines

### Commit Message Format

We follow the [Conventional Commits](https://www.conventionalcommits.org/) specification:

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Test additions or modifications
- `build`: Build system changes
- `ci`: CI/CD changes

### Examples

```bash
feat(parser): add support for union types
fix(lexer): handle nested comments correctly
docs(readme): update installation instructions
test(codegen): add tests for compound assignment
```

## Pull Request Process

### Before Submitting

1. **Create an issue** first (unless it's a trivial fix)
2. **Fork the repository** and create a feature branch
3. **Write tests** for your changes
4. **Update documentation** if needed
5. **Run the full test suite**
6. **Format your code** with clang-format

### PR Checklist

- [ ] My code follows the project's coding standards
- [ ] I have performed a self-review of my code
- [ ] I have commented my code, particularly in hard-to-understand areas
- [ ] I have made corresponding changes to the documentation
- [ ] My changes generate no new warnings
- [ ] I have added tests that prove my fix is effective or that my feature works
- [ ] New and existing unit tests pass locally with my changes

### PR Template

When creating a PR, please use our template and provide:

- **Description** of the changes
- **Type of change** (bug fix, feature, docs, etc.)
- **Testing** performed
- **Screenshots** (if applicable)
- **Related issues** (if any)

## Issue Guidelines

### Bug Reports

When reporting bugs, please include:

- **Steps to reproduce** the issue
- **Expected vs actual behavior**
- **Environment details** (OS, compiler version, etc.)
- **Code samples** that demonstrate the issue
- **Error messages** (full stack traces when available)

### Feature Requests

For feature requests, please provide:

- **Problem description**: What problem does this solve?
- **Proposed solution**: How would you like it implemented?
- **Alternatives considered**: Any other approaches you've thought of?
- **Use cases**: Real-world scenarios where this would be useful

## Testing

### Running Tests

```bash
./build.sh
tests/run.sh                 # full suite; summary line PASS/FAIL/XFAIL/XPASS
tests/run.sh regression      # only tests whose path contains "regression"
scripts/check_doc_samples.sh # compile the ```tspp samples in the docs
```

### Writing Tests

Every fix and feature needs a test. A run test is `tests/<area>/<name>.tspp`
plus `<name>.expected` (exact stdout). Parse-only and compile-error tests are
marked with a header comment; a known bug is pinned with a `<name>.xfail`
file whose first line names the bug ID. Details: [tests/README.md](tests/README.md).

Never edit a `.expected` file to make a test pass unless the expectation is
provably wrong, and say so in the PR.

## Documentation

### Types of Documentation

- **API Documentation**: In-code comments and headers
- **User Documentation**: README, language spec, examples
- **Developer Documentation**: Architecture, contributing guides

### Documentation Standards

- Use clear, concise language
- Include code examples
- Keep documentation up to date with code changes
- Use proper markdown formatting

## Community and Support

### Communication Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: General questions and community discussion

### Getting Help

If you need help:

1. Check existing **issues** and **discussions**
2. Review the **documentation**
3. Ask questions in **GitHub Discussions**

### Recognition

Contributors are recognized in several ways:

- Listed in project **contributors**
- Mentioned in **release notes** for significant contributions
- **Maintainer status** for consistent, quality contributions

## Legal

By contributing to tspp, you agree that your contributions will be licensed under the MIT License. You also confirm that you have the right to submit the work under this license and agree to the [Developer Certificate of Origin](https://developercertificate.org/).

---

Thank you for contributing to tspp! 🚀

If you have questions about contributing, please don't hesitate to ask in our GitHub Discussions or create an issue.