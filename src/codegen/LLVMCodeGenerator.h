#pragma once
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "parser/nodes/ast_visitor.h"
#include "parser/visitors/semantic/SemanticAnalyzerVisitor.h"

namespace codegen {

class LLVMCodeGenerator : public ast::ASTVisitor {
 public:
  LLVMCodeGenerator();
  LLVMCodeGenerator(ast::SemanticAnalyzerVisitor* semanticAnalyzer);
  void generate(ast::BaseNode* root);
  void generate(ast::BaseNode* root, const std::string& outFile);

  // AST node visit overrides
  void visit(ast::ProgramNode&) override;
  void visit(ast::VarDecl&) override;
  void visit(ast::LiteralExpr&) override;
  void visit(ast::IdentifierExpr&) override;
  void visit(ast::BinaryExpr&) override;
  void visit(ast::FunctionDecl&) override;
  void visit(ast::ReturnStmt&) override;
  void visit(ast::ExprStmt&) override;
  void visit(ast::BlockStmt&) override;
  void visit(ast::CallExpr&) override;
  void visit(ast::IfStmt&) override;
  void visit(ast::ClassDecl&) override;
  void visit(ast::InterfaceDecl&) override;
  void visit(ast::TypeAliasDecl&) override;
  // ...add more as needed...

 private:
  llvm::LLVMContext context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  struct SymbolInfo {
    llvm::Value* value;
    llvm::Type* type;
    bool isMutable;
  };
  std::unordered_map<std::string, SymbolInfo> symbolTable;
  llvm::Value* lastValue = nullptr;
  llvm::Function* currentFunction = nullptr;

  // Semantic analyzer for type resolution
  ast::SemanticAnalyzerVisitor* semanticAnalyzer = nullptr;

  // Helper method to convert semantic type to LLVM type
  llvm::Type* getLLVMType(const std::string& typeName);
  // ...other helpers for function/variable management...
};

}  // namespace codegen
