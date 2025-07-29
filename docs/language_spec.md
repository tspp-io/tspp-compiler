# TSPP++ Language Specification

## Memory Model
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
