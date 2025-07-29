#pragma once
#include <vector>

#include "base_node.h"
#include "declaration_nodes.h"
#include "expression_nodes.h"
#include "statement_nodes.h"
#include "tokens/tokens.h"
#include "type_nodes.h"

namespace ast {

#define AST_ACCEPT_IMPL(NodeType)             \
  void accept(ASTVisitor& visitor) override { \
    visitor.visit(*this);                     \
  }

class ProgramNode : public BaseNode {
 public:
  std::vector<Shared(ImportDecl)> imports;
  std::vector<Shared(Decl)> declarations;
  std::vector<Shared(Stmt)> statements;
  AST_ACCEPT_IMPL(ProgramNode);
};

class ModifierNode : public BaseNode {
 public:
  tokens::Token modifierToken;  // e.g. #const, #atomic
  AST_ACCEPT_IMPL(ModifierNode);
};

class StorageQualifierNode : public BaseNode {
 public:
  tokens::Token qualifierToken;  // e.g. #stack, #heap
  AST_ACCEPT_IMPL(StorageQualifierNode);
};

class ParameterListNode : public BaseNode {
 public:
  std::vector<Shared(Parameter)> parameters;
  AST_ACCEPT_IMPL(ParameterListNode);
};

#undef AST_ACCEPT_IMPL

}  // namespace ast
