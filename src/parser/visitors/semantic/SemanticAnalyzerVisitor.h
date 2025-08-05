#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "parser/nodes/ast_visitor.h"
#include "parser/visitors/semantic/SemanticScope.h"

namespace ast {

class SemanticAnalyzerVisitor : public ASTVisitor {
 public:
  SemanticAnalyzerVisitor();

  // === All AST node visit methods ===
  void visit(BaseNode&) override;
  void visit(Expr&) override;
  void visit(Stmt&) override;
  void visit(Decl&) override;
  void visit(TypeNode&) override;
  void visit(BinaryExpr&) override;
  void visit(UnaryExpr&) override;
  void visit(LiteralExpr&) override;
  void visit(IdentifierExpr&) override;
  void visit(AssignmentExpr&) override;
  void visit(CallExpr&) override;
  void visit(MemberAccessExpr&) override;
  void visit(BlockStmt&) override;
  void visit(ExprStmt&) override;
  void visit(IfStmt&) override;
  void visit(WhileStmt&) override;
  void visit(ForStmt&) override;
  void visit(ReturnStmt&) override;
  void visit(VarDecl&) override;
  void visit(Parameter&) override;
  void visit(FunctionDecl&) override;
  void visit(ClassDecl&) override;
  void visit(InterfaceDecl&) override;
  void visit(TypeAliasDecl&) override;
  void visit(ImportDecl&) override;
  void visit(BasicTypeNode&) override;
  void visit(PointerTypeNode&) override;
  void visit(SmartPointerTypeNode&) override;
  void visit(UnionTypeNode&) override;
  void visit(TypeConstraintNode&) override;
  void visit(ProgramNode&) override;
  void visit(ModifierNode&) override;
  void visit(StorageQualifierNode&) override;
  void visit(ParameterListNode&) override;
  void visit(GroupingExpr&) override;
  void visit(ThisExpr&) override;
  void visit(NullExpr&) override;

  // Get all semantic errors found
  const std::vector<std::string>& getErrors() const;

  // Public method to resolve types (for use by codegen)
  std::string resolveType(TypeNode* type);

 private:
  std::unordered_map<std::string, std::shared_ptr<TypeNode>> typeAliases;
  std::shared_ptr<SemanticScope> currentScope;
  std::vector<std::string> errors;

  void enterScope();
  void exitScope();
  void reportError(const std::string& message);
};

}  // namespace ast
