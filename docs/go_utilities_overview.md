# Go Utilities Overview

This document describes the Go built-in functions and utilities implemented in the TSPP++ standard library (`src/packages/tspp_std`).

## Overview

The TSPP++ language brings Go semantics to TypeScript syntax. This includes implementing Go's built-in functions that provide essential operations for memory management, collections, and error handling.

## Implemented Go Built-ins

### Memory Allocation Functions

#### `make()`
- **Purpose**: Dynamic allocation for slices, maps, and channels
- **Implementations**:
  - `tspp_make_slice(elem_size, len, cap)` - Create a slice with specified length and capacity
  - `tspp_make_map(key_size, value_size)` - Create a map (hash table)
  - `tspp_make_channel(elem_size, buffer_size)` - Create a buffered or unbuffered channel

#### `new()`
- **Purpose**: Zero-value allocation for any type
- **Implementation**: `tspp_new(size)` - Allocates memory and initializes to zero

### Collection Functions

#### `len()`
- **Purpose**: Get length of collections
- **Implementation**: `tspp_len(collection, type)` 
- **Supported types**: slices (0), maps (1), channels (2), strings (3)

#### `cap()`
- **Purpose**: Get capacity of slices and channels
- **Implementation**: `tspp_cap(collection, type)`
- **Supported types**: slices (0), channels (2)

### Slice Operations

#### `append()`
- **Purpose**: Append elements to slices with automatic growth
- **Implementation**: `tspp_append(slice, elem, elem_size)`
- **Features**: Automatic capacity growth using Go's growth strategy

#### `copy()`
- **Purpose**: Copy slice elements with overlap handling
- **Implementation**: `tspp_copy(dst, src, n)`
- **Features**: Uses `memmove` for safe overlapping memory operations

### Map Operations

#### `delete()`
- **Purpose**: Remove key-value pairs from maps
- **Implementation**: `tspp_delete(map, key, key_size)`
- **Note**: Currently a placeholder implementation

### Channel Operations

#### `close()`
- **Purpose**: Close channels to signal no more values
- **Implementation**: `tspp_close(channel)`
- **Effect**: Sets the channel's closed flag

### Error Handling

#### `panic()` and `recover()`
- **Purpose**: Go-style error handling mechanism
- **Implementations**:
  - `tspp_panic(message)` - Trigger a panic with optional message
  - `tspp_recover()` - Recover from a panic if possible
- **Features**: 
  - Uses `setjmp`/`longjmp` for non-local control flow
  - Prints panic messages to stderr
  - Supports recovery in deferred functions

### Memory Management

#### `tspp_gc_collect()`
- **Purpose**: Trigger garbage collection
- **Note**: Placeholder for Boehm GC integration

## Data Structures

The implementation defines several core Go data structures:

### `TsppSlice`
```c
typedef struct TsppSlice {
    void* data;    // Pointer to slice elements
    size_t len;    // Current length
    size_t cap;    // Capacity
} TsppSlice;
```

### `TsppString`
```c
typedef struct TsppString {
    const char* data;  // String data
    size_t len;        // String length
} TsppString;
```

### `TsppMap`
```c
typedef struct TsppMap {
    void* impl;    // Internal hash table implementation
    size_t len;    // Number of key-value pairs
} TsppMap;
```

### `TsppChannel`
```c
typedef struct TsppChannel {
    void* impl;     // Internal channel implementation
    size_t cap;     // Buffer capacity
    bool closed;    // Closed flag
} TsppChannel;
```

## Implementation Status

### ✅ Completed
- Basic slice operations (make, append, len, cap, copy)
- Basic memory allocation (new, make)
- Basic map operations (make, len, delete)
- Basic channel operations (make, close, cap)
- Panic/recover mechanism
- Data structure definitions

### 🚧 Partial Implementation
- Map operations (delete is placeholder)
- Channel operations (send/receive not implemented)
- Garbage collection (placeholder)

### ❌ Not Yet Implemented
- Hash table implementation for maps
- Channel communication (send/receive)
- Goroutine integration
- Type assertions and type switches
- Interface method dispatch
- Range iteration support

## Usage

The Go built-ins are automatically linked into the TSPP++ runtime through the standard library. They provide the foundation for Go-style programming patterns in TSPP++ code.

### Example Usage (C API)
```c
#include "go_builtins.h"

// Create a slice of integers
TsppSlice* slice = tspp_make_slice(sizeof(int), 0, 10);

// Append an element
int value = 42;
slice = tspp_append(slice, &value, sizeof(int));

// Check length
size_t len = tspp_len(slice, 0);  // Returns 1

// Clean up (in actual usage, handled by GC)
free(slice->data);
free(slice);
```

## Integration

The Go built-ins are integrated into the TSPP++ standard library through:

1. **Header file**: `go_builtins.h` - Declares all built-in functions
2. **Implementation**: `go_builtins.cpp` - C++ implementation with C linkage
3. **Build system**: Added to `CMakeLists.txt` for compilation
4. **Entry point**: Included in `std_entrypoint.cpp` for linking

## Future Enhancements

1. **Performance**: Optimize slice growth and memory allocation
2. **Concurrency**: Implement full channel semantics with goroutines
3. **Types**: Add runtime type information for better type checking
4. **GC Integration**: Full integration with Boehm GC
5. **Error Handling**: Enhanced panic/recover with stack traces
6. **Memory Safety**: Add bounds checking and memory safety features

This implementation provides the foundation for Go-style programming in TSPP++, enabling developers to use familiar Go patterns while benefiting from TypeScript syntax and LLVM performance.