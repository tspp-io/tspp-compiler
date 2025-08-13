#pragma once

namespace ast {

enum class StorageQualifier { None, Stack, Heap, Static };

enum class FunctionModifier {
  None,
  Const,
  Constexpr,
  Simd,
  Zerocast,
  Prefetch,
  Atomic,
  Pinned
};

enum class MethodAttribute {
  None,
  Inline,
  Virtual,
  Override,
  Static,
  Abstract
};
enum class FieldModifier { None, Readonly, Volatile, Atomic, Constexpr };
enum class ClassModifier { None, Packed, Abstract, Pinned, Final };

}  // namespace ast
