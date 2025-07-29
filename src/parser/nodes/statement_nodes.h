#pragma once
#include <vector>

#include "base_node.h"
#include "expression_nodes.h"

namespace ast {

#define AST_ACCEPT_IMPL(NodeType)             \
  void accept(ASTVisitor& visitor) override { \
    visitor.visit(*this);                     \
  }

class BlockStmt : public Stmt {
 public:
  std::vector<Shared(Stmt)> statements;
  AST_ACCEPT_IMPL(BlockStmt);
};

class ExprStmt : public Stmt {
 public:
  Shared(Expr) expression;
  AST_ACCEPT_IMPL(ExprStmt);
};

class IfStmt : public Stmt {
 public:
  Shared(Expr) condition;
  Shared(Stmt) thenBranch;
  Shared(Stmt) elseBranch;
  AST_ACCEPT_IMPL(IfStmt);
};

class WhileStmt : public Stmt {
 public:
  Shared(Expr) condition;
  Shared(Stmt) body;
  AST_ACCEPT_IMPL(WhileStmt);
};

class ForStmt : public Stmt {
 public:
  Shared(Expr) init;
  Shared(Expr) condition;
  Shared(Expr) increment;
  Shared(Stmt) body;
  AST_ACCEPT_IMPL(ForStmt);
};

class ReturnStmt : public Stmt {
 public:
  Shared(Expr) value;
  AST_ACCEPT_IMPL(ReturnStmt);
};

#undef AST_ACCEPT_IMPL

}  // namespace ast
