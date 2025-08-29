# TSPP++ Programming Language

<div align="center">

![TSPP++ Logo](docs/logo.png)

**A modern systems programming language combining TypeScript-like syntax with Go-style semantics**

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build Status](https://github.com/theQuarky/tspp/workflows/CI/badge.svg)](https://github.com/theQuarky/tspp/actions)
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![LLVM](https://img.shields.io/badge/powered%20by-LLVM-brightgreen.svg)](https://llvm.org/)

</div>

## Overview

TSPP++ is an innovative programming language that brings together the familiar syntax of TypeScript with the performance and concurrency model of Go. It compiles to LLVM IR, enabling high-performance native code generation while providing advanced memory management, explicit control over memory regions, smart pointers, and zero-cost abstractions.

### Key Features

🚀 **Performance-First**: Compiles to optimized LLVM IR  
🔒 **Memory Safety**: Explicit memory region control (`#stack`, `#heap`, `#static`)  
🧠 **Smart Pointers**: Built-in support for `#shared<T>`, `#unique<T>`, `#weak<T>`  
⚡ **SIMD Operations**: Native vectorization with `#simd`  
🔧 **Zero-Cost Abstractions**: `#zerocast` for compile-time optimizations  
🛡️ **Safety Controls**: `@unsafe` annotations for explicit unsafe operations  
🎯 **TypeScript Syntax**: Familiar syntax for web developers  
🔄 **Go Semantics**: Goroutine-inspired concurrency model  

## Quick Start

### Prerequisites

- **C++ Compiler**: GCC 9+ or Clang 10+
- **CMake**: Version 3.14 or higher
- **LLVM**: Version 17 or higher
- **Boehm GC**: For garbage collection support

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake llvm-17 llvm-17-dev libgc-dev
```

#### macOS
```bash
brew install cmake llvm bdw-gc
```

### Building from Source

```bash
# Clone the repository
git clone https://github.com/theQuarky/tspp.git
cd tspp

# Build the project
./build.sh

# The binary will be located at ./build/src/tspp
```

### Your First TSPP++ Program

Create a file called `hello.tspp`:

```typescript
// hello.tspp - A simple TSPP++ program
#stack string message = "Hello, TSPP++ World!";

function main(): void {
    console.log(message);
    
    // Demonstrate memory regions
    #heap int* numbers = new int[10];
    #stack int counter = 0;
    
    // SIMD operations
    #simd float[4] vector = [1.0, 2.0, 3.0, 4.0];
    
    // Smart pointer usage
    #shared<MyClass> obj = make_shared<MyClass>();
    
    // Compile-time constants
    #const int BUFFER_SIZE = 1024;
}

class MyClass {
    #private int value;
    
    constructor(val: int) {
        this.value = val;
    }
    
    #public getValue(): int {
        return this.value;
    }
}
```

Compile and run:

```bash
./build/src/tspp hello.tspp
# Output: Hello, TSPP++ World!
```

## Language Features

### Memory Management

TSPP++ provides explicit control over memory regions:

```typescript
#stack int localVar = 42;        // Stack allocation
#heap int* dynamicVar = new int(42);  // Heap allocation  
#static int globalVar = 42;      // Static/global allocation
```

### Smart Pointers

Built-in smart pointer support for safe memory management:

```typescript
#shared<Resource> resource = make_shared<Resource>();
#unique<Buffer> buffer = make_unique<Buffer>(1024);
#weak<Resource> weakRef = resource.weak_from_this();
```

### SIMD and Performance

Native SIMD support for high-performance computing:

```typescript
#simd float[4] vector1 = [1.0, 2.0, 3.0, 4.0];
#simd float[4] vector2 = [5.0, 6.0, 7.0, 8.0];
#simd float[4] result = vector1 + vector2;  // Vectorized addition
```

### Zero-Cost Abstractions

Compile-time optimizations with zero runtime overhead:

```typescript
#zerocast function<T> max(a: T, b: T): T {
    return a > b ? a : b;
}

#const int result = max(10, 20);  // Resolved at compile time
```

## Architecture

TSPP++ follows a modern compiler architecture:

```mermaid
flowchart TD
    A[Source Code] --> B[Lexer]
    B --> C[Parser]
    C --> D[AST]
    D --> E[Type Checker]
    E --> F[LLVM IR Generator]
    F --> G[Optimization]
    G --> H[Native Code]
```

### Components

- **Lexer**: Tokenizes TSPP++ source code
- **Parser**: Builds Abstract Syntax Tree (AST)
- **Type System**: Advanced type checking and inference
- **Code Generator**: LLVM IR generation
- **Runtime**: Garbage collection and memory management
- **Standard Library**: Core functionality and utilities

## Documentation

- 📖 **[Language Specification](docs/language_spec.md)** - Complete language reference
- 🔧 **[Compilation Pipeline](docs/pipeline.md)** - How TSPP++ works internally
- 📝 **[Grammar Reference](docs/grammar.md)** - EBNF grammar specification
- 🎯 **[Examples](examples/)** - Sample programs and tutorials

## Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Quick Links

- 🐛 [Report a Bug](https://github.com/theQuarky/tspp/issues/new?template=bug_report.md)
- ✨ [Request a Feature](https://github.com/theQuarky/tspp/issues/new?template=feature_request.md)
- 💬 [Join Discussions](https://github.com/theQuarky/tspp/discussions)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## TODO - Roadmap to Full Implementation

### 🔄 Go Semantics Implementation

#### Concurrency Primitives
- [ ] **Goroutines**: Implement `go` keyword for lightweight thread creation
- [ ] **Channels**: Typed channels with buffered/unbuffered variants (`chan T`, `chan<- T`, `<-chan T`)
- [ ] **Select Statement**: Multi-way communication with timeout support
- [ ] **WaitGroups**: Synchronization primitives for goroutine coordination
- [ ] **Mutexes**: Read/write mutexes for shared memory protection

#### Built-in Functions
- [ ] **`mark_shared<T>()`**: Memory sharing annotation for concurrent access
- [ ] **`make()`**: Dynamic allocation for slices, maps, channels
- [ ] **`new()`**: Zero-value allocation for any type
- [ ] **`len()`**: Length function for collections (arrays, slices, maps, strings, channels)
- [ ] **`cap()`**: Capacity function for slices and channels
- [ ] **`close()`**: Channel closing mechanism
- [ ] **`copy()`**: Slice copying with overlap handling
- [ ] **`append()`**: Slice append with automatic growth
- [ ] **`delete()`**: Map key deletion
- [ ] **`panic()` / `recover()`**: Error handling mechanism

#### Type System Extensions
- [ ] **Interfaces**: Method set definitions and dynamic dispatch
- [ ] **Type Assertions**: Runtime type checking (`x.(T)`, `x.(type)`)
- [ ] **Type Switches**: Pattern matching on interface types
- [ ] **Slices**: Dynamic arrays with length/capacity (`[]T`)
- [ ] **Maps**: Hash table implementation (`map[K]V`)
- [ ] **Method Sets**: Interface satisfaction checking

#### Control Flow
- [ ] **Defer Statements**: Deferred execution with LIFO ordering
- [ ] **Range Iteration**: For-range over slices, maps, channels, strings
- [ ] **Labeled Breaks/Continues**: Multi-level loop control

### 📚 Standard Library Implementation

#### Core Packages
- [ ] **`context`**: Cancellation and deadline propagation
- [ ] **`errors`**: Error creation and handling utilities
- [ ] **`fmt`**: Formatted I/O (Printf, Sprintf, etc.)
- [ ] **`io`**: I/O primitives (Reader, Writer, Closer interfaces)
- [ ] **`sort`**: Sorting algorithms and interfaces
- [ ] **`strings`**: String manipulation utilities
- [ ] **`strconv`**: String conversions (Atoi, Itoa, etc.)
- [ ] **`time`**: Time and date handling with timezones

#### Network Programming
- [ ] **`net`**: TCP/UDP socket programming
  - [ ] TCP server/client implementation
  - [ ] UDP socket support
  - [ ] Unix domain sockets
  - [ ] IP address parsing and manipulation
- [ ] **`net/http`**: HTTP client and server
  - [ ] HTTP/1.1 and HTTP/2 support
  - [ ] Middleware system
  - [ ] WebSocket support
  - [ ] TLS integration
- [ ] **`net/url`**: URL parsing and manipulation
- [ ] **`net/smtp`**: SMTP client implementation

#### Operating System Interface
- [ ] **`os`**: Operating system interface
  - [ ] File operations (create, open, read, write, delete)
  - [ ] Directory operations (mkdir, readdir, walk)
  - [ ] Process management (exec, fork, wait)
  - [ ] Environment variables
  - [ ] Signal handling
  - [ ] User and group information
- [ ] **`os/exec`**: External command execution
- [ ] **`path/filepath`**: File path manipulation
- [ ] **`syscall`**: Low-level system calls

#### Data Encoding/Decoding
- [ ] **`encoding/json`**: JSON marshaling/unmarshaling
- [ ] **`encoding/xml`**: XML processing
- [ ] **`encoding/base64`**: Base64 encoding/decoding
- [ ] **`encoding/hex`**: Hexadecimal encoding
- [ ] **`encoding/csv`**: CSV file processing
- [ ] **`compress/gzip`**: GZIP compression/decompression

#### Cryptography & Security
- [ ] **`crypto`**: Cryptographic primitives
- [ ] **`crypto/rand`**: Cryptographically secure random numbers
- [ ] **`crypto/md5`**: MD5 hashing (deprecated but needed)
- [ ] **`crypto/sha1`**: SHA-1 hashing
- [ ] **`crypto/sha256`**: SHA-256 hashing
- [ ] **`crypto/aes`**: AES encryption/decryption
- [ ] **`crypto/rsa`**: RSA public key cryptography
- [ ] **`crypto/tls`**: TLS/SSL implementation

#### Mathematics
- [ ] **`math`**: Mathematical functions (sin, cos, sqrt, etc.)
- [ ] **`math/big`**: Arbitrary precision arithmetic
- [ ] **`math/rand`**: Pseudo-random number generation

#### Concurrency Support
- [ ] **`sync`**: Synchronization primitives
  - [ ] Mutex, RWMutex implementations
  - [ ] WaitGroup for goroutine synchronization
  - [ ] Once for one-time initialization
  - [ ] Pool for object reuse
  - [ ] Atomic operations
- [ ] **`sync/atomic`**: Low-level atomic memory primitives

#### Development & Testing
- [ ] **`testing`**: Unit testing framework
- [ ] **`log`**: Logging utilities with different levels
- [ ] **`flag`**: Command-line flag parsing
- [ ] **`runtime`**: Runtime reflection and control
  - [ ] Garbage collector interface
  - [ ] Memory statistics
  - [ ] Goroutine inspection

### 🏗️ Language Infrastructure

#### Memory Management
- [ ] **Garbage Collector**: Enhance Boehm GC integration
- [ ] **Memory Regions**: Implement `#stack`, `#heap`, `#static` annotations
- [ ] **Smart Pointers**: Complete implementation of `#shared<T>`, `#unique<T>`, `#weak<T>`
- [ ] **Memory Safety**: Bounds checking for arrays and slices

#### Compiler Features
- [ ] **Package System**: Import/export mechanism with visibility rules
- [ ] **Generic Programming**: Type parameters and constraints
- [ ] **Reflection**: Runtime type information and method calling
- [ ] **Build System**: Package compilation and dependency management
- [ ] **Cross-compilation**: Multiple target architectures

#### IR Code Optimization Passes
- [ ] **Post-IR Generation Optimizations**: LLVM-based optimization pipeline
  - [ ] **Escape Analysis**: Stack vs heap allocation decisions
  - [ ] **Inlining**: Function call optimization
  - [ ] **Dead Code Elimination**: Unused code removal
  - [ ] **Constant Folding**: Compile-time constant evaluation
  - [ ] **Loop Optimizations**: Loop unrolling and vectorization
  - [ ] **SIMD Optimizations**: Vector operation enhancements
  - [ ] **Register Allocation**: Optimal register usage
  - [ ] **Instruction Scheduling**: CPU pipeline optimization

### 🎯 Priority Implementation Order

1. **Phase 1: Core Go Semantics**
   - Built-in functions (`make`, `new`, `len`, `cap`)
   - Slices and maps basic implementation
   - Interfaces and method sets

2. **Phase 2: Concurrency Foundation**
   - Goroutines and channel basics
   - Select statements
   - Defer implementation

3. **Phase 3: Essential Standard Library**
   - `fmt` package for formatted I/O
   - `os` package for file operations
   - `net` package for basic networking

4. **Phase 4: Advanced Features**
   - HTTP server/client
   - JSON marshaling
   - Testing framework

5. **Phase 5: Performance & Polish**
   - IR code optimization passes (post-IR generation)
   - Memory management refinements
   - Documentation and examples

## Acknowledgments

- **LLVM Project** - For the excellent compiler infrastructure
- **Boehm GC** - For garbage collection support
- **TypeScript Team** - For syntax inspiration
- **Go Team** - For semantic design inspiration

---

<div align="center">

**Made with ❤️ by the TSPP++ community**

[Website](https://tspp.dev) • [Documentation](https://docs.tspp.dev) • [Discord](https://discord.gg/tspp)

</div>
