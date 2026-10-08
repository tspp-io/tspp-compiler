#pragma once
#include <vector>

#include "base_node.h"
#include "expression_nodes.h"

namespace ast {

#define AST_ACCEPT_IMPL(NodeType)             \
  void accept(ASTVisitor& visitor) override { \
    visitor.visit(*this);                     \
  }

class BlockStmt : public Stmt {
 public:
  std::vector<Shared(Stmt)> statements;
  AST_ACCEPT_IMPL(BlockStmt);
};

// Represents a 'static { ... }' block inside a class body
class StaticBlockStmt : public Stmt {
 public:
  Shared(BlockStmt) body;  // actual block of statements executed statically
  AST_ACCEPT_IMPL(StaticBlockStmt);
};

class ExprStmt : public Stmt {
 public:
  Shared(Expr) expression;
  AST_ACCEPT_IMPL(ExprStmt);
};

class IfStmt : public Stmt {
 public:
  Shared(Expr) condition;
  Shared(Stmt) thenBranch;
  Shared(Stmt) elseBranch;
  AST_ACCEPT_IMPL(IfStmt);
};

class WhileStmt : public Stmt {
 public:
  Shared(Expr) condition;
  Shared(Stmt) body;
  AST_ACCEPT_IMPL(WhileStmt);
};

class ForStmt : public Stmt {
 public:
  // Initializer can be either a declaration (VarDecl) or an expression
  // wrapped in an ExprStmt. Use Stmt to cover both cases.
  Shared(Stmt) init;
  Shared(Expr) condition;
  Shared(Expr) increment;
  Shared(Stmt) body;
  AST_ACCEPT_IMPL(ForStmt);
};

class AsmStmt : public Stmt {
 public:
  std::string assembly;
  struct Constraint {
    std::string constraint;
    Shared(Expr) expression;  // For inputs/outputs
  };
  std::vector<Constraint> outputs;
  std::vector<Constraint> inputs;
  std::vector<std::string> clobbers;
  bool isVolatile = false;

  AST_ACCEPT_IMPL(AsmStmt);
};

class ReturnStmt : public Stmt {
 public:
  Shared(Expr) value;
  AST_ACCEPT_IMPL(ReturnStmt);
};

// do stmt while (condition);
class DoWhileStmt : public Stmt {
 public:
  Shared(Stmt) body;
  Shared(Expr) condition;
  AST_ACCEPT_IMPL(DoWhileStmt);
};

// for (let|const name[: Type] of iterable) stmt
class ForOfStmt : public Stmt {
 public:
  // Loop variable declaration (a VarDecl without initializer)
  Shared(Stmt) variable;
  Shared(Expr) iterable;
  Shared(Stmt) body;
  AST_ACCEPT_IMPL(ForOfStmt);
};

class BreakStmt : public Stmt {
 public:
  AST_ACCEPT_IMPL(BreakStmt);
};

class ContinueStmt : public Stmt {
 public:
  AST_ACCEPT_IMPL(ContinueStmt);
};

// switch (discriminant) { case expr: stmt* ... default: stmt* }
class SwitchStmt : public Stmt {
 public:
  struct Case {
    Shared(Expr) test;  // nullptr for 'default'
    std::vector<Shared(Stmt)> body;
    core::Location location;
  };
  Shared(Expr) discriminant;
  std::vector<Case> cases;
  AST_ACCEPT_IMPL(SwitchStmt);
};

class ThrowStmt : public Stmt {
 public:
  Shared(Expr) value;
  AST_ACCEPT_IMPL(ThrowStmt);
};

// try { ... } [catch [(name[: Type])] { ... }] [finally { ... }]
class TryStmt : public Stmt {
 public:
  Shared(BlockStmt) tryBlock;
  bool hasCatchParam = false;
  tokens::Token catchParam;
  Shared(TypeNode) catchType;      // optional
  Shared(BlockStmt) catchBlock;    // nullptr when no catch clause
  Shared(BlockStmt) finallyBlock;  // nullptr when no finally clause
  AST_ACCEPT_IMPL(TryStmt);
};

#undef AST_ACCEPT_IMPL

}  // namespace ast
