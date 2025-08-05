#include "LLVMCodeGenerator.h"

#include <llvm/IR/DerivedTypes.h>  // Required for PointerType
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>

#include <iostream>

#include "parser/nodes/base_node.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/meta_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/nodes/type_nodes.h"

using namespace ast;

namespace codegen {

// Symbol table now stores SymbolInfo (defined in class header)

// Visit ExprStmt: emit code for expression statements (e.g., function calls)
void LLVMCodeGenerator::visit(ExprStmt& node) {
  if (node.expression) node.expression->accept(*this);
}

LLVMCodeGenerator::LLVMCodeGenerator()
    : module(std::make_unique<llvm::Module>("tspp_module", context)),
      builder(std::make_unique<llvm::IRBuilder<>>(context)),
      currentFunction(nullptr),
      semanticAnalyzer(nullptr) {
  module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
}

LLVMCodeGenerator::LLVMCodeGenerator(
    ast::SemanticAnalyzerVisitor* semanticAnalyzer)
    : module(std::make_unique<llvm::Module>("tspp_module", context)),
      builder(std::make_unique<llvm::IRBuilder<>>(context)),
      currentFunction(nullptr),
      semanticAnalyzer(semanticAnalyzer) {
  module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
}

// Helper method to convert semantic type to LLVM type
llvm::Type* LLVMCodeGenerator::getLLVMType(const std::string& typeName) {
  // Handle basic types first
  if (typeName == "int" || typeName == "int32") {
    return llvm::Type::getInt32Ty(context);
  } else if (typeName == "int64") {
    return llvm::Type::getInt64Ty(context);
  } else if (typeName == "int16") {
    return llvm::Type::getInt16Ty(context);
  } else if (typeName == "int8") {
    return llvm::Type::getInt8Ty(context);
  } else if (typeName == "float") {
    return llvm::Type::getFloatTy(context);
  } else if (typeName == "double") {
    return llvm::Type::getDoubleTy(context);
  } else if (typeName == "bool") {
    return llvm::Type::getInt1Ty(context);
  } else if (typeName == "string") {
    return llvm::Type::getInt8Ty(context)->getPointerTo();
  } else if (typeName == "void") {
    return llvm::Type::getVoidTy(context);
  }

  // Handle pointer types (e.g., "int*")
  if (typeName.size() > 1 && typeName.back() == '*') {
    std::string baseType = typeName.substr(0, typeName.size() - 1);
    llvm::Type* baseTypeLLVM = getLLVMType(baseType);
    if (baseTypeLLVM) {
      return baseTypeLLVM->getPointerTo();
    }
  }

  // Default to int32 for unknown types
  std::cerr << "Warning: Unknown type '" << typeName << "', defaulting to int32"
            << std::endl;
  return llvm::Type::getInt32Ty(context);
}

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
  std::cerr << "DEBUG: ProgramNode visit started, declarations count: "
            << node.declarations.size() << std::endl;

  // Handle all declarations - let VarDecl::visit handle global variable
  // creation
  for (auto& decl : node.declarations) {
    if (decl) {
      std::cerr << "DEBUG: Processing declaration type: "
                << typeid(*decl).name() << std::endl;
      decl->accept(*this);
    }
  }

  std::cerr << "DEBUG: ProgramNode declarations processed, statements count: "
            << node.statements.size() << std::endl;

  // Handle all statements (including main function)
  for (auto& stmt : node.statements) {
    if (stmt) {
      stmt->accept(*this);
    }
  }
}

// Visit VarDecl: allocate and initialize variable
void LLVMCodeGenerator::visit(VarDecl& node) {
  std::cerr << "DEBUG: VarDecl::visit called for variable: "
            << node.name.getLexeme() << std::endl;

  // Determine LLVM type using semantic analyzer if available
  llvm::Type* llvmType = nullptr;
  std::string resolvedTypeName;

  std::cerr << "DEBUG: semanticAnalyzer is "
            << (semanticAnalyzer ? "available" : "null") << std::endl;
  std::cerr << "DEBUG: node.type is " << (node.type ? "available" : "null")
            << std::endl;

  if (semanticAnalyzer && node.type) {
    // Use semantic analyzer to resolve the type
    resolvedTypeName = semanticAnalyzer->resolveType(node.type.get());
    llvmType = getLLVMType(resolvedTypeName);
    std::cerr << "DEBUG: Variable '" << node.name.getLexeme()
              << "' resolved type: '" << resolvedTypeName << "'" << std::endl;
  } else {
    // Fallback to old behavior: infer type from initializer
    bool isString = false;
    if (node.initializer) {
      node.initializer->accept(*this);
      // crude check: if lastValue is a pointer to i8, treat as string
      if (lastValue && lastValue->getType()->isPointerTy()) {
        llvm::Type* elemType = nullptr;
        if (auto ptrTy =
                llvm::dyn_cast<llvm::PointerType>(lastValue->getType())) {
          // Opaque pointers: treat as string if the initializer is a string
          // literal (string literals are emitted as i8* by
          // CreateGlobalStringPtr)
          elemType = llvm::Type::getInt8Ty(context);
        }
        if (elemType && elemType->isIntegerTy(8)) {
          llvmType = llvm::Type::getInt8Ty(context)->getPointerTo();
          isString = true;
          resolvedTypeName = "string";
        } else {
          llvmType = llvm::Type::getInt32Ty(context);
          resolvedTypeName = "int";
        }
      } else {
        llvmType = llvm::Type::getInt32Ty(context);
        resolvedTypeName = "int";
      }
    } else {
      llvmType = llvm::Type::getInt32Ty(context);
      resolvedTypeName = "int";
    }
  }

  bool isMutable = !node.isConst;

  if (currentFunction) {
    // Local variable allocation
    llvm::IRBuilder<> tmpBuilder(&currentFunction->getEntryBlock(),
                                 currentFunction->getEntryBlock().begin());
    llvm::Value* alloca =
        tmpBuilder.CreateAlloca(llvmType, nullptr, node.name.getLexeme());
    symbolTable[node.name.getLexeme()] =
        SymbolInfo{alloca, llvmType, isMutable};

    // Initialize if there's an initializer
    if (node.initializer) {
      // Evaluate the initializer
      node.initializer->accept(*this);
      llvm::Value* initVal = lastValue;
      if (initVal) {
        builder->CreateStore(initVal, alloca);
      }
    }
  } else {
    // Global variable allocation
    llvm::Constant* init = nullptr;
    bool isString = (resolvedTypeName == "string");

    // Evaluate initializer first to determine if it's a constant
    llvm::Value* initValue = nullptr;
    if (node.initializer) {
      node.initializer->accept(*this);
      initValue = lastValue;
    }

    // Set appropriate default initializer
    if (isString) {
      init = llvm::ConstantPointerNull::get(
          llvm::Type::getInt8Ty(context)->getPointerTo());
    } else if (resolvedTypeName == "bool") {
      init = llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 0);
    } else if (resolvedTypeName == "float") {
      init = llvm::ConstantFP::get(llvm::Type::getFloatTy(context), 0.0);
    } else if (resolvedTypeName == "double") {
      init = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), 0.0);
    } else {
      // Default to int types
      init = llvm::ConstantInt::get(llvmType, 0);
    }

    // If we have a constant initializer, use it
    if (initValue && llvm::isa<llvm::Constant>(initValue)) {
      init = llvm::cast<llvm::Constant>(initValue);
    }

    auto* gvar = new llvm::GlobalVariable(*module, llvmType, false,
                                          llvm::GlobalValue::ExternalLinkage,
                                          init, node.name.getLexeme());
    symbolTable[node.name.getLexeme()] = SymbolInfo{gvar, llvmType, isMutable};

    // If initializer is not constant, we'll need to store at runtime
    // This should be handled by ProgramNode's deferred initialization logic
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
    // For string literals, we need different handling at global vs function
    // scope
    if (currentFunction) {
      // Inside a function: emit a global string pointer
      lastValue = builder->CreateGlobalStringPtr(val, "", 0, module.get());
    } else {
      // At global scope: create a constant global string
      auto* strConstant = llvm::ConstantDataArray::getString(context, val);
      auto* globalStr = new llvm::GlobalVariable(
          *module, strConstant->getType(), true,
          llvm::GlobalValue::PrivateLinkage, strConstant);
      lastValue = llvm::ConstantExpr::getBitCast(
          globalStr, llvm::Type::getInt8Ty(context)->getPointerTo());
    }
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
    SymbolInfo sym = it->second;
    llvm::Value* ptr = sym.value;
    llvm::Type* type = sym.type;
    if (!ptr || !type) {
      lastValue = nullptr;
      return;
    }
    // If this is a global variable, only load if inside a function
    if (llvm::isa<llvm::GlobalVariable>(ptr)) {
      if (currentFunction) {
        lastValue = builder->CreateLoad(type, ptr);
      } else {
        // At global scope, don't emit a load; just use the global variable
        // pointer
        lastValue = ptr;
      }
    } else {
      // Local variable (alloca)
      lastValue = builder->CreateLoad(type, ptr);
    }
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

  // If we're at global scope (no current function), we can't emit IR
  // instructions This indicates a non-constant global initializer that should
  // be deferred
  if (!currentFunction) {
    lastValue = nullptr;
    return;
  }

  // Ensure both operands are valid
  if (!lhs || !rhs) {
    lastValue = nullptr;
    return;
  }

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
  std::vector<std::pair<llvm::Value*, bool>> argWithFree;
  for (auto& arg : node.arguments) {
    arg->accept(*this);
    llvm::Value* v = lastValue;
    bool needsFree = false;
    if (v->getType()->isIntegerTy(32)) {
      auto intToStrType = llvm::FunctionType::get(
          llvm::Type::getInt8Ty(context)->getPointerTo(),
          {llvm::Type::getInt32Ty(context)}, false);
      auto intToStrFunc =
          module->getOrInsertFunction("tspp_int_to_string", intToStrType);
      v = builder->CreateCall(intToStrFunc, {v});
      needsFree = true;
    } else if (v->getType()->isFloatTy()) {
      auto floatToStrType = llvm::FunctionType::get(
          llvm::Type::getInt8Ty(context)->getPointerTo(),
          {llvm::Type::getFloatTy(context)}, false);
      auto floatToStrFunc =
          module->getOrInsertFunction("tspp_float_to_string", floatToStrType);
      v = builder->CreateCall(floatToStrFunc, {v});
      needsFree = true;
    }
    args.push_back(v);
    argWithFree.push_back({v, needsFree});
  }
  if (funcName == "console.log") {
    auto logType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        {llvm::Type::getInt8Ty(context)->getPointerTo()}, false);
    auto logFunc = module->getOrInsertFunction("tspp_console_log", logType);
    builder->CreateCall(logFunc, args);
    // Now free any heap-allocated strings
    auto freeType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        {llvm::Type::getInt8Ty(context)->getPointerTo()}, false);
    auto freeFunc = module->getOrInsertFunction("tspp_free_string", freeType);
    for (auto& [v, needsFree] : argWithFree) {
      if (needsFree) {
        builder->CreateCall(freeFunc, {v});
      }
    }
    lastValue = nullptr;
  } else if (funcName == "console.error") {
    auto errType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        {llvm::Type::getInt8Ty(context)->getPointerTo()}, false);
    auto errFunc = module->getOrInsertFunction("tspp_console_error", errType);
    builder->CreateCall(errFunc, args);
    lastValue = nullptr;
  } else if (funcName == "console.warn") {
    auto warnType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        {llvm::Type::getInt8Ty(context)->getPointerTo()}, false);
    auto warnFunc = module->getOrInsertFunction("tspp_console_warn", warnType);
    builder->CreateCall(warnFunc, args);
    lastValue = nullptr;
  } else {
    lastValue = nullptr;
  }
}

// Visit IfStmt: emit conditional branching
void LLVMCodeGenerator::visit(IfStmt& node) {
  if (!currentFunction) {
    // Can't emit control flow at global scope
    return;
  }

  // Evaluate the condition
  node.condition->accept(*this);
  llvm::Value* condValue = lastValue;

  if (!condValue) {
    return;  // Invalid condition
  }

  // Create basic blocks for then, else, and merge
  llvm::BasicBlock* thenBB =
      llvm::BasicBlock::Create(context, "then", currentFunction);
  llvm::BasicBlock* elseBB =
      llvm::BasicBlock::Create(context, "else", currentFunction);
  llvm::BasicBlock* mergeBB =
      llvm::BasicBlock::Create(context, "ifcont", currentFunction);

  // Create conditional branch
  builder->CreateCondBr(condValue, thenBB, elseBB);

  // Emit then block
  builder->SetInsertPoint(thenBB);
  if (node.thenBranch) {
    node.thenBranch->accept(*this);
  }
  // Only create branch if block doesn't already have a terminator
  if (!thenBB->getTerminator()) {
    builder->CreateBr(mergeBB);
  }

  // Emit else block
  builder->SetInsertPoint(elseBB);
  if (node.elseBranch) {
    node.elseBranch->accept(*this);
  }
  // Only create branch if block doesn't already have a terminator
  if (!elseBB->getTerminator()) {
    builder->CreateBr(mergeBB);
  }

  // Continue with merge block
  builder->SetInsertPoint(mergeBB);
}

// Add more visit methods as needed for full coverage

void LLVMCodeGenerator::visit(ast::ClassDecl& node) {
  // TODO: Implement class codegen
  // For now, classes are parsed but not used at runtime
  // Could implement as LLVM struct types in the future
  std::cerr << "Warning: ClassDecl '" << node.name.getLexeme()
            << "' parsed but not implemented in codegen (skipped)\n";
}

void LLVMCodeGenerator::visit(ast::InterfaceDecl& node) {
  // TODO: Implement interface codegen
  // Interfaces typically don't generate runtime code by themselves
  // Could be used for type checking in semantic analysis
  std::cerr << "Warning: InterfaceDecl '" << node.name.getLexeme()
            << "' parsed but not implemented in codegen (skipped)\n";
}

void LLVMCodeGenerator::visit(ast::TypeAliasDecl& node) {
  // Type aliases don't generate runtime code - they are resolved at compile
  // time by the semantic analyzer. No action needed in codegen.
  (void)node;  // Suppress unused parameter warning
}

}  // namespace codegen
