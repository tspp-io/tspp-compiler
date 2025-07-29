#pragma once
#include "base_node.h"
#include "core/common/macros.h"
#include "tokens/Token.h"

namespace ast {

class GroupingExpr : public Expr {
 public:
  Shared(Expr) inner;
  void accept(ASTVisitor& visitor) override {
    visitor.visit(*this);
  }
};

class ThisExpr : public Expr {
 public:
  tokens::Token keyword;  // 'this'
  void accept(ASTVisitor& visitor) override {
    visitor.visit(*this);
  }
};

class NullExpr : public Expr {
 public:
  tokens::Token nullToken;  // 'null'
  void accept(ASTVisitor& visitor) override {
    visitor.visit(*this);
  }
};

}  // namespace ast
