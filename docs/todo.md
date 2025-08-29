# Remaining to implement (prioritized)

> **Note**: For the comprehensive roadmap including Go semantics, built-in functions, and standard library implementation, see the [TODO section in README.md](../README.md#todo---roadmap-to-full-implementation).

## Current Language Feature Priorities

## Method/field modifiers beyond what’s wired
- Method: #static, #abstract, #virtual, #override, #inline (currently skipped as generic attributes).
- Field: #readonly, #volatile, #atomic, #constexpr (only storage is captured now).
- Actions:
  - Add tokens for INLINE, VIRTUAL, OVERRIDE, READONLY, VOLATILE (and map in attribute scanner).
  - Extend AST with explicit method/field modifiers (bitset or vector).
  - Parse and attach these modifiers; add parse-only tests.

## Class modifiers parity with grammar
- Add #final (token + parse); allow multiple modifiers (#abstract #packed #pinned #final).
- Optionally store all class modifiers (vector/bitset) instead of a single enum.

## Generics capture (currently skipped)
- Class/interface/function TypeParameters (<T extends Base>), TypeArguments, TypeConstraint.
- Add AST nodes for TypeParam and generic lists; update builders to populate them.
- Parse-only tests; codegen can ignore for now.

## Type system completeness
- PointerType with @unsafe/@aligned and trailing *.
- SmartPointerType: #shared/#unique/#weak<T>.
- UnionType: T1 | T2 | ...
- TypeConstraint: Type extends Type.
- Actions:
  - Finish TypeBuilder for these nodes (placeholders exist with TODOs).
  - Add targeted tests (types/expression contexts).

## Static block node (optional)
- If you need semantic/codegen distinction, add a StaticBlock AST node (not just a BlockStmt) and store it on ClassDecl.

## Import at top level
- Wire DeclarationBuilder::buildImport into the main switch so Import is accepted per grammar (compile-only tests).
