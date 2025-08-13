#pragma once
#include <vector>

#include "ast_visitor.h"
#include "base_node.h"
#include "core/common/macros.h"
#include "tokens/tokens.h"

namespace ast {

#define AST_ACCEPT_IMPL(NodeType)             \
  void accept(ASTVisitor& visitor) override { \
    visitor.visit(*this);                     \
  }

class BasicTypeNode : public TypeNode {
 public:
  tokens::Token name;
  AST_ACCEPT_IMPL(BasicTypeNode);
};

class PointerTypeNode : public TypeNode {
 public:
  tokens::Token qualifier;  // @unsafe or @aligned
  Shared(TypeNode) baseType;
  AST_ACCEPT_IMPL(PointerTypeNode);
};

class SmartPointerTypeNode : public TypeNode {
 public:
  tokens::Token kind;  // shared / unique / weak
  Shared(TypeNode) target;
  AST_ACCEPT_IMPL(SmartPointerTypeNode);
};

class UnionTypeNode : public TypeNode {
 public:
  std::vector<Shared(TypeNode)> types;
  AST_ACCEPT_IMPL(UnionTypeNode);
};

class TypeConstraintNode : public TypeNode {
 public:
  Shared(TypeNode) base;
  Shared(TypeNode) constraint;
  AST_ACCEPT_IMPL(TypeConstraintNode);
};

// Generic type application: Name<Arg1, Arg2, ...>
class GenericTypeNode : public TypeNode {
 public:
  tokens::Token name;                      // base type identifier
  std::vector<Shared(TypeNode)> typeArgs;  // type arguments in order
  AST_ACCEPT_IMPL(GenericTypeNode);
};

#undef AST_ACCEPT_IMPL

}  // namespace ast
