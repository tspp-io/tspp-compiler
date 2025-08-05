# Integration Tests

This directory contains end-to-end integration tests that combine multiple TSPP++ language features to validate complete program functionality.

## Test Files

### Program Structure

#### `program_structure_sample.tspp` ✅
- **Purpose**: Demonstrates complete program structure
- **Features**: Global declarations, functions, main entry point
- **Expected Output**: Well-formed program execution
- **Status**: Working ✅

#### `simple_runtime_init.tspp` ✅
- **Purpose**: Tests runtime initialization patterns
- **Features**: Variable initialization, startup behavior
- **Expected Output**: Proper initialization sequence
- **Status**: Working ✅

### Global Management

#### `global_vars_test.tspp` ✅
- **Purpose**: Tests global variable management
- **Features**: Global variable declaration, access, modification
- **Expected Output**: Global state management working
- **Status**: Working ✅

## Integration Scenarios

### Complete Program Flow
```typescript
// Global declarations
let globalCounter: int = 0;
let applicationName: string = "TSPP++ App";

// Main entry point
function main(): bool {
    // Local variables
    let localData: int = 42;
    
    // Control flow
    if (globalCounter == 0) {
        console.log("First run");
        globalCounter = 1;
    }
    
    // Loops
    while (localData > 0) {
        console.log(localData);
        localData = localData - 1;
    }
    
    return true;
}
```

### Multi-Feature Integration
```typescript
// Types and memory
type UserID = int;
let currentUser: UserID = 123;
let userPtr: UserID* = &currentUser;

// Functions and expressions
function calculateScore(base: int, multiplier: int): int {
    return base * multiplier + globalBonus;
}

// Comprehensive logic
function processData(): bool {
    // Expressions
    let result: int = calculateScore(10, 5);
    
    // Statements
    if (result > 50) {
        console.log("High score achieved");
    } else {
        console.log("Try again");
    }
    
    return true;
}
```

## Test Categories

### 🏗️ **Structure**: Complete program organization
### 🌐 **Global**: Global variable and state management  
### ⚡ **Runtime**: Initialization and execution flow
### 🔗 **Multi-Feature**: Combining multiple language features
### 📊 **Real-World**: Practical usage scenarios

## Integration Aspects Tested

### Language Features
- ✅ **Variable Declarations**: Global and local variables
- ✅ **Function Definitions**: Function declaration and calls
- ✅ **Type System**: Type aliases and basic types
- ✅ **Control Flow**: If statements, loops, returns
- ✅ **Expressions**: Arithmetic and logical operations
- ✅ **Memory**: Basic pointer operations
- ✅ **I/O**: Console output functionality

### Program Structure
- ✅ **Entry Point**: main() function as program entry
- ✅ **Global Scope**: Global variable and function declarations
- ✅ **Local Scope**: Function-local variable management
- ✅ **Initialization**: Variable and system initialization
- ✅ **Execution Flow**: Sequential program execution

### System Integration
- ✅ **Parsing**: Complete source file parsing
- ✅ **Semantic Analysis**: Full program semantic validation
- ✅ **Code Generation**: Complete LLVM IR generation
- ✅ **Compilation**: LLVM IR to executable compilation
- ✅ **Runtime**: Program execution and output

## Common Integration Patterns

### Application Structure
```typescript
// Configuration and globals
let config: ApplicationConfig;
let isInitialized: bool = false;

// Initialization
function initialize(): bool {
    console.log("Initializing application...");
    isInitialized = true;
    return true;
}

// Main application logic
function main(): bool {
    if (!initialize()) {
        return false;
    }
    
    // Application logic here
    console.log("Application running");
    
    return true;
}
```

### Data Processing Pipeline
```typescript
// Input data
let inputData: int = 100;
let processedData: int = 0;

// Processing functions
function validateInput(data: int): bool {
    return data > 0;
}

function processData(input: int): int {
    return input * 2 + 10;
}

// Main pipeline
function main(): bool {
    if (validateInput(inputData)) {
        processedData = processData(inputData);
        console.log("Result:");
        console.log(processedData);
    }
    return true;
}
```

## Validation Criteria

### Functional Requirements
1. **Complete Compilation**: Programs compile without errors
2. **Semantic Correctness**: All references and types resolve correctly
3. **Runtime Execution**: Programs run and produce expected output
4. **Feature Integration**: Multiple features work together seamlessly

### Quality Metrics
1. **LLVM IR Quality**: Generated IR is well-formed and efficient
2. **Memory Safety**: No memory leaks or dangling pointers
3. **Type Safety**: All operations are type-safe
4. **Performance**: Reasonable compilation and execution speed

## Known Working Integrations

✅ **Variables + Functions**: Global and local variable access in functions
✅ **Types + Expressions**: Type-safe expression evaluation
✅ **Control Flow + I/O**: Conditional console output
✅ **Memory + Types**: Typed pointer operations
✅ **Functions + Statements**: Statement sequences in functions
✅ **Global + Local**: Mixed global and local variable usage

## Future Integration Tests

1. **Modules**: Multi-file program integration
2. **Libraries**: External library integration
3. **Error Handling**: Exception and error propagation
4. **Concurrency**: Multi-threaded program integration
5. **Interop**: C/C++ library integration
6. **Package System**: Package and namespace integration

## Running Integration Tests

```bash
# Run all integration tests
find tests/integration -name "*.tspp" -exec ./test.sh {} \;

# Run specific integration test
./test.sh tests/integration/program_structure_sample.tspp
./test.sh tests/integration/global_vars_test.tspp

# Run with verbose output
./test.sh tests/integration/simple_runtime_init.tspp -v
```

## Success Criteria

### Compilation Success
- ✅ Lexical analysis completes without errors
- ✅ Parsing produces valid AST
- ✅ Semantic analysis passes all checks
- ✅ LLVM IR generation succeeds
- ✅ LLVM compilation to executable succeeds

### Runtime Success  
- ✅ Program executes without crashes
- ✅ Expected output is produced
- ✅ Return values are correct
- ✅ Memory usage is reasonable
- ✅ No runtime errors occur

## Integration Test Philosophy

1. **Holistic Validation**: Test complete program functionality
2. **Real-World Scenarios**: Use practical programming patterns
3. **Feature Combination**: Ensure features work together
4. **End-to-End Coverage**: From source to execution
5. **Quality Assurance**: Validate production readiness
