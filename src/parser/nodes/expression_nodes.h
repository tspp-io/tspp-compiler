#pragma once
#include <vector>

#include "base_node.h"
#include "core/common/macros.h"
#include "tokens/token_type.h"
#include "tokens/tokens.h"

namespace ast {

#define AST_ACCEPT_IMPL(NodeType)             \
  void accept(ASTVisitor& visitor) override { \
    visitor.visit(*this);                     \
  }

class BinaryExpr : public Expr {
 public:
  Shared(Expr) left;
  tokens::Token op;
  Shared(Expr) right;
  AST_ACCEPT_IMPL(BinaryExpr);
};

class UnaryExpr : public Expr {
 public:
  tokens::Token op;
  Shared(Expr) operand;
  AST_ACCEPT_IMPL(UnaryExpr);
};

class LiteralExpr : public Expr {
 public:
  tokens::Token value;
  AST_ACCEPT_IMPL(LiteralExpr);
};

class IdentifierExpr : public Expr {
 public:
  tokens::Token name;
  AST_ACCEPT_IMPL(IdentifierExpr);
};

class AssignmentExpr : public Expr {
 public:
  Shared(Expr) target;  // Left-hand side (identifier, member access, etc.)
  Shared(Expr) value;   // Right-hand side value to assign
  AST_ACCEPT_IMPL(AssignmentExpr);
};

class CallExpr : public Expr {
 public:
  Shared(Expr) callee;
  std::vector<Shared(Expr)> arguments;
  AST_ACCEPT_IMPL(CallExpr);
};

class MemberAccessExpr : public Expr {
 public:
  Shared(Expr) object;
  tokens::Token member;
  AST_ACCEPT_IMPL(MemberAccessExpr);
};

// Object literal expression: { key: value, ... }
class ObjectLiteralExpr : public Expr {
 public:
  struct Field {
    tokens::Token key;   // identifier, string, or number token
    Shared(Expr) value;  // expression for the value
  };
  std::vector<Field> fields;
  AST_ACCEPT_IMPL(ObjectLiteralExpr);
};

#undef AST_ACCEPT_IMPL

}  // namespace ast
