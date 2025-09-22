# TSPP Foreign Function Interface (FFI) and Syscall Support

## Overview

TSPP now supports the ability to declare and call external functions, enabling standard libraries to be written in TSPP itself rather than requiring C++ implementations. This provides a powerful Foreign Function Interface (FFI) that allows TSPP code to make system calls and interface with external libraries.

## External Function Declarations

Use the `extern` keyword to declare functions that are implemented outside of TSPP:

```typescript
// Basic external function declaration
extern function my_function(param1: int, param2: string): int;

// External function with custom library name
extern function print_message(msg: string): void "libc_print";
```

### Syntax

```
extern function <function_name>(<parameters>): <return_type> [<external_name>];
```

- `function_name`: The name used to call the function in TSPP code
- `parameters`: Standard TSPP parameter list with types
- `return_type`: The return type of the function
- `external_name`: Optional string literal specifying the actual external function name (defaults to function_name)

## Syscall Interface

TSPP provides a comprehensive syscall interface that allows direct access to operating system functions:

### Generic Syscalls

```typescript
extern function tspp_syscall0(number: int): int;
extern function tspp_syscall1(number: int, arg1: int): int;
extern function tspp_syscall2(number: int, arg1: int, arg2: int): int;
// ... up to tspp_syscall6
```

### Common Syscall Wrappers

```typescript
extern function tspp_sys_open(pathname: string, flags: int, mode: int): int;
extern function tspp_sys_read(fd: int, buf: string, count: int): int;
extern function tspp_sys_write(fd: int, buf: string, count: int): int;
extern function tspp_sys_close(fd: int): int;
extern function tspp_sys_exit(status: int): int;
```

## Writing Standard Libraries in TSPP

With the FFI and syscall support, you can now write standard library modules entirely in TSPP:

### Example: File I/O Module

```typescript
// File: stdlib/io.tspp

// External syscall declarations
extern function tspp_sys_open(pathname: string, flags: int, mode: int): int;
extern function tspp_sys_write(fd: int, buf: string, count: int): int;
extern function tspp_sys_close(fd: int): int;

// Constants
const STDOUT_FILENO = 1;
const O_WRONLY = 1;
const O_CREAT = 64;

// High-level TSPP functions
function createFile(path: string): int {
    return tspp_sys_open(path, O_WRONLY | O_CREAT, 0644);
}

function writeToFile(fd: int, data: string): void {
    tspp_sys_write(fd, data, data.length);
}

function println(message: string): void {
    tspp_sys_write(STDOUT_FILENO, message, message.length);
    tspp_sys_write(STDOUT_FILENO, "\n", 1);
}
```

### Example: Memory Management Module

```typescript
// File: stdlib/memory.tspp

extern function tspp_syscall6(number: int, arg1: int, arg2: int, 
                              arg3: int, arg4: int, arg5: int, arg6: int): int;

const SYS_mmap = 9;
const SYS_munmap = 11;
const PROT_READ = 1;
const PROT_WRITE = 2;
const MAP_PRIVATE = 2;
const MAP_ANONYMOUS = 32;

function allocateMemory(size: int): int {
    return tspp_syscall6(SYS_mmap, 0, size, PROT_READ | PROT_WRITE, 
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

function deallocateMemory(addr: int, size: int): int {
    return tspp_syscall2(SYS_munmap, addr, size);
}
```

## Implementation Details

### How It Works

1. **Parser Extension**: The parser now recognizes `extern function` declarations and marks them as external
2. **Code Generation**: External functions are declared in LLVM but not defined, allowing the linker to resolve them
3. **Runtime Linking**: External functions are linked at compile/runtime against the TSPP standard library or system libraries
4. **Type Safety**: Full type checking is maintained for external function calls

### Syscall Implementation

The syscall interface is implemented in C++ (`src/packages/tspp_std/syscall.cpp`) and provides:

- Generic syscall functions that accept varying numbers of arguments
- Common syscall wrappers for frequently used system calls
- Platform-specific implementations (currently Linux x86_64)

### Benefits

1. **Flexibility**: Standard libraries can be written in TSPP itself
2. **Performance**: Direct syscall access eliminates C++ wrapper overhead
3. **Maintainability**: Standard library code in TSPP is easier to understand and modify
4. **Extensibility**: Users can easily add new system interfaces

## Examples

See the `examples/` directory for complete examples:

- `examples/syscall_demo.tspp`: Demonstrates basic syscall usage
- `examples/stdlib/io.tspp`: File I/O operations using syscalls
- `examples/stdlib/memory.tspp`: Memory management using syscalls

## Future Enhancements

- Module import system for TSPP standard libraries
- Automatic binding generation for C libraries
- Cross-platform syscall abstraction
- Dynamic library loading support