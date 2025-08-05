#pragma once

#include "core/common/macros.h"
#include "parser/nodes/expression_nodes.h"

namespace tspp_std {

// Standard library function declarations for memory management
class MemoryStd {
 public:
  // Smart pointer factory functions
  static void registerMakeShared();
  static void registerMakeUnique();
  static void registerMakeWeak();

  // Smart pointer utility functions
  static void registerSmartPtrValid();
  static void registerSmartPtrReset();
  static void registerSmartPtrGet();

  // Memory management functions
  static void registerAlloc();
  static void registerFree();
  static void registerDestroy();

  // Register all memory functions
  static void registerAll();
};

}  // namespace tspp_std
