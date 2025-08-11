#pragma once
#include "base_node.h"
#include "core/common/macros.h"
#include "statement_nodes.h"
#include "tokens/tokens.h"
#include "type_nodes.h"

namespace ast {

enum class StorageQualifier { None, Stack, Heap, Static };
enum class FunctionModifier {
  None,
  Const,
  Constexpr,
  Zerocast,
  Simd,
  Prefetch,
  Atomic,
  Pinned
};
enum class ClassModifier { None, Packed, Abstract, Pinned };

#define AST_ACCEPT_IMPL(NodeType)             \
  void accept(ASTVisitor& visitor) override { \
    visitor.visit(*this);                     \
  }

class VarDecl : public Stmt {
 public:
  StorageQualifier qualifier;
  Shared(TypeNode) type;
  tokens::Token name;
  bool isConst = false;  // true if 'const' keyword is used
  Shared(Expr) initializer;
  AST_ACCEPT_IMPL(VarDecl);
};

class Parameter : public BaseNode {
 public:
  Shared(TypeNode) type;
  tokens::Token name;
  AST_ACCEPT_IMPL(Parameter);
};

class FunctionDecl : public Decl {
 public:
  FunctionModifier modifier;
  tokens::Token name;
  std::vector<Shared(Parameter)> params;
  Shared(TypeNode) returnType;
  Shared(BlockStmt) body;
  AST_ACCEPT_IMPL(FunctionDecl);
};

class ClassDecl : public Decl {
 public:
  ClassModifier modifier;
  tokens::Token name;
  tokens::Token baseClass;  // optional
  Shared(BlockStmt) body;
  AST_ACCEPT_IMPL(ClassDecl);
};

class InterfaceDecl : public Decl {
 public:
  tokens::Token name;
  Shared(BlockStmt) body;
  AST_ACCEPT_IMPL(InterfaceDecl);
};

class TypeAliasDecl : public Decl {
 public:
  tokens::Token name;
  Shared(TypeNode) aliasedType;
  AST_ACCEPT_IMPL(TypeAliasDecl);
};

#undef AST_ACCEPT_IMPL

}  // namespace ast
