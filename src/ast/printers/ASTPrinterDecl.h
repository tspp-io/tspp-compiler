#pragma once
#include <iostream>

#include "ASTPrinterStmt.h"
#include "ASTPrinterType.h"
#include "ASTPrinterUtils.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/meta_nodes.h"

namespace ast {

class ASTPrinterDecl : public ASTVisitor {
  int indentLevel = 0;
  ASTPrinterStmt stmtPrinter;
  ASTPrinterType typePrinter;
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
  // Declaration nodes
  void visit(FunctionDecl& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"FunctionDecl\"," << "\n";
    printIndent();
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\"," << "\n";
    printIndent();
    printKey("modifier");
    std::cout << "\"" << functionModifierToString(node.modifier) << "\","
              << "\n";
    printIndent();
    printKey("methodAttributes");
    std::cout << "[ ";
    for (size_t i = 0; i < node.methodAttributes.size(); ++i) {
      std::cout << '"' << methodAttributeToString(node.methodAttributes[i])
                << '"';
      if (i + 1 < node.methodAttributes.size()) std::cout << ", ";
    }
    std::cout << " ],\n";
    printIndent();
    printKey("params");
    std::cout << "[\n";
    ++indentLevel;
    for (size_t i = 0; i < node.params.size(); ++i) {
      node.params[i]->accept(*this);
      if (i + 1 < node.params.size()) std::cout << ",\n";
    }
    std::cout << "\n";
    --indentLevel;
    printIndent();
    std::cout << "],\n";
    printIndent();
    printKey("returnType");
    if (node.returnType) {
      std::cout << "\n";
      ++indentLevel;
      node.returnType->accept(typePrinter);
      --indentLevel;
      std::cout << ",\n";
    } else {
      std::cout << "null,\n";
    }
    printIndent();
    printKey("body");
    if (node.body) {
      stmtPrinter.visitInline(*node.body);
      std::cout << "\n";
    } else {
      std::cout << "null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }
  void visit(VarDecl& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "VarDecl,\n";
    printIndent();
    printKey("name");
    std::cout << node.name.getLexeme() << ",\n";
    printIndent();
    printKey("qualifier");
    std::cout << storageQualifierToString(node.qualifier) << ",\n";
    printIndent();
    printKey("isConst");
    std::cout << (node.isConst ? "true" : "false");
    std::cout << ",\n";
    printIndent();
    printKey("fieldModifiers");
    std::cout << "[ ";
    for (size_t i = 0; i < node.fieldModifiers.size(); ++i) {
      std::cout << '"' << fieldModifierToString(node.fieldModifiers[i]) << '"';
      if (i + 1 < node.fieldModifiers.size()) std::cout << ", ";
    }
    std::cout << " ],\n";
    printIndent();
    printKey("typeNode");
    if (node.type) {
      std::cout << "\n";
      ++indentLevel;
      node.type->accept(*this);
      --indentLevel;
      std::cout << "\n";
    } else {
      std::cout << "\n";
    }
    printIndent();
    printKey("initializer");
    if (node.initializer) {
      std::cout << "\n";
      ++indentLevel;
      node.initializer->accept(*this);
      --indentLevel;
      std::cout << "\n";
    } else {
      std::cout << "\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }
  void visit(Parameter& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    printType("Parameter");
    std::cout << ", ";
    printKey("name");
    printType(node.name.getLexeme());
    std::cout << ", ";
    printKey("typeNode");
    if (node.type) {
      node.type->accept(*this);
    } else {
      printType("null");
    }
    std::cout << " }";
  }
  void visit(ClassDecl& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"ClassDecl\"," << "\n";
    printIndent();
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\"," << "\n";
    printIndent();
    printKey("modifier");
    std::cout << "\"" << classModifierToString(node.modifier) << "\"," << "\n";
    printIndent();
    printKey("baseClass");
    std::cout << "\"" << node.baseClass.getLexeme() << "\"," << "\n";
    printIndent();
    printKey("body");
    if (node.body) {
      std::cout << "\n";
      ++indentLevel;
      node.body->accept(*this);
      --indentLevel;
      std::cout << "\n";
    } else {
      std::cout << "null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }
  void visit(InterfaceDecl& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"InterfaceDecl\"," << "\n";
    printIndent();
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\"," << "\n";
    printIndent();
    printKey("body");
    if (node.body) {
      std::cout << "\n";
      ++indentLevel;
      node.body->accept(*this);
      --indentLevel;
      std::cout << "\n";
    } else {
      std::cout << "null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }
  void visit(TypeAliasDecl& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"TypeAliasDecl\"," << "\n";
    printIndent();
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\"," << "\n";
    printIndent();
    printKey("aliasedType");
    if (node.aliasedType) {
      std::cout << "\n";
      ++indentLevel;
      node.aliasedType->accept(*this);
      --indentLevel;
      std::cout << "\n";
    } else {
      std::cout << "null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }
  void visit(ImportDecl& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"ImportDecl\", ";
    printKey("name");
    std::cout << '"' << node.name.getLexeme() << '"';
    if (!node.fromPath.empty()) {
      std::cout << ", ";
      printKey("from");
      std::cout << '"' << node.fromPath << '"';
    }
    std::cout << " }";
  }
  void visit(ProgramNode& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"ProgramNode\"," << "\n";
    printIndent();
    printKey("imports");
    std::cout << "[\n";
    ++indentLevel;
    for (size_t i = 0; i < node.imports.size(); ++i) {
      node.imports[i]->accept(*this);
      if (i + 1 < node.imports.size()) std::cout << ",\n";
    }
    std::cout << "\n";
    --indentLevel;
    printIndent();
    std::cout << "],\n";
    printIndent();
    printKey("declarations");
    std::cout << "[\n";
    ++indentLevel;
    for (size_t i = 0; i < node.declarations.size(); ++i) {
      node.declarations[i]->accept(*this);
      if (i + 1 < node.declarations.size()) std::cout << ",\n";
    }
    std::cout << "\n";
    --indentLevel;
    printIndent();
    std::cout << "],\n";
    printIndent();
    printKey("statements");
    std::cout << "[\n";
    ++indentLevel;
    for (size_t i = 0; i < node.statements.size(); ++i) {
      node.statements[i]->accept(*this);
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
  void visit(ModifierNode& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"ModifierNode\", ";
    printKey("modifierToken");
    std::cout << '"' << node.modifierToken.getLexeme() << "\" }";
  }
  void visit(StorageQualifierNode& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"StorageQualifierNode\", ";
    printKey("qualifierToken");
    std::cout << '"' << node.qualifierToken.getLexeme() << "\" }";
  }
  void visit(ParameterListNode& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"ParameterListNode\", ";
    printKey("parameters");
    std::cout << " [ ";
    for (size_t i = 0; i < node.parameters.size(); ++i) {
      node.parameters[i]->accept(*this);
      if (i + 1 < node.parameters.size()) std::cout << ", ";
    }
    std::cout << " ] }";
  }
  // Delegate statement nodes to stmtPrinter
  void visit(BlockStmt& node) override {
    stmtPrinter.visit(node);
  }
  void visit(IfStmt& node) override {
    stmtPrinter.visit(node);
  }
  void visit(WhileStmt& node) override {
    stmtPrinter.visit(node);
  }
  void visit(ForStmt& node) override {
    stmtPrinter.visit(node);
  }
  void visit(ReturnStmt& node) override {
    stmtPrinter.visit(node);
  }
  void visit(ExprStmt& node) override {
    stmtPrinter.visit(node);
  }
};

}  // namespace ast
