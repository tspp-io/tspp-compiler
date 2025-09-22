# Node.js-Style Print Utilities for TSPP

This directory contains a Node.js-style console library implemented entirely in TSPP using syscalls. The library provides familiar `console.log`, `console.error`, `console.warn` and other utilities that Node.js developers are accustomed to.

## Features

### Console Logging Functions

- **`console.log(message)`** - Standard output logging
- **`console.info(message)`** - Information logging with icon (ℹ)
- **`console.warn(message)`** - Warning messages to stderr with icon (⚠)
- **`console.error(message)`** - Error messages to stderr with icon (✖)
- **`console.debug(message)`** - Debug messages with timestamp
- **`console.trace(message)`** - Trace messages with stack trace indicator

### Timing Utilities

- **`console.time(label)`** - Start a timer with the given label
- **`console.timeEnd(label)`** - End the timer and display duration

### Development Tools

- **`console.assert(condition, message)`** - Assert a condition, show error if false
- **`console.clear()`** - Clear the console screen
- **`console.count(label)`** - Count and display occurrences of a label
- **`console.countReset(label)`** - Reset a counter

### Grouping and Organization

- **`console.group(label)`** - Start a collapsible group
- **`console.groupCollapsed(label)`** - Start a collapsed group
- **`console.groupEnd()`** - End the current group
- **`console.table(data)`** - Display data in a table format

### Process Utilities

- **`process.exit(code)`** - Exit the process with status code
- **`process.getpid()`** - Get the current process ID
- **`process.env.get(key)`** - Get environment variable (simplified)

### Global Print Functions

- **`print(message)`** - Simple print without newline
- **`println(message)`** - Print with newline
- **`printf(format, ...args)`** - Printf-style formatting (simplified)

## Usage Examples

```typescript
// Basic logging
console.log("Hello, World!");
console.info("Application started");
console.warn("This is a warning");
console.error("Something went wrong");

// Timing operations
console.time("database-query");
// ... some operation ...
console.timeEnd("database-query");

// Assertions
console.assert(user !== null, "User should not be null");

// Grouping output
console.group("User Details");
console.log("Name: John Doe");
console.log("Email: john@example.com");
console.groupEnd();

// Clear screen
console.clear();

// Process utilities
let pid = process.getpid();
console.log(`Process ID: ${pid}`);
```

## Implementation Details

### Color Support

The library includes ANSI color codes for styled output:

- **Info messages**: Cyan with info icon (ℹ)
- **Warning messages**: Yellow with warning icon (⚠)
- **Error messages**: Red with error icon (✖)
- **Debug messages**: Dimmed with debug icon (🐛)
- **Trace messages**: Magenta with trace icon (📍)

### Syscall-Based Implementation

All functionality is implemented using direct syscalls:

- **`tspp_sys_write()`** - For all output operations
- **`tspp_sys_exit()`** - For process termination
- **`tspp_syscall0()`** - For process ID retrieval
- **`tspp_syscall1()`** - For time operations

### File Descriptors

- **STDOUT_FILENO (1)** - Standard output for normal messages
- **STDERR_FILENO (2)** - Standard error for warnings and errors

## Files

- **`console.tspp`** - Complete console library implementation
- **`console_demo.tspp`** - Demonstration program showing all features
- **`README.md`** - This documentation file

## Node.js Compatibility

This implementation aims to provide a familiar API for developers coming from Node.js:

| Node.js Function | TSPP Implementation | Status |
|------------------|---------------------|---------|
| `console.log()` | ✅ Implemented | Identical behavior |
| `console.info()` | ✅ Implemented | With icon enhancement |
| `console.warn()` | ✅ Implemented | With icon enhancement |
| `console.error()` | ✅ Implemented | With icon enhancement |
| `console.debug()` | ✅ Implemented | With timestamp |
| `console.trace()` | ✅ Implemented | Simplified |
| `console.time()` | ✅ Implemented | Simplified |
| `console.timeEnd()` | ✅ Implemented | Simplified |
| `console.assert()` | ✅ Implemented | Basic version |
| `console.clear()` | ✅ Implemented | ANSI escape codes |
| `console.count()` | ✅ Implemented | Simplified |
| `console.group()` | ✅ Implemented | Visual grouping |
| `console.table()` | ✅ Implemented | ASCII table |
| `process.exit()` | ✅ Implemented | Direct syscall |

## Benefits

1. **Familiar API**: Node.js developers can use familiar console functions
2. **Zero Dependencies**: No C++ runtime dependencies
3. **Direct Syscalls**: Maximum performance with minimal overhead
4. **Self-Contained**: Entire library implemented in TSPP
5. **Extensible**: Easy to add new console utilities
6. **Cross-Platform**: Syscall-based implementation works on Unix-like systems

## Building and Running

```bash
# Build the TSPP compiler
./build.sh

# Run the console demo
./build/src/tspp examples/console_demo.tspp
```

## Future Enhancements

- String interpolation support for template literals
- Advanced table formatting options
- Performance timing with high-resolution timers
- Stack trace capture for console.trace()
- Environment variable access
- Signal handling utilities
- File-based logging options

This console library demonstrates how TSPP's FFI and syscall support enables the creation of familiar, high-level APIs while maintaining direct system access and zero-overhead abstractions.