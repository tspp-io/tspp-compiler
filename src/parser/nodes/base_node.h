#pragma once
#include "core/common/common_types.h"
#include "core/common/macros.h"
#include "parser/nodes/ast_visitor.h"

namespace ast {

class BaseNode {
 public:
  virtual void accept(ASTVisitor& visitor) = 0;
  virtual ~BaseNode() = default;

  core::Location location;
};

#define AST_ACCEPT_IMPL(NodeType)             \
  void accept(ASTVisitor& visitor) override { \
    visitor.visit(*this);                     \
  }

class Expr : public BaseNode {
 public:
  AST_ACCEPT_IMPL(Expr);
};
class Stmt : public BaseNode {
 public:
  AST_ACCEPT_IMPL(Stmt);
};
class Decl : public BaseNode {
 public:
  AST_ACCEPT_IMPL(Decl);
};
class TypeNode : public BaseNode {
 public:
  AST_ACCEPT_IMPL(TypeNode);
};

#undef AST_ACCEPT_IMPL

}  // namespace ast
