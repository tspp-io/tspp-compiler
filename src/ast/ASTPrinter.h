#pragma once
#include "./printers/ASTPrinterDecl.h"
#include "./printers/ASTPrinterExpr.h"
#include "./printers/ASTPrinterStmt.h"
#include "./printers/ASTPrinterUtils.h"

namespace ast {

class ASTPrinter : public ASTVisitor {
  ASTPrinterExpr exprPrinter;
  ASTPrinterStmt stmtPrinter;
  ASTPrinterDecl declPrinter;

 public:
  // Expression nodes
  void visit(BinaryExpr& node) override {
    exprPrinter.visit(node);
  }
  void visit(LiteralExpr& node) override {
    exprPrinter.visit(node);
  }
  void visit(IdentifierExpr& node) override {
    exprPrinter.visit(node);
  }
  void visit(CallExpr& node) override {
    exprPrinter.visit(node);
  }
  // Add more as needed...

  // Statement nodes
  void visit(BlockStmt& node) override {
    stmtPrinter.visit(node);
  }
  // Add more as needed (IfStmt, WhileStmt, etc.)

  // Declaration nodes
  void visit(FunctionDecl& node) override {
    declPrinter.visit(node);
  }
  void visit(ProgramNode& node) override {
    declPrinter.visit(node);
  }
  // Add more as needed (VarDecl, ClassDecl, etc.)
};

}  // namespace ast
