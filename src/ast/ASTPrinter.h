#pragma once
#include <iostream>
#include <string>

#include "parser/nodes/ast_visitor.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/meta_nodes.h"

namespace ast {

class ASTPrinter : public ASTVisitor {
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

  void visit(VarDecl& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"VarDecl\",\n";
    printIndent();
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\" ,\n";
    printIndent();
    printKey("qualifier");
    std::cout << "\""
              << (node.qualifier == StorageQualifier::None
                      ? "None"
                      : (node.qualifier == StorageQualifier::Stack
                             ? "Stack"
                             : (node.qualifier == StorageQualifier::Heap
                                    ? "Heap"
                                    : "Static")))
              << "\" ,\n";
    printIndent();

    printKey("isConst");
    std::cout << (node.isConst ? "true" : "false");
    if (node.initializer) {
      std::cout << ",\n";
      printIndent();
      printKey("initializer");
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

  void visit(ImportDecl& node) override {
    printIndent();
    std::cout << "{ ";
    printKey("type");
    std::cout << "\"ImportDecl\", ";
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\"";
    if (!node.fromPath.empty()) {
      std::cout << ", ";
      printKey("from");
      std::cout << "\"" << node.fromPath << "\"";
    }
    std::cout << " }";
  }

  void visit(ProgramNode& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"ProgramNode\",\n";
    printIndent();
    printKey("imports");
    std::cout << " [\n";
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
    std::cout << " [\n";
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
    std::cout << " [\n";
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

  void visit(FunctionDecl& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"FunctionDecl\",\n";
    printIndent();
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\",\n";
    printIndent();
    printKey("modifier");
    switch (node.modifier) {
      case FunctionModifier::None:
        std::cout << "\"None\",\n";
        break;
      case FunctionModifier::Const:
        std::cout << "\"Const\",\n";
        break;
      case FunctionModifier::Constexpr:
        std::cout << "\"Constexpr\",\n";
        break;
      case FunctionModifier::Zerocast:
        std::cout << "\"Zerocast\",\n";
        break;
      case FunctionModifier::Simd:
        std::cout << "\"Simd\",\n";
        break;
      case FunctionModifier::Prefetch:
        std::cout << "\"Prefetch\",\n";
        break;
      case FunctionModifier::Atomic:
        std::cout << "\"Atomic\",\n";
        break;
      case FunctionModifier::Pinned:
        std::cout << "\"Pinned\",\n";
        break;
      default:
        std::cout << "\"None\",\n";
    }
    printIndent();
    printKey("params");
    std::cout << " [\n";
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
      node.returnType->accept(*this);
      --indentLevel;
      std::cout << ",\n";
    } else {
      std::cout << " null,\n";
    }
    printIndent();
    printKey("body");
    if (node.body) {
      std::cout << "\n";
      ++indentLevel;
      node.body->accept(*this);
      --indentLevel;
      std::cout << "\n";
    } else {
      std::cout << " null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }

  void visit(ClassDecl& node) override {
    printIndent();
    std::cout << "{\n";
    ++indentLevel;
    printIndent();
    printKey("type");
    std::cout << "\"ClassDecl\",\n";
    printIndent();
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\",\n";
    printIndent();
    printKey("modifier");
    std::cout << "\"" << static_cast<int>(node.modifier) << "\",\n";
    printIndent();
    printKey("baseClass");
    std::cout << "\"" << node.baseClass.getLexeme() << "\",\n";
    printIndent();
    printKey("body");
    if (node.body) {
      std::cout << "\n";
      ++indentLevel;
      node.body->accept(*this);
      --indentLevel;
      std::cout << "\n";
    } else {
      std::cout << " null\n";
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
    std::cout << "\"InterfaceDecl\",\n";
    printIndent();
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\",\n";
    printIndent();
    printKey("body");
    if (node.body) {
      std::cout << "\n";
      ++indentLevel;
      node.body->accept(*this);
      --indentLevel;
      std::cout << "\n";
    } else {
      std::cout << " null\n";
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
    std::cout << "\"TypeAliasDecl\",\n";
    printIndent();
    printKey("name");
    std::cout << "\"" << node.name.getLexeme() << "\",\n";
    printIndent();
    printKey("aliasedType");
    if (node.aliasedType) {
      std::cout << "\n";
      ++indentLevel;
      node.aliasedType->accept(*this);
      --indentLevel;
      std::cout << "\n";
    } else {
      std::cout << " null\n";
    }
    --indentLevel;
    printIndent();
    std::cout << "}";
  }

  // Add more visit methods as needed...
};

}  // namespace ast
