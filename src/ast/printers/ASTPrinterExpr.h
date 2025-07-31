#pragma once
#include <iostream>

#include "ASTPrinterUtils.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/misc_nodes.h"

namespace ast {

class ASTPrinterExpr : public ASTVisitor {
  int indentLevel = 0;
  void printIndent() const {
    for (int i = 0; i < indentLevel; ++i) std::cout << "  ";
  }
  void printKey(const std::string& key) const {
    std::cout << "\"" << key << "\": ";
  }

 public:
  void visit(BinaryExpr& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"BinaryExpr\",\n";
    printIndent();
    printKey("operator");
    std::cout << "\"" << node.op.getLexeme() << "\",\n";
    printIndent();
    printKey("left");
    std::cout << "\n";
    ++indentLevel;
    node.left->accept(*this);
    std::cout << ",\n";
    --indentLevel;
    printIndent();
    printKey("right");
    std::cout << "\n";
    ++indentLevel;
    node.right->accept(*this);
    std::cout << "\n";
    --indentLevel;
    --indentLevel;
    printIndent();
    std::cout << "}";
  }
  void visit(LiteralExpr& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"LiteralExpr\", ";
    printKey("value");
    std::cout << "\"" << node.value.getLexeme() << "\" }";
  }
  void visit(IdentifierExpr& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"IdentifierExpr\", ";
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\" }";
  }
  void visit(CallExpr& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"CallExpr\",\n";
    printIndent();
    printKey("callee");
    std::cout << "\n";
    ++indentLevel;
    node.callee->accept(*this);
    std::cout << ",\n";
    --indentLevel;
    printIndent();
    printKey("arguments");
    std::cout << " [\n";
    ++indentLevel;
    for (size_t i = 0; i < node.arguments.size(); ++i) {
      node.arguments[i]->accept(*this);
      if (i + 1 < node.arguments.size()) std::cout << ",\n";
    }
    std::cout << "\n";
    --indentLevel;
    printIndent();
    std::cout << "]\n";
    --indentLevel;
    printIndent();
    std::cout << "}";
  }
  void visit(UnaryExpr& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"UnaryExpr\", ";
    printKey("operator");
    std::cout << '"' << node.op.getLexeme() << "\", ";
    printKey("operand");
    if (node.operand) {
      node.operand->accept(*this);
    } else
      std::cout << "null";
    std::cout << " }";
  }
  void visit(AssignmentExpr& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"AssignmentExpr\", ";
    printKey("name");
    std::cout << '"' << node.name.getLexeme() << "\", ";
    printKey("value");
    if (node.value) {
      node.value->accept(*this);
    } else
      std::cout << "null";
    std::cout << " }";
  }
  void visit(MemberAccessExpr& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"MemberAccessExpr\", ";
    printKey("object");
    if (node.object) {
      node.object->accept(*this);
    } else
      std::cout << "null";
    std::cout << ", ";
    printKey("member");
    std::cout << '"' << node.member.getLexeme() << "\" }";
  }
  void visit(GroupingExpr& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"GroupingExpr\", ";
    printKey("inner");
    if (node.inner) {
      node.inner->accept(*this);
    } else
      std::cout << "null";
    std::cout << " }";
  }
  void visit(ThisExpr& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"ThisExpr\", ";
    printKey("keyword");
    std::cout << '"' << node.keyword.getLexeme() << "\" }";
  }
  void visit(NullExpr& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"NullExpr\", ";
    printKey("nullToken");
    std::cout << '"' << node.nullToken.getLexeme() << "\" }";
  }
  // Add more visit methods for other expression types if needed...
};

}  // namespace ast
