# Memory Management Tests

This directory contains tests for TSPP++ memory management features including pointers, smart pointers, and memory operations.

## Test Files

### Basic Pointers

#### `basic_pointers_working.tspp` ✅
- **Purpose**: Tests fundamental pointer operations
- **Features**: Pointer declaration, dereferencing, address-of operator
- **Expected Output**: Basic pointer functionality working
- **Status**: Working ✅

#### `basic_pointers_simple.tspp` ✅
- **Purpose**: Tests simple pointer scenarios
- **Features**: Simple pointer usage patterns
- **Expected Output**: Basic pointer operations
- **Status**: Working ✅

#### `basic_pointers_test.tspp` ⚠️
- **Purpose**: Tests comprehensive pointer functionality
- **Features**: Advanced pointer operations
- **Status**: May need refinement ⚠️

### Address Operations

#### `basic_address_test.tspp` ✅
- **Purpose**: Tests address-of operator (&) functionality
- **Features**: Taking addresses of variables, pointer assignment
- **Expected Output**: Address operations working correctly
- **Status**: Working ✅

#### `pointer_syntax_test.tspp` ✅
- **Purpose**: Tests pointer syntax variations
- **Features**: Different pointer declaration styles
- **Expected Output**: Syntax parsing working
- **Status**: Working ✅

### Smart Pointers

#### `smart_pointer_test.tspp` ✅
- **Purpose**: Tests smart pointer functionality
- **Features**: shared, unique, weak pointer types
- **Expected Output**: Smart pointer declarations
- **Status**: Working ✅

### Memory Types

#### `memory_types_test.tspp` ✅
- **Purpose**: Tests memory allocation types
- **Features**: stack, heap, static memory qualifiers
- **Expected Output**: Memory type declarations working
- **Status**: Working ✅

## Memory Concepts

### Pointer Basics
```typescript
// Basic pointer declaration and usage
let value: int = 42;
let ptr: int* = &value;  // Take address of value
let deref: int = *ptr;   // Dereference pointer
```

### Smart Pointers
```typescript
// Automatic memory management
shared let sharedPtr: int;    // Reference counted
unique let uniquePtr: int;    // Exclusive ownership  
weak let weakPtr: int;        // Non-owning reference
```

### Memory Qualifiers
```typescript
// Explicit memory allocation
stack let stackVar: int = 10;   // Stack allocated
heap let heapVar: int = 20;     // Heap allocated
static let staticVar: int = 30; // Static storage
```

## Test Categories

### 🔗 **Basic Pointers**: Fundamental pointer operations
### 📍 **Addresses**: Address-of and dereferencing operations
### 🧠 **Smart Pointers**: Automatic memory management
### 💾 **Memory Types**: Storage class specifiers
### 🏗️ **Complex**: Advanced memory scenarios

## Common Patterns

### Pointer Operations
```typescript
let data: int = 100;
let ptr: int* = &data;

function main(): bool {
    console.log(*ptr);  // Dereference to get value
    *ptr = 200;         // Modify through pointer
    console.log(data);  // Shows modified value
    return true;
}
```

### Smart Pointer Usage
```typescript
shared let resource: int;

function allocateResource(): bool {
    // Smart pointer automatically manages lifetime
    resource = 42;
    return true;
}
```

## Memory Safety Features

### ✅ **Type Safety**: Pointer types are type-checked
### ✅ **Address Operations**: & and * operators working
### ✅ **Smart Pointers**: Basic smart pointer syntax supported
### ✅ **Storage Classes**: Memory qualifiers working
### 🔄 **Ownership**: Advanced ownership semantics in development

## Pointer Syntax Variations

```typescript
// Different pointer declaration styles
let ptr1: int* = &value;      // C-style
let ptr2: *int = &value;      // Alternative style
let ptr3: int * = &value;     // With spaces
```

## Smart Pointer Types

```typescript
// Reference counting (shared ownership)
shared let sharedResource: ResourceType;

// Exclusive ownership (unique)  
unique let uniqueResource: ResourceType;

// Non-owning reference (weak)
weak let weakReference: ResourceType;
```

## Memory Management Philosophy

1. **Safety First**: Prevent memory leaks and dangling pointers
2. **Performance**: Minimal overhead for memory operations
3. **Clarity**: Clear ownership semantics
4. **Compatibility**: C-style pointers for interop

## Known Capabilities

✅ **Basic Pointers**: Declaration, address-of, dereference working
✅ **Type Safety**: Pointer type checking implemented
✅ **Smart Pointer Syntax**: Declaration syntax supported
✅ **Memory Qualifiers**: stack, heap, static working
✅ **Address Operations**: & and * operators functional

## Future Enhancements

1. **Automatic Deallocation**: Smart pointer lifetime management
2. **Reference Counting**: Shared pointer implementation
3. **Move Semantics**: Unique pointer transfers
4. **Weak References**: Weak pointer cycle breaking
5. **Array Pointers**: Pointer arithmetic for arrays
6. **Function Pointers**: Callable pointer types

## Running Memory Tests

```bash
# Run all memory tests
find tests/memory -name "*.tspp" -exec ./test.sh {} \;

# Run pointer tests
./test.sh tests/memory/basic_pointers_working.tspp
./test.sh tests/memory/basic_address_test.tspp

# Run smart pointer tests  
./test.sh tests/memory/smart_pointer_test.tspp
```

## Memory Model

```
Memory Management
├── Raw Pointers
│   ├── Basic Pointers (T*)
│   ├── Address Operations (&, *)
│   └── Pointer Arithmetic
├── Smart Pointers  
│   ├── shared<T> (Reference Counted)
│   ├── unique<T> (Exclusive Ownership)
│   └── weak<T> (Non-owning)
└── Storage Classes
    ├── stack (Automatic)
    ├── heap (Dynamic)
    └── static (Global)
```

## Safety Considerations

- **Null Pointer Checks**: Always validate pointer before dereferencing
- **Dangling Pointers**: Avoid using pointers after target deallocation
- **Memory Leaks**: Use smart pointers for automatic cleanup
- **Type Safety**: Maintain type consistency in pointer operations
