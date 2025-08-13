#pragma once
#include "base_node.h"
#include "core/common/macros.h"
#include "tokens/tokens.h"

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

// "new" operator for class/struct instantiation
class NewExpr : public Expr {
 public:
  tokens::Token keyword;                // 'new'
  tokens::Token className;              // Identifier of the class
  std::vector<Shared(Expr)> arguments;  // constructor arguments
  void accept(ASTVisitor& visitor) override {
    visitor.visit(*this);
  }
};

}  // namespace ast
