
#include "LLVMCodeGenerator.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include "parser/nodes/base_node.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/meta_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/nodes/type_nodes.h"

using namespace ast;

namespace codegen {

// Visit ExprStmt: emit code for expression statements (e.g., function calls)
void LLVMCodeGenerator::visit(ExprStmt& node) {
  if (node.expression) node.expression->accept(*this);
}

LLVMCodeGenerator::LLVMCodeGenerator()
    : module(std::make_unique<llvm::Module>("tspp_module", context)),
      builder(std::make_unique<llvm::IRBuilder<>>(context)),
      currentFunction(nullptr) {}

void LLVMCodeGenerator::generate(ast::BaseNode* root,
                                 const std::string& outFile) {
  if (!root) return;
  root->accept(*this);
  llvm::verifyModule(*module, &llvm::errs());
  // Print to stdout as before
  module->print(llvm::outs(), nullptr);
  // Write to file if requested
  if (!outFile.empty()) {
    std::error_code EC;
    llvm::raw_fd_ostream out(outFile, EC);
    if (!EC) {
      module->print(out, nullptr);
    } else {
      llvm::errs() << "Could not write IR to file: " << outFile << "\n";
    }
  }
}

// Visit ProgramNode: emit all declarations
void LLVMCodeGenerator::visit(ProgramNode& node) {
  for (auto& decl : node.declarations) {
    if (decl) decl->accept(*this);
  }
}

// Visit VarDecl: allocate and initialize variable
void LLVMCodeGenerator::visit(VarDecl& node) {
  // Only handle int for now
  llvm::Type* llvmType = llvm::Type::getInt32Ty(context);
  if (currentFunction) {
    // Local variable: allocate in function entry block
    llvm::IRBuilder<> tmpBuilder(&currentFunction->getEntryBlock(),
                                 currentFunction->getEntryBlock().begin());
    llvm::Value* alloca =
        tmpBuilder.CreateAlloca(llvmType, nullptr, node.name.getLexeme());
    symbolTable[node.name.getLexeme()] = alloca;
    if (node.initializer) {
      node.initializer->accept(*this);
      llvm::Value* initVal = lastValue;
      builder->CreateStore(initVal, alloca);
    }
  } else {
    // Global variable
    llvm::Constant* init = llvm::ConstantInt::get(llvmType, 0);
    if (node.initializer) {
      node.initializer->accept(*this);
      if (auto c = llvm::dyn_cast<llvm::Constant>(lastValue)) {
        init = c;
      }
    }
    auto* gvar = new llvm::GlobalVariable(*module, llvmType,
                                          false,  // isConstant
                                          llvm::GlobalValue::ExternalLinkage,
                                          init, node.name.getLexeme());
    symbolTable[node.name.getLexeme()] = gvar;
  }
}

// Visit LiteralExpr: emit constant (int, bool, string)
void LLVMCodeGenerator::visit(LiteralExpr& node) {
  using tokens::TokenType;
  TokenType type = node.value.getType();
  std::string val = node.value.getLexeme();
  if (type == TokenType::TRUE) {
    lastValue = llvm::ConstantInt::getTrue(context);
  } else if (type == TokenType::FALSE) {
    lastValue = llvm::ConstantInt::getFalse(context);
  } else if (type == TokenType::STRING_LITERAL) {
    // Emit a global string pointer for string literals
    lastValue = builder->CreateGlobalStringPtr(val);
  } else {
    // Assume integer for now
    int intVal = std::stoi(val);
    lastValue = llvm::ConstantInt::get(context, llvm::APInt(32, intVal));
  }
}

// Visit IdentifierExpr: load variable
void LLVMCodeGenerator::visit(IdentifierExpr& node) {
  auto it = symbolTable.find(node.name.getLexeme());
  if (it != symbolTable.end()) {
    lastValue =
        builder->CreateLoad(llvm::Type::getInt32Ty(context), it->second);
  } else {
    lastValue = nullptr;
  }
}

// Visit BinaryExpr: emit arithmetic/logic
void LLVMCodeGenerator::visit(BinaryExpr& node) {
  node.left->accept(*this);
  llvm::Value* lhs = lastValue;
  node.right->accept(*this);
  llvm::Value* rhs = lastValue;
  std::string op = node.op.getLexeme();
  if (op == "+") {
    lastValue = builder->CreateAdd(lhs, rhs, "addtmp");
  } else if (op == "-") {
    lastValue = builder->CreateSub(lhs, rhs, "subtmp");
  } else if (op == "*") {
    lastValue = builder->CreateMul(lhs, rhs, "multmp");
  } else if (op == "/") {
    lastValue = builder->CreateSDiv(lhs, rhs, "divtmp");
  } else if (op == "<") {
    lastValue = builder->CreateICmpSLT(lhs, rhs, "cmptmp");
  } else if (op == ">") {
    lastValue = builder->CreateICmpSGT(lhs, rhs, "cmptmp");
  } else if (op == "==") {
    lastValue = builder->CreateICmpEQ(lhs, rhs, "cmptmp");
  } else if (op == "!=") {
    lastValue = builder->CreateICmpNE(lhs, rhs, "cmptmp");
  } else {
    lastValue = nullptr;
  }
}

// Visit FunctionDecl: emit function definition
void LLVMCodeGenerator::visit(FunctionDecl& node) {
  // Only handle int->int or bool->bool, no params for now
  llvm::Type* retType = llvm::Type::getInt1Ty(context);  // bool
  std::vector<llvm::Type*> argTypes;
  for (auto& param : node.params) {
    argTypes.push_back(llvm::Type::getInt32Ty(context));
  }
  auto funcType = llvm::FunctionType::get(retType, argTypes, false);
  auto func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                     node.name.getLexeme(), module.get());
  auto entry = llvm::BasicBlock::Create(context, "entry", func);
  builder->SetInsertPoint(entry);
  // Track current function for alloca placement
  auto* prevFunction = currentFunction;
  currentFunction = func;
  // Body
  if (node.body) node.body->accept(*this);
  // For now, always return false if no explicit return
  if (!entry->getTerminator()) {
    builder->CreateRet(llvm::ConstantInt::getFalse(context));
  }
  currentFunction = prevFunction;
}

// Visit ReturnStmt: emit return
void LLVMCodeGenerator::visit(ReturnStmt& node) {
  if (node.value) {
    node.value->accept(*this);
    builder->CreateRet(lastValue);
  }
}

// Visit BlockStmt: emit all statements
void LLVMCodeGenerator::visit(BlockStmt& node) {
  for (auto& stmt : node.statements) {
    if (stmt) stmt->accept(*this);
  }
}

// Visit CallExpr: emit function call (including stdlib like console.log)
void LLVMCodeGenerator::visit(CallExpr& node) {
  // Only support direct identifier or member access for now
  std::string funcName;
  if (auto ident = dynamic_cast<IdentifierExpr*>(node.callee.get())) {
    funcName = ident->name.getLexeme();
  } else if (auto member = dynamic_cast<MemberAccessExpr*>(node.callee.get())) {
    // e.g., console.log
    if (auto objIdent = dynamic_cast<IdentifierExpr*>(member->object.get())) {
      funcName = objIdent->name.getLexeme() + "." + member->member.getLexeme();
    }
  }
  std::vector<llvm::Value*> args;
  for (auto& arg : node.arguments) {
    arg->accept(*this);
    args.push_back(lastValue);
  }
  if (funcName == "console.log") {
    // Declare external if needed
    auto logType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(context))}, false);
    auto logFunc = module->getOrInsertFunction("tspp_console_log", logType);
    builder->CreateCall(logFunc, args);
    lastValue = nullptr;
  } else if (funcName == "console.error") {
    auto errType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(context))}, false);
    auto errFunc = module->getOrInsertFunction("tspp_console_error", errType);
    builder->CreateCall(errFunc, args);
    lastValue = nullptr;
  } else if (funcName == "console.warn") {
    auto warnType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(context))}, false);
    auto warnFunc = module->getOrInsertFunction("tspp_console_warn", warnType);
    builder->CreateCall(warnFunc, args);
    lastValue = nullptr;
  } else {
    // User or other function call (not handled here)
    lastValue = nullptr;
  }
}

// Add more visit methods as needed for full coverage

}  // namespace codegen
