#pragma once
#include <memory>

namespace ast {

// Forward declarations for all node types
class BaseNode;
class Expr;
class Stmt;
class Decl;
class TypeNode;
class BinaryExpr;
class UnaryExpr;
class LiteralExpr;
class IdentifierExpr;
class AssignmentExpr;
class CallExpr;
class MemberAccessExpr;
class BlockStmt;
class StaticBlockStmt;
class ExprStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class ReturnStmt;
class VarDecl;
class Parameter;
class FunctionDecl;
class ConstructorDecl;
class ClassDecl;
class InterfaceDecl;
class TypeAliasDecl;
class ImportDecl;
class BasicTypeNode;
class PointerTypeNode;
class SmartPointerTypeNode;
class UnionTypeNode;
class TypeConstraintNode;
class GenericTypeNode;
class TypeParam;
class ProgramNode;
class ModifierNode;
class StorageQualifierNode;
class ParameterListNode;
class GroupingExpr;
class ThisExpr;
class NullExpr;
class NewExpr;
class StatementSequenceNode;

class ASTVisitor {
 public:
  virtual void visit(BaseNode&) {}
  virtual void visit(Expr&) {}
  virtual void visit(Stmt&) {}
  virtual void visit(Decl&) {}
  virtual void visit(TypeNode&) {}
  virtual void visit(BinaryExpr&) {}
  virtual void visit(UnaryExpr&) {}
  virtual void visit(LiteralExpr&) {}
  virtual void visit(IdentifierExpr&) {}
  virtual void visit(AssignmentExpr&) {}
  virtual void visit(CallExpr&) {}
  virtual void visit(MemberAccessExpr&) {}
  virtual void visit(BlockStmt&) {}
  virtual void visit(StaticBlockStmt&) {}
  virtual void visit(ExprStmt&) {}
  virtual void visit(IfStmt&) {}
  virtual void visit(WhileStmt&) {}
  virtual void visit(ForStmt&) {}
  virtual void visit(ReturnStmt&) {}
  virtual void visit(VarDecl&) {}
  virtual void visit(Parameter&) {}
  virtual void visit(FunctionDecl&) {}
  virtual void visit(ConstructorDecl&) {}
  virtual void visit(ClassDecl&) {}
  virtual void visit(InterfaceDecl&) {}
  virtual void visit(TypeAliasDecl&) {}
  virtual void visit(ImportDecl&) {}
  virtual void visit(BasicTypeNode&) {}
  virtual void visit(PointerTypeNode&) {}
  virtual void visit(SmartPointerTypeNode&) {}
  virtual void visit(UnionTypeNode&) {}
  virtual void visit(TypeConstraintNode&) {}
  virtual void visit(GenericTypeNode&) {}
  virtual void visit(TypeParam&) {}
  virtual void visit(ProgramNode&) {}
  virtual void visit(ModifierNode&) {}
  virtual void visit(StorageQualifierNode&) {}
  virtual void visit(ParameterListNode&) {}
  virtual void visit(GroupingExpr&) {}
  virtual void visit(ThisExpr&) {}
  virtual void visit(NullExpr&) {}
  virtual void visit(NewExpr&) {}
  virtual void visit(StatementSequenceNode&) {}
  virtual ~ASTVisitor() = default;
};

}  // namespace ast
