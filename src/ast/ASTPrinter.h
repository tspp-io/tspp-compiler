#pragma once
#include <iostream>

#include "parser/nodes/ast_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"

namespace ast {

class ASTPrinter : public ASTVisitor {
 public:
  void visit(BinaryExpr& node) override {
    std::cout << "(";
    node.left->accept(*this);
    std::cout << " " << node.op.getLexeme() << " ";
    node.right->accept(*this);
    std::cout << ")";
  }

  void visit(LiteralExpr& node) override {
    std::cout << node.value.getLexeme();
  }

  void visit(IdentifierExpr& node) override {
    std::cout << node.name.getLexeme();
  }

  void visit(CallExpr& node) override {
    node.callee->accept(*this);
    std::cout << "(";
    for (size_t i = 0; i < node.arguments.size(); ++i) {
      node.arguments[i]->accept(*this);
      if (i + 1 < node.arguments.size()) std::cout << ", ";
    }
    std::cout << ")";
  }

  void visit(VarDecl& node) override {
    std::cout << "var " << node.name.getLexeme();
    if (node.initializer) {
      std::cout << " = ";
      node.initializer->accept(*this);
    }
  }

  // Add more visit methods as needed...
};

}  // namespace ast
