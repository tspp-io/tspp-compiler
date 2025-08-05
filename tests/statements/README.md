# Statement Tests

This directory contains tests for all TSPP++ statement types and control flow constructs.

## Test Files

### Core Statement Types

#### `comprehensive_statements_test.tspp` ✅
- **Purpose**: Tests all major statement types working together
- **Features**: Block, If-else, While, For, Expression, Return statements
- **Expected Output**: Demonstrates proper execution order and variable scoping
- **Status**: Working ✅

#### `while_loop_test.tspp` ✅  
- **Purpose**: Tests while loop functionality
- **Features**: Loop condition evaluation, iteration control, variable updates
- **Expected Output**: Loop iterations 0,1,2 then final counter value 3
- **Status**: Working ✅

#### `for_loop_simple_test.tspp` ✅
- **Purpose**: Tests for loop with pre-declared variables
- **Features**: C-style for loop with initialization, condition, increment
- **Expected Output**: Loop iterations 0,1,2,3,4 then final value 5
- **Status**: Working ✅

#### `for_loop_test.tspp` ⚠️
- **Purpose**: Tests for loop with inline variable declaration
- **Features**: `for (let i: int = 0; i < 5; i = i + 1)`
- **Status**: Needs parser enhancement for inline declarations ⚠️

### Conditional Statements

#### `if_else_test.tspp` ✅
- **Purpose**: Tests basic if-else functionality
- **Features**: Simple conditionals, boolean evaluation
- **Status**: Working ✅

#### `if_else_variations_test.tspp` ⚠️
- **Purpose**: Tests complex nested if-else statements
- **Features**: Nested conditionals, complex control flow
- **Status**: Minor issue with deeply nested statements ⚠️

#### `simple_nested_if_test.tspp` ⚠️
- **Purpose**: Tests simple nested if statements
- **Features**: If statements inside if statements
- **Status**: Minor LLVM IR terminator issue ⚠️

## Common Patterns

### Basic Structure
```typescript
// Variable declarations at top level
let variable: int = value;

function main(): bool {
    // Statement tests here
    return true;
}
```

### Testing Approach
1. **Isolation**: Each test focuses on specific statement type
2. **Progression**: Simple to complex variations
3. **Integration**: Comprehensive tests combine multiple statements
4. **Validation**: Runtime output confirms correct behavior

## Known Issues

1. **Nested If Statements**: Deep nesting can create basic blocks without proper terminators
2. **For Loop Declarations**: Inline variable declarations in for loops need parser enhancement
3. **Complex Control Flow**: Some edge cases in deeply nested statements

## Future Enhancements

1. **Switch Statements**: Add support for switch-case constructs
2. **Break/Continue**: Add loop control statements
3. **Try-Catch**: Add exception handling statements
4. **Enhanced For**: Support for-in and for-of style loops

## Running Statement Tests

```bash
# Run all statement tests
find tests/statements -name "*.tspp" -exec ./test.sh {} \;

# Run individual test
./test.sh tests/statements/comprehensive_statements_test.tspp

# Run specific category
./test.sh tests/statements/while_loop_test.tspp
./test.sh tests/statements/for_loop_simple_test.tspp
```
