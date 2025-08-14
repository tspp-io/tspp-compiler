#pragma once
#include <iostream>

#include "ASTPrinterExpr.h"
#include "ASTPrinterUtils.h"
#include "parser/nodes/statement_nodes.h"

namespace ast {

class ASTPrinterStmt : public ASTVisitor {
  int indentLevel = 0;
  bool inlineBlock = false;
  ASTPrinterExpr exprPrinter;  // For delegating expression printing
  void printIndent() const {
    for (int i = 0; i < indentLevel; ++i) std::cout << "  ";
  }
  void printKey(const std::string& key) const {
    std::cout << colorKey(key) << ": ";
  }
  void printType(const std::string& type) const {
    std::cout << colorType(type);
  }

 public:
  // Inline mode for block statements (for function/class bodies)
  void visit(BlockStmt& node) override {
    if (inlineBlock) {
      std::cout << "{";
      ++indentLevel;
      std::cout << "\n";
      printIndent();
      printKey("type");
      printType("BlockStmt");
      std::cout << ",\n";
      printIndent();
      std::cout << "statement: ";
      std::cout << "[\n";
      ++indentLevel;
      for (size_t i = 0; i < node.statements.size(); ++i) {
        if (node.statements[i]) {
          node.statements[i]->accept(*this);
        } else {
          printIndent();
          std::cout << "null";
        }
        if (i + 1 < node.statements.size()) std::cout << ",\n";
      }
      std::cout << "\n";
      --indentLevel;
      printIndent();
      std::cout << "]\n";
      --indentLevel;
      printIndent();
      std::cout << "}";
    } else {
      printIndent();
      std::cout << "{\n";
      ++indentLevel;
      printIndent();
      printKey("type");
      printType("BlockStmt");
      std::cout << ",\n";
      printIndent();
      printKey("statements");
      std::cout << "[\n";
      ++indentLevel;
      for (size_t i = 0; i < node.statements.size(); ++i) {
        if (node.statements[i]) {
          node.statements[i]->accept(*this);
        } else {
          printIndent();
          std::cout << "null";
        }
        if (i + 1 < node.statements.size()) std::cout << ",\n";
      }
      std::cout << "\n";
      --indentLevel;
      printIndent();
      std::cout << "]\n";
      --indentLevel;
      printIndent();
      std::cout << "}";
    }
  }

  void visit(StaticBlockStmt& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    printType("StaticBlockStmt");
    std::cout << ",\n";
    printIndent();
    printKey("body");
    if (node.body) {
      node.body->accept(*this);
      std::cout << "\n";
    } else {
      std::cout << "null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }

  // Helper for decl printer to print block inline
  void visitInline(BlockStmt& node) {
    bool prev = inlineBlock;
    inlineBlock = true;
    visit(node);
    inlineBlock = prev;
  }

  void visit(ExprStmt& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    printType("ExprStmt");
    std::cout << ", ";
    printKey("expression");
    if (node.expression) {
      node.expression->accept(exprPrinter);
    } else {
      std::cout << "null";
    }
    std::cout << " }";
  }

  void visit(IfStmt& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    printType("IfStmt");
    std::cout << ",\n";
    printIndent();
    printKey("condition");
    if (node.condition) {
      node.condition->accept(exprPrinter);
      std::cout << ",\n";
    } else {
      std::cout << "null,\n";
    }
    printIndent();
    printKey("thenBranch");
    if (node.thenBranch) {
      node.thenBranch->accept(*this);
      std::cout << ",\n";
    } else {
      std::cout << "null,\n";
    }
    printIndent();
    printKey("elseBranch");
    if (node.elseBranch) {
      node.elseBranch->accept(*this);
      std::cout << "\n";
    } else {
      std::cout << "null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }

  void visit(WhileStmt& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    printType("WhileStmt");
    std::cout << ",\n";
    printIndent();
    printKey("condition");
    if (node.condition) {
      node.condition->accept(*this);
      std::cout << ",\n";
    } else {
      std::cout << "null,\n";
    }
    printIndent();
    printKey("body");
    if (node.body) {
      node.body->accept(*this);
      std::cout << "\n";
    } else {
      std::cout << "null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }

  void visit(ForStmt& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    printType("ForStmt");
    std::cout << ",\n";
    printIndent();
    printKey("init");
    if (node.init) {
      node.init->accept(*this);
      std::cout << ",\n";
    } else {
      std::cout << "null,\n";
    }
    printIndent();
    printKey("condition");
    if (node.condition) {
      node.condition->accept(*this);
      std::cout << ",\n";
    } else {
      std::cout << "null,\n";
    }
    printIndent();
    printKey("increment");
    if (node.increment) {
      node.increment->accept(*this);
      std::cout << ",\n";
    } else {
      std::cout << "null,\n";
    }
    printIndent();
    printKey("body");
    if (node.body) {
      node.body->accept(*this);
      std::cout << "\n";
    } else {
      std::cout << "null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }

  void visit(ReturnStmt& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    printType("ReturnStmt");
    std::cout << ",\n";
    printIndent();
    printKey("value");
    if (node.value) {
      node.value->accept(exprPrinter);
      std::cout << "\n";
    } else {
      std::cout << "null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }
};

}  // namespace ast
