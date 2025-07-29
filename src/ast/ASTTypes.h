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

}  // namespace ast
