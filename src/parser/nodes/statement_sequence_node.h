#pragma once

#include <vector>

#include "parser/nodes/ast_visitor.h"  // For ASTVisitor
#include "parser/nodes/misc_nodes.h"   // For Stmt and Shared(Stmt)

namespace ast {

class StatementSequenceNode : public Stmt {
 public:
  std::vector<Shared(Stmt)> statements;
  void accept(ASTVisitor& visitor) override {
    visitor.visit(*this);
  }
};

}  // namespace ast
