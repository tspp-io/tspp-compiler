# Expression Tests

This directory contains tests for TSPP++ expressions, operators, and arithmetic operations.

## Test Files

### Basic Expressions

#### `simple_expression_test.tspp` ✅
- **Purpose**: Tests basic arithmetic expressions
- **Features**: Addition, subtraction, multiplication, division
- **Expected Output**: Correct arithmetic evaluation
- **Status**: Working ✅

#### `expression_test.tspp` ✅
- **Purpose**: Tests complex expression evaluation
- **Features**: Mixed arithmetic, operator precedence
- **Expected Output**: Proper order of operations
- **Status**: Working ✅

#### `comprehensive_expression_test.tspp` ✅
- **Purpose**: Tests wide range of expression types
- **Features**: All operators, complex combinations
- **Expected Output**: Comprehensive expression evaluation
- **Status**: Working ✅

### Parentheses and Precedence

#### `arithmetic_parentheses_test.tspp` ✅
- **Purpose**: Tests parentheses in arithmetic expressions
- **Features**: Override operator precedence with parentheses
- **Expected Output**: Correct evaluation with parentheses
- **Status**: Working ✅

#### `complex_parentheses_test.tspp` ✅
- **Purpose**: Tests deeply nested parentheses
- **Features**: Multiple levels of parentheses nesting
- **Expected Output**: Proper nested evaluation
- **Status**: Working ✅

### Global Context

#### `global_expressions_test.tspp` ✅
- **Purpose**: Tests expressions in global scope
- **Features**: Global variable expressions, initialization
- **Expected Output**: Global expression evaluation
- **Status**: Working ✅

## Expression Types Tested

### Arithmetic Operators
- `+` Addition
- `-` Subtraction  
- `*` Multiplication
- `/` Division
- `%` Modulus

### Comparison Operators
- `==` Equality
- `!=` Inequality
- `<` Less than
- `>` Greater than
- `<=` Less than or equal
- `>=` Greater than or equal

### Logical Operators
- `&&` Logical AND
- `||` Logical OR
- `!` Logical NOT

### Assignment Operators
- `=` Simple assignment
- `+=` Addition assignment
- `-=` Subtraction assignment
- `*=` Multiplication assignment
- `/=` Division assignment

## Common Patterns

### Test Structure
```typescript
let operand1: int = value1;
let operand2: int = value2;
let result: int = 0;

function main(): bool {
    // Expression tests
    result = operand1 + operand2 * operand3;
    console.log(result);
    return true;
}
```

### Validation Approach
1. **Operand Setup**: Initialize test values
2. **Expression Evaluation**: Perform operations
3. **Result Output**: Display computed values
4. **Verification**: Confirm expected results

## Test Categories

### 🔢 **Arithmetic**: Basic math operations
### 🔍 **Comparison**: Relational operators
### 🧠 **Logical**: Boolean operations
### 📝 **Assignment**: Variable assignment operators
### 🌐 **Global**: Global scope expressions
### 🏗️ **Complex**: Multi-operator expressions

## Expected Behavior

1. **Correct Precedence**: Operators follow standard precedence rules
2. **Parentheses Override**: Parentheses properly override precedence
3. **Type Safety**: Expressions maintain type consistency
4. **Evaluation Order**: Left-to-right evaluation where appropriate

## Known Features

✅ **Integer Arithmetic**: All basic integer operations working
✅ **Boolean Logic**: Logical operators functioning correctly  
✅ **Comparison**: Relational operators working
✅ **Precedence**: Operator precedence properly implemented
✅ **Parentheses**: Grouping expressions working
✅ **Global Expressions**: Global scope evaluation working

## Future Enhancements

1. **Floating Point**: Add float/double arithmetic support
2. **String Operations**: String concatenation and comparison
3. **Bitwise Operators**: Add bitwise AND, OR, XOR, shift operations
4. **Ternary Operator**: Add conditional operator `condition ? true_val : false_val`
5. **Increment/Decrement**: Add `++` and `--` operators

## Running Expression Tests

```bash
# Run all expression tests
find tests/expressions -name "*.tspp" -exec ./test.sh {} \;

# Run specific tests
./test.sh tests/expressions/comprehensive_expression_test.tspp
./test.sh tests/expressions/arithmetic_parentheses_test.tspp
```
