#pragma once
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ConstantExpressionEvaluator.h"
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
  void visit(ast::UnaryExpr&) override;
  void visit(ast::AssignmentExpr&) override;
  void visit(ast::FunctionDecl&) override;
  void visit(ast::ConstructorDecl&) override;
  void visit(ast::ReturnStmt&) override;
  void visit(ast::ExprStmt&) override;
  void visit(ast::BlockStmt&) override;
  void visit(ast::CallExpr&) override;
  void visit(ast::MemberAccessExpr&) override;
  void visit(ast::ThisExpr&) override;
  void visit(ast::NewExpr&) override;
  void visit(ast::IfStmt&) override;
  void visit(ast::WhileStmt&) override;
  void visit(ast::ForStmt&) override;
  void visit(ast::ClassDecl&) override;
  void visit(ast::InterfaceDecl&) override;
  void visit(ast::TypeAliasDecl&) override;
  // ...add more as needed...

  // Type node visitors
  void visit(ast::BasicTypeNode&) override;
  void visit(ast::PointerTypeNode&) override;
  void visit(ast::SmartPointerTypeNode&) override;
  void visit(ast::UnionTypeNode&) override;
  void visit(ast::TypeConstraintNode&) override;

 private:
  llvm::LLVMContext context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  struct SymbolInfo {
    llvm::Value* value;
    llvm::Type* type;
    bool isMutable;
    // Resolved semantic type name (e.g., "int", "string", "int*", etc.)
    std::string typeName;
  };
  std::unordered_map<std::string, SymbolInfo> symbolTable;
  llvm::Value* lastValue = nullptr;
  llvm::Function* currentFunction = nullptr;

  // Semantic analyzer for type resolution
  ast::SemanticAnalyzerVisitor* semanticAnalyzer = nullptr;

  // Constant expression evaluator for global initializers
  ast::ConstantExpressionEvaluator constantEvaluator;

  // Track function return types by name (resolved via semantic analyzer)
  std::unordered_map<std::string, std::string> functionReturnTypes;

  // Class metadata for codegen
  struct ClassInfo {
    llvm::StructType* structTy = nullptr;             // class struct type
    std::unordered_map<std::string, int> fieldIndex;  // field name -> index
    std::unordered_map<std::string, llvm::Type*>
        fieldTypes;  // field name -> LLVM type
    bool hasConstructor = false;
    // Track static methods by simple name
    std::unordered_set<std::string> staticMethods;
  };
  std::unordered_map<std::string, ClassInfo> classes;  // class name -> info
  std::vector<std::string>
      classNameStack;  // track current class during emission

  // Helper method to convert semantic type to LLVM type
  llvm::Type* getLLVMType(const std::string& typeName);

  // Helper method to convert ConstantValue to LLVM Constant
  llvm::Constant* constantValueToLLVM(const ast::ConstantValue& constVal);

  // Helpers for classes
  llvm::Type* getPointerTy();
  llvm::Value* emitSizeOfClass(const std::string& className);
  llvm::Value* ensureBitcast(llvm::Value* value, llvm::Type* toTy);
  llvm::Value* emitFieldPtr(llvm::Value* objPtr, const std::string& className,
                            const std::string& fieldName);

  // Deferred global initializers emitted into module ctor
  struct PendingGlobalInit {
    ast::Expr* expr;             // initializer expression AST
    llvm::GlobalVariable* gvar;  // global to initialize
    llvm::Type* type;            // LLVM type for the global
  };
  std::vector<PendingGlobalInit> pendingGlobalInits;
  void emitPendingGlobalInits();

  // ...other helpers for function/variable management...
};

}  // namespace codegen
