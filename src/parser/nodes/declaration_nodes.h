#pragma once
#include <vector>

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
// Access modifiers for class members (optional, may be ignored by backends)
enum class AccessModifier { Default, Public, Private, Protected };

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
  // Optional access modifier when declared inside a class
  AccessModifier access = AccessModifier::Default;
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
  // Optional access modifier when declared inside a class
  AccessModifier access = AccessModifier::Default;
  AST_ACCEPT_IMPL(FunctionDecl);
};

// Constructor declaration inside a class
class ConstructorDecl : public Decl {
 public:
  // Optional access modifier; default implies language default (usually public)
  AccessModifier access = AccessModifier::Default;
  tokens::Token keyword;  // 'constructor'
  std::vector<Shared(Parameter)> params;
  Shared(BlockStmt) body;
  AST_ACCEPT_IMPL(ConstructorDecl);
};

class ClassDecl : public Decl {
 public:
  ClassModifier modifier;
  tokens::Token name;
  tokens::Token baseClass;  // optional
  // Optional implemented interfaces (names only for now)
  std::vector<tokens::Token> interfaces;
  // Unified body retained for backward compatibility (not used by new flow)
  Shared(BlockStmt) body;
  // Parsed members (fields/methods/ctor)
  std::vector<Shared(VarDecl)> fields;
  std::vector<Shared(FunctionDecl)> methods;
  Shared(ConstructorDecl) constructor;  // optional
  AST_ACCEPT_IMPL(ClassDecl);
};

class InterfaceDecl : public Decl {
 public:
  tokens::Token name;
  // Optional base interfaces this interface extends
  std::vector<tokens::Token> bases;
  // Parsed members: property signatures and method signatures (no bodies)
  std::vector<Shared(VarDecl)> properties;
  std::vector<Shared(FunctionDecl)> methods;
  // Retain body for backward compatibility; not used for codegen
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
