# TSPP++ Test Suite

This directory contains comprehensive tests for the TSPP++ language interpreter, organized by functional categories.

## Directory Structure

```
tests/
├── statements/     # Control flow and statement tests
├── expressions/    # Expression and operator tests  
├── types/         # Type system and aliasing tests
├── memory/        # Memory management and pointer tests
├── integration/   # End-to-end integration tests
├── semantics/     # Semantic analysis tests
└── README.md      # This file
```

## Running Tests

### Individual Test
```bash
./test.sh tests/category/test_name.tspp
```

### Category Tests
```bash
# Run all statement tests
find tests/statements -name "*.tspp" -exec ./test.sh {} \;

# Run all expression tests  
find tests/expressions -name "*.tspp" -exec ./test.sh {} \;

# Run all type tests
find tests/types -name "*.tspp" -exec ./test.sh {} \;

# Run all memory tests
find tests/memory -name "*.tspp" -exec ./test.sh {} \;

# Run all integration tests
find tests/integration -name "*.tspp" -exec ./test.sh {} \;
```

### All Tests
```bash
find tests -name "*.tspp" -not -path "*/.*" -exec ./test.sh {} \;
```

## Test Categories

### 📋 Statements (`statements/`)
Tests for control flow constructs and statement types:
- If/else conditionals
- While and for loops  
- Block statements
- Return statements
- Comprehensive statement combinations

### 🧮 Expressions (`expressions/`)
Tests for expressions and operators:
- Arithmetic operations
- Boolean logic
- Parentheses precedence
- Global expressions
- Complex expression evaluation

### 🏷️ Types (`types/`)
Tests for the type system:
- Type aliases and typedefs
- Storage qualifiers
- Union constraints
- Advanced type features

### 🧠 Memory (`memory/`)
Tests for memory management:
- Basic pointers
- Smart pointers
- Address operations
- Memory allocation
- Pointer syntax

### 🔗 Integration (`integration/`)
End-to-end tests combining multiple features:
- Global variable management
- Runtime initialization
- Program structure samples
- Complex feature interactions

### 🔍 Semantics (`semantics/`)
Semantic analysis and validation tests:
- Scope resolution
- Type checking
- Error detection

## Test File Naming Convention

- `basic_*.tspp` - Simple, fundamental tests
- `advanced_*.tspp` - Complex feature tests
- `comprehensive_*.tspp` - Multi-feature combination tests
- `*_test.tspp` - Standard test naming
- `*_sample.tspp` - Example/demo files

## Expected Output

Each test should:
1. ✅ Parse successfully (tokenize and build AST)
2. ✅ Pass semantic analysis
3. ✅ Generate valid LLVM IR
4. ✅ Compile and execute correctly
5. ✅ Produce expected runtime output

## Adding New Tests

1. Create test file in appropriate category directory
2. Follow naming convention: `feature_description_test.tspp`
3. Include comments describing what is being tested
4. Add expected behavior documentation
5. Run test to verify it works correctly

## Test Status

- ✅ **Statements**: All major statement types working
- ✅ **Expressions**: Arithmetic and boolean expressions working  
- ✅ **Types**: Basic type system working
- ✅ **Memory**: Basic pointer operations working
- ✅ **Integration**: Core language features working
- 🔄 **Semantics**: Ongoing improvements
