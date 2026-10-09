# tspp Language Specification

> **Note**: This specification covers currently implemented features. For planned Go semantics, concurrency primitives, and standard library features, see the [comprehensive TODO in README.md](../README.md#todo---roadmap-to-full-implementation).

## Current Implementation Status

### Memory Model
- `#stack`, `#heap`, `#static`: Explicit memory region selection for variables and objects.
- Example: `#stack int x = 5;`

## Pointer Types
- `@unsafe`: Raw pointer, unchecked access.
- `@aligned`: Pointer with alignment guarantees.
- Example: `@unsafe int* ptr;`

## Smart Pointers
- `#shared<T>`, `#unique<T>`, `#weak<T>`: Ownership and lifetime management.
- Example: `#shared<MyClass> obj;`

## SIMD and Cache Control
- `#simd`: Vectorized operations.
- `#prefetch`: Manual cache prefetching.
- Example: `#simd float[4] v; #prefetch v;`

## Compile-Time Execution
- `#const`, `#constexpr`: Compile-time evaluation and constants.
- Example: `#const int N = 10;`

## Zero-Cost Abstraction
- `#zerocast`: Inline generics, no runtime overhead.
- Example: `#zerocast function<T>(T x) { ... }`

## Safety and Concurrency
- `#unsafe`: Mark unsafe code blocks.
- `#atomic`: Atomic operations for concurrency.
- `#pinned`: Prevents object movement in memory.
- Example: `#atomic int counter; #pinned MyObject obj;`

## Type System and Optimization
- Union types: `int | string`
- Type constraints: `T extends BaseType`
- Performance hints: `#simd`, `#prefetch`, `#zerocast`

## Example
```tspp
#stack int x = 5;
@unsafe int* ptr = &x;
#shared<MyClass> obj;
#simd float[4] v;
#const int N = 10;
function add(a: int, b: int): int { return a + b; }
```

## Program Startup and Initialization Order

Before `main` runs, the program executes its startup code once, in this order:

1. The runtime is initialized (`tspp_runtime_init`: Boehm GC).
2. Every top-level item that has code to run, in source order:
   - a global whose initializer is not a compile-time constant
     (`let g: int = compute();`); constant initializers are folded into the
     global and need no code;
   - a class's non-constant `#static` field initializers and its `static { }`
     blocks, in the order they appear in the class body;
   - a top-level statement (`console.log("hi");`, `if`, `for`, `{ ... }`).
3. `main`, if there is one.

A global initializer can read any global declared before it. Reading a global
declared later gives its zero value (or its constant initializer).

Imports are merged into the importing file (no namespaces). Code from an
imported module runs before the code of the file that imports it. Several
imports of one file run in the order they are written. A module imported more
than once runs once, at the position of its first merge (with diamond imports
this may be after a module that also imports it).

