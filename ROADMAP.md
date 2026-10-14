# tspp Roadmap

tspp aims to be *TypeScript syntax with C++-level memory control, compiled to
native via LLVM. Safe by default, unsafe when you ask for it.* This file says
what comes next. What works today is the "Works today" table in the
[README](README.md#works-today); bug IDs (H5, M5, ...) are defined in
[docs/KNOWLEDGE_TRANSFER.md §9](docs/KNOWLEDGE_TRANSFER.md#9-known-issues-and-technical-debt).

This replaces `docs/todo.md` and `docs/todo.ebnf`. Their still-open items are
folded in below; the rest (import at top level, generics capture, class and
member modifier parsing, `#final`, multiple class modifiers, static blocks)
is done.

## Done: stabilization (2026)

The compiler was made honest before adding features: a golden-output test
suite (`tests/run.sh`) that gates CI, a driver that never invokes a shell,
GC-owned runtime strings, global initializers and top-level code that
actually run, located diagnostics and IR verification, TypeScript/C operator
precedence, `boolean` = `bool`, an honest REPL, and Debug/Release builds with
optional sanitizers. Details: [docs/STABILIZATION_PLAN.md](docs/STABILIZATION_PLAN.md).

## Next: the memory model, in order

Each step lands with tests, one branch per step.

1. **Typed arrays with bounds checks (H5).** Arrays are `i32`-only today
   (floats truncated, pointers squeezed into 32 bits) and unchecked. Give
   arrays an element type and a length, and check every index.
2. **Split `LLVMCodeGenerator.cpp` (M5).** ~5k lines, with ~1.1k-line
   `visit(BinaryExpr&)` and `visit(CallExpr&)`. Split by concern (expressions,
   statements, calls/builtins, classes, runtime ABI) and replace type-name
   string matching with real type objects where it is touched.
3. **Real `#heap`.** Today `#stack`/`#heap`/`#static` parse and `#heap` is
   ignored. Define what `#heap` means next to GC-allocated `new` and
   implement it.
4. **`#unique<T>`.** Single ownership, freed at end of scope, moves. First fix
   parsing (H12): `#shared`/`#unique`/`#weak` lex as one token the type parser does
   not expect, and `>>` closing nested generics lexes as a shift
   (`tests/types/pointer_union_smart_types.tspp`, xfail).
5. **`#shared<T>` / `#weak<T>`.** Reference counting and non-owning references.
6. **`@unsafe` / `@aligned` pointer semantics.** Raw pointers, address-of
   (`@x`) and dereference, allowed only where explicitly marked unsafe.

## Open bugs to fix along the way

Ordered roughly by how often they bite. Each has (or should get) an xfail
test that turns into a pass when fixed.

| ID | Bug | Pinned by |
|---|---|---|
| M8 | A union-typed global with a string initializer is invalid IR | `tests/sample.test.tspp` (xfail) |
| M9 | Some string expressions print an address: string-typed `?:` inside `+`, static string fields and static methods returning `string` inside `+` | — |
| M3a | A local redeclared in the same scope is not reported | — |
| M4 | Imports: no namespaces, `import { x } from` unsupported, a missing import is only a warning, weak cycle detection, diamond imports can run a shared module after one importer | — |
| M11 | `lib/system.tspp` defines `exit`/`write`/`read`/`strlen` globally, shadowing libc | — |
| L5 | Parser fragility: `@` lexes as a binary operator; `<...>` after a binary operator is taken as generic arguments; parameters typed `#shared<int>` end the parameter list early | — |

## Language features after the memory model

- Lower the statements that parse but are rejected by codegen: `break`,
  `continue`, `switch`, `do`/`while`, `for..of`; then `try`/`catch`/`throw`.
- `enum` and `namespace` (rejected by the parser today).
- Give the parsed method and field modifiers meaning: `#static`,
  `#abstract`, `#virtual`, `#override`, `#inline`, `#readonly`, `#volatile`,
  `#atomic`, `#constexpr`; class modifiers `#abstract`, `#packed`, `#pinned`,
  `#final`.
- Union types, type constraints (`T extends X`) and object types in the
  checker (they collapse to one type today).
- Dynamic dispatch through interfaces.
- Real generics (today: type erasure to an opaque pointer).
- `#simd`, `#zerocast`, `#prefetch`: define semantics, then implement
  (today they are LLVM string attributes with no effect).

## Future ideas (not planned)

- Go-style concurrency: goroutines (`go`), channels, `select`, `defer`.
- A standard library beyond the `sys.*` runtime calls (formatting, strings,
  collections, networking).
- An optimization pipeline of our own (today `clang -O3` optimizes the IR),
  escape analysis for stack allocation.
- A REPL that keeps state between inputs.
- Cross-compilation; `__builtin_syscall` is x86-64 only (M6).
