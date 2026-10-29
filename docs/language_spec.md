# tspp Language Overview

What the compiler accepts **today**, and what is planned. The grammar is in
[grammar.md](grammar.md); the tested feature list is the "Works today" table in
the [README](../README.md#works-today). Every ```` ```tspp ```` sample in this
file is compiled by `scripts/check_doc_samples.sh`; samples fenced as
`tspp-planned` show syntax that does not work yet.

## Types

| Type | Status |
|---|---|
| `int` | 32-bit signed integer. Works. |
| `boolean` / `bool` | Same type (LLVM `i1`). Works. |
| `string` | Garbage-collected, immutable C string. `+` concatenates strings, ints and booleans. Escape sequences are not processed yet; `==` compares pointers. |
| `float`, `double` | Arithmetic compiles; printing a float is broken (prints `0.000000`). |
| `void` | Function return type. |
| `T[]` | Arrays of `int` only; no bounds checks yet. |
| classes, generic classes | Works (see below). Generics are type-erased. |
| interfaces | Declarations and `implements` work; there is no dynamic dispatch through an interface type. |
| `typedef X = T;` / `type X = T;` | Aliases work; union and object types collapse to a single type. |
| `T*`, `@unsafe T*`, `@aligned T*` | Parsed only. Planned. |
| `#shared<T>`, `#unique<T>`, `#weak<T>` | Do not parse yet. Planned. |

## Variables and functions

```tspp
const LIMIT: int = 10;
let counter = 0;              // type inferred

function add(a: int, b: int = 1): int {
  return a + b;
}

function main(): int {
  counter = add(LIMIT);       // default argument: b = 1
  counter += 2;
  console.log("counter: " + counter);
  return 0;
}
```

`main` may return `int` (its value is the exit status) or `void` (exit 0).
A program without `main` runs only its top-level code.

## Control flow

`if`/`else`, `while` and C-style `for` work. Conditions must be boolean
expressions (`if (n != 0)`, not `if (n)` yet). `switch`, `do`/`while`,
`for..of`, `break`, `continue`, `try`/`catch`/`finally` and `throw` parse,
but code generation rejects them with an error.

Operators follow TypeScript/C precedence; see [grammar.md](grammar.md#expressions-with-precedence).

## Classes

```tspp
class Animal {
  protected let name: string;
  constructor(name: string) { this.name = name; }
  function speak(): string { return this.name + " makes a sound"; }
}

class Dog extends Animal {
  constructor(name: string) { super(name); }
  public #override function speak(): string { return this.name + " barks"; }
}

class Box<T> {
  public let value: T;
  constructor(v: T) { this.value = v; }
  function get(): T { return this.value; }
}

function main(): int {
  let d: Dog = new Dog("Rex");
  console.log(d.speak());
  let b: Box<string> = new Box<string>("boxed");
  console.log(b.get());
  return 0;
}
```

Class `#static` fields and `static { }` blocks run at startup (below).
Class and member modifiers (`#packed`, `#abstract`, `#final`, `#inline`,
`#virtual`, `#readonly`, `#volatile`, `#atomic`, `#constexpr`) are parsed and
have no effect yet.

## Imports and runtime calls

`import Lib from "./lib";` merges `lib.tspp` into the program; its names are
global (no namespaces). `import sys;` is optional documentation: `console.log`
and the `sys.*` runtime calls (files, environment, processes, HTTP, TCP) are
built in. The tested ones are listed in the README.

## Memory model (planned)

Variables can carry a storage region: `#stack`, `#heap` or `#static`. Today
the parser accepts them, `#static` gives a global internal linkage, and
`#heap`/`#stack` have no effect: locals live on the stack and objects created
with `new` live on the garbage-collected heap. The planned model, in order
(see [ROADMAP.md](../ROADMAP.md)):

```tspp-planned
#heap let big: Buffer = new Buffer(4096);   // explicit heap allocation
let owner: #unique<Buffer> = new Buffer(1); // single owner, freed at scope end
let rc: #shared<Buffer> = new Buffer(1);    // reference counted
let observer: #weak<Buffer> = rc;           // non-owning
@unsafe let raw: int* = @x;                 // raw pointer, unchecked
```

Performance attributes `#simd`, `#prefetch`, `#zerocast`, `#atomic`,
`#pinned`, `#const` and `#constexpr` are recorded on functions as LLVM string
attributes and do nothing yet.

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

