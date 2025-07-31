#pragma once
#include <iostream>

#include "ASTPrinterUtils.h"
#include "parser/nodes/type_nodes.h"

namespace ast {

class ASTPrinterType : public ASTVisitor {
  int indentLevel = 0;
  void printIndent() const {
    for (int i = 0; i < indentLevel; ++i) std::cout << "  ";
  }
  void printKey(const std::string& key) const {
    std::cout << "\"" << key << "\": ";
  }
  void printString(const std::string& value) const {
    std::cout << '\"' << value << '\"';
  }
  void printNull() const {
    std::cout << "null";
  }

 public:
  void visit(BasicTypeNode& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    printString("BasicTypeNode");
    std::cout << ", ";
    printKey("name");
    printString(node.name.getLexeme());
    std::cout << " }";
  }
  void visit(PointerTypeNode& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    printString("PointerTypeNode");
    std::cout << ", ";
    printKey("qualifier");
    printString(node.qualifier.getLexeme());
    std::cout << ", ";
    printKey("baseType");
    if (node.baseType) {
      node.baseType->accept(*this);
    } else {
      printNull();
    }
    std::cout << " }";
  }
  void visit(SmartPointerTypeNode& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    printString("SmartPointerTypeNode");
    std::cout << ", ";
    printKey("kind");
    printString(node.kind.getLexeme());
    std::cout << ", ";
    printKey("target");
    if (node.target) {
      node.target->accept(*this);
    } else {
      printNull();
    }
    std::cout << " }";
  }
  void visit(UnionTypeNode& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    printString("UnionTypeNode");
    std::cout << ", ";
    printKey("types");
    std::cout << "[ ";
    for (size_t i = 0; i < node.types.size(); ++i) {
      node.types[i]->accept(*this);
      if (i + 1 < node.types.size()) std::cout << ", ";
    }
    std::cout << " ] }";
  }
  void visit(TypeConstraintNode& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    printString("TypeConstraintNode");
    std::cout << ", ";
    printKey("base");
    if (node.base) {
      node.base->accept(*this);
    } else {
      printNull();
    }
    std::cout << ", ";
    printKey("constraint");
    if (node.constraint) {
      node.constraint->accept(*this);
    } else {
      printNull();
    }
    std::cout << " }";
  }
};

}  // namespace ast
