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
- **LLVM**: Version 17 or higher
- **Boehm GC**: For garbage collection support
- **Git**: For version control

### Development Setup

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/tspp.git
   cd tspp
   ```

3. **Install dependencies**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install build-essential cmake llvm-17 llvm-17-dev libgc-dev
   
   # macOS
   brew install cmake llvm bdw-gc
   ```

4. **Build the project**:
   ```bash
   ./build.sh
   ```

5. **Run tests**:
   ```bash
   cd tests
   ./run_tests.sh  # If available
   ```

## How to Contribute

### Types of Contributions

We welcome several types of contributions:

- 🐛 **Bug fixes**
- ✨ **New features**
- 📚 **Documentation improvements**
- 🧪 **Tests and testing improvements**
- 🔧 **Build system and tooling**
- 🎨 **Examples and tutorials**
- 🌐 **Translations**

### Areas Needing Help

Check our [issues](https://github.com/theQuarky/tspp/issues) for areas where we need help:

- Issues labeled `good first issue` for newcomers
- Issues labeled `help wanted` for general contributions
- Issues labeled `documentation` for docs improvements

## Development Setup

### Project Structure

```
tspp/
├── src/                    # Source code
│   ├── ast/               # Abstract Syntax Tree
│   ├── lexer/             # Tokenization
│   ├── parser/            # Parsing logic
│   ├── types/             # Type system
│   ├── codegen/           # LLVM code generation
│   ├── core/              # Core utilities
│   └── packages/          # Standard library
├── tests/                 # Test files
├── docs/                  # Documentation
├── examples/              # Example programs
└── tools/                 # Development tools
```

### Building Components

```bash
# Full build
./build.sh

# Debug build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run specific tests
./test.sh examples/hello.tspp
```

## Coding Guidelines

### C++ Style

We use **clang-format** for code formatting. The configuration is in `.clang-format`.

```bash
# Format your code before committing
clang-format -i src/**/*.cpp src/**/*.h
```

### Key Principles

1. **Performance**: tspp is designed for performance - consider the impact of your changes
2. **Safety**: Maintain memory safety and type safety
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
test(codegen): add tests for SIMD operations
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
# Build the project
./build.sh

# Run example tests
./test.sh tests/basic/variables.tspp
./test.sh tests/advanced/generics.tspp

# Run all tests (if test runner exists)
cd tests && ./run_all.sh
```

### Writing Tests

When adding new features, please include tests:

```bash
# Create test file
echo 'your test code here' > tests/feature/new_feature.tspp

# Test it
./test.sh tests/feature/new_feature.tspp
```

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
- **Discord**: Real-time chat (coming soon)

### Getting Help

If you need help:

1. Check existing **issues** and **discussions**
2. Review the **documentation**
3. Ask questions in **GitHub Discussions**
4. Join our **Discord server** (coming soon)

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