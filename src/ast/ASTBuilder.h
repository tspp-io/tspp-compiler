#pragma once
#include <vector>

#include "core/common/common_types.h"
#include "core/common/macros.h"
#include "parser/nodes/base_node.h"
#include "tokens/tokens.h"

namespace ast {

template <typename T, typename... Args>
Shared(T) makeNodeWithLoc(const tokens::Token& token, Args&&... args) {
  auto node = MakeShared<T>(std::forward<Args>(args)...);
  node->location = token.location;
  return node;
}

template <typename T, typename... Args>
std::vector<Shared(T)> makeNodeList(Args&&... args) {
  return {MakeShared<T>(std::forward<Args>(args))...};
}

}  // namespace ast
