# Type System Tests

This directory contains tests for TSPP++ type system features including type aliases, storage qualifiers, and advanced type constructs.

## Test Files

### Type Aliases

#### `type_alias_test.tspp` ✅
- **Purpose**: Tests basic type aliasing functionality
- **Features**: `type` keyword, alias definitions, usage
- **Expected Output**: Type aliases work correctly
- **Status**: Working ✅

#### `advanced_typedef_test.tspp` ✅
- **Purpose**: Tests advanced typedef features
- **Features**: Complex type definitions, nested types
- **Expected Output**: Advanced type aliasing
- **Status**: Working ✅

#### `final_typedef_test.tspp` ✅
- **Purpose**: Tests comprehensive typedef scenarios
- **Features**: Multiple aliases, complex type chains
- **Expected Output**: Complete typedef functionality
- **Status**: Working ✅

### Storage Qualifiers

#### `storage_test.tspp` ✅
- **Purpose**: Tests storage class specifiers
- **Features**: `stack`, `heap`, `static` qualifiers
- **Expected Output**: Proper storage allocation
- **Status**: Working ✅

### Advanced Types

#### `union_constraint_test.tspp` ⚠️
- **Purpose**: Tests union type constraints
- **Features**: Union types, type constraints
- **Expected Output**: Union type functionality
- **Status**: Implementation pending ⚠️

## Type System Features

### Basic Types
```typescript
// Primitive types
let intVar: int = 42;
let floatVar: float = 3.14;
let boolVar: bool = true;
let stringVar: string = "hello";
```

### Type Aliases
```typescript
// Simple alias
type MyInt = int;
let value: MyInt = 100;

// Complex alias
type PersonID = int;
type Name = string;
```

### Storage Qualifiers
```typescript
// Stack allocation (default)
stack let localVar: int = 10;

// Heap allocation
heap let dynamicVar: int = 20;

// Static storage
static let globalVar: int = 30;
```

### Pointer Types
```typescript
// Basic pointer
let ptr: int* = &variable;

// Smart pointers
shared let sharedPtr: int;
unique let uniquePtr: int;
weak let weakPtr: int;
```

## Test Categories

### 🏷️ **Aliases**: Type alias definitions and usage
### 💾 **Storage**: Memory storage qualifiers
### 🔗 **Pointers**: Pointer type declarations
### 🧩 **Complex**: Advanced type combinations
### ⚡ **Smart**: Smart pointer types

## Common Patterns

### Type Definition
```typescript
// Define custom types
type UserID = int;
type Username = string;
type Score = float;

function main(): bool {
    let id: UserID = 123;
    let name: Username = "player";
    let points: Score = 95.5;
    return true;
}
```

### Storage Classes
```typescript
// Different storage durations
static let globalCounter: int = 0;
stack let localTemp: int = 42;
heap let dynamicBuffer: int = 1024;
```

## Type Safety Features

### ✅ **Type Checking**: Compile-time type validation
### ✅ **Alias Resolution**: Type aliases resolve correctly
### ✅ **Storage Classes**: Different storage qualifiers supported
### ✅ **Pointer Safety**: Basic pointer type checking
### 🔄 **Advanced Types**: Union and generic types in development

## Semantic Analysis

1. **Type Resolution**: All types resolve to concrete types
2. **Compatibility**: Type compatibility checking
3. **Scope Rules**: Type visibility and scoping
4. **Error Detection**: Invalid type usage detection

## Known Capabilities

✅ **Basic Types**: int, float, bool, string supported
✅ **Type Aliases**: typedef functionality working
✅ **Storage Qualifiers**: stack, heap, static working
✅ **Pointer Types**: Basic pointer declarations working
✅ **Type Resolution**: Semantic analysis working

## Future Enhancements

1. **Generic Types**: Template/generic type support
2. **Union Types**: Discriminated union types
3. **Interface Types**: Interface definitions
4. **Enum Types**: Enumeration types
5. **Array Types**: Fixed-size and dynamic arrays
6. **Function Types**: Function pointer types

## Running Type Tests

```bash
# Run all type tests
find tests/types -name "*.tspp" -exec ./test.sh {} \;

# Run specific categories
./test.sh tests/types/type_alias_test.tspp
./test.sh tests/types/storage_test.tspp
./test.sh tests/types/advanced_typedef_test.tspp
```

## Type Hierarchy

```
Type System
├── Primitive Types
│   ├── int
│   ├── float  
│   ├── bool
│   └── string
├── Compound Types
│   ├── Pointers (T*)
│   ├── Arrays (T[])
│   └── Functions (T -> U)
├── Smart Pointers
│   ├── shared<T>
│   ├── unique<T>
│   └── weak<T>
└── User Defined
    ├── Type Aliases
    ├── Interfaces
    └── Enums
```
