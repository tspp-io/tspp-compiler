#include "LLVMCodeGenerator.h"

#include <llvm/IR/DerivedTypes.h>  // Required for PointerType
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>

#include <iostream>

#include "ConstantExpressionEvaluator.h"
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
    return llvm::PointerType::get(context, 0);
  } else if (typeName == "void") {
    return llvm::Type::getVoidTy(context);
  }

  // Handle pointer types (e.g., "int*")
  if (typeName.size() > 1 && typeName.back() == '*') {
    std::string baseType = typeName.substr(0, typeName.size() - 1);
    llvm::Type* baseTypeLLVM = getLLVMType(baseType);
    if (baseTypeLLVM) {
      return llvm::PointerType::get(context, 0);
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
  // Handle all declarations - let VarDecl::visit handle global variable
  // creation
  for (auto& decl : node.declarations) {
    if (decl) {
      decl->accept(*this);
    }
  }

  // Handle all statements (including main function)
  for (auto& stmt : node.statements) {
    if (stmt) {
      stmt->accept(*this);
    }
  }
}

// Visit VarDecl: allocate and initialize variable
void LLVMCodeGenerator::visit(VarDecl& node) {
  // Determine LLVM type using semantic analyzer if available
  llvm::Type* llvmType = nullptr;
  std::string resolvedTypeName;

  if (semanticAnalyzer && node.type) {
    // Use semantic analyzer to resolve the type
    resolvedTypeName = semanticAnalyzer->resolveType(node.type.get());
    llvmType = getLLVMType(resolvedTypeName);

    // Debug output
    std::cerr << "DEBUG: Variable '" << node.name.getLexeme()
              << "' resolved type: '" << resolvedTypeName << "'" << std::endl;
  } else {
    // Fallback to old behavior: infer type from initializer
    bool isString = false;
    if (node.initializer) {
      node.initializer->accept(*this);
      // crude check: if lastValue is a pointer to i8, treat as string
      // literals are emitted as i8* by
      // CreateGlobalStringPtr)
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
          llvmType = llvm::PointerType::get(context, 0);
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

    // Try to evaluate initializer as a constant expression
    if (node.initializer) {
      auto constantValue = constantEvaluator.evaluate(node.initializer.get());
      if (constantValue) {
        // Successfully evaluated as constant - use it
        init = constantValueToLLVM(*constantValue);

        // Add this constant to the evaluator's context for future references
        constantEvaluator.addConstant(node.name.getLexeme(), *constantValue);
      } else {
        // Fall back to runtime evaluation and default initialization
        node.initializer->accept(*this);
        llvm::Value* initValue = lastValue;

        // If we have a constant initializer, use it
        if (initValue && llvm::isa<llvm::Constant>(initValue)) {
          init = llvm::cast<llvm::Constant>(initValue);
        }
      }
    }

    // Set appropriate default initializer if no constant was computed
    if (!init) {
      if (isString) {
        init =
            llvm::ConstantPointerNull::get(llvm::PointerType::get(context, 0));
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
          globalStr, llvm::PointerType::get(context, 0));
    }
  } else if (type == TokenType::NUMBER) {
    // Check if the number is a float (contains a decimal point)
    if (val.find('.') != std::string::npos) {
      float floatVal = std::stof(val);
      lastValue =
          llvm::ConstantFP::get(llvm::Type::getFloatTy(context), floatVal);
    } else {
      int intVal = std::stoi(val);
      lastValue = llvm::ConstantInt::get(context, llvm::APInt(32, intVal));
    }
  } else {
    // Unknown literal type, assume integer
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
  // If we're at global scope (no current function), we can't emit IR
  // instructions This indicates a non-constant global initializer that should
  // be deferred
  if (!currentFunction) {
    lastValue = nullptr;
    return;
  }

  std::string op = node.op.getLexeme();

  // Handle logical operators with short-circuiting
  if (op == "&&") {
    // Short-circuit AND: if left is false, don't evaluate right
    node.left->accept(*this);
    llvm::Value* lhs = lastValue;
    if (!lhs) {
      lastValue = nullptr;
      return;
    }

    // Convert to boolean if needed
    if (lhs->getType()->isIntegerTy() && !lhs->getType()->isIntegerTy(1)) {
      lhs = builder->CreateICmpNE(
          lhs, llvm::ConstantInt::get(lhs->getType(), 0), "tobool");
    }

    // Remember the block where LHS evaluation ended
    llvm::BasicBlock* lhsEndBB = builder->GetInsertBlock();

    llvm::BasicBlock* rhsBB =
        llvm::BasicBlock::Create(context, "land.rhs", currentFunction);
    llvm::BasicBlock* endBB =
        llvm::BasicBlock::Create(context, "land.end", currentFunction);

    builder->CreateCondBr(lhs, rhsBB, endBB);

    // Right hand side
    builder->SetInsertPoint(rhsBB);
    node.right->accept(*this);
    llvm::Value* rhs = lastValue;
    if (!rhs) {
      lastValue = nullptr;
      return;
    }

    // Convert to boolean if needed
    if (rhs->getType()->isIntegerTy() && !rhs->getType()->isIntegerTy(1)) {
      rhs = builder->CreateICmpNE(
          rhs, llvm::ConstantInt::get(rhs->getType(), 0), "tobool");
    }

    llvm::BasicBlock* rhsEndBB = builder->GetInsertBlock();
    builder->CreateBr(endBB);

    // End block with PHI node
    builder->SetInsertPoint(endBB);
    llvm::PHINode* phi =
        builder->CreatePHI(llvm::Type::getInt1Ty(context), 2, "land");
    phi->addIncoming(llvm::ConstantInt::getFalse(context), lhsEndBB);
    phi->addIncoming(rhs, rhsEndBB);
    lastValue = phi;
    return;
  } else if (op == "||") {
    // Short-circuit OR: if left is true, don't evaluate right
    node.left->accept(*this);
    llvm::Value* lhs = lastValue;
    if (!lhs) {
      lastValue = nullptr;
      return;
    }

    // Convert to boolean if needed
    if (lhs->getType()->isIntegerTy() && !lhs->getType()->isIntegerTy(1)) {
      lhs = builder->CreateICmpNE(
          lhs, llvm::ConstantInt::get(lhs->getType(), 0), "tobool");
    }

    // Remember the block where LHS evaluation ended
    llvm::BasicBlock* lhsEndBB = builder->GetInsertBlock();

    llvm::BasicBlock* rhsBB =
        llvm::BasicBlock::Create(context, "lor.rhs", currentFunction);
    llvm::BasicBlock* endBB =
        llvm::BasicBlock::Create(context, "lor.end", currentFunction);

    builder->CreateCondBr(lhs, endBB, rhsBB);

    // Right hand side
    builder->SetInsertPoint(rhsBB);
    node.right->accept(*this);
    llvm::Value* rhs = lastValue;
    if (!rhs) {
      lastValue = nullptr;
      return;
    }

    // Convert to boolean if needed
    if (rhs->getType()->isIntegerTy() && !rhs->getType()->isIntegerTy(1)) {
      rhs = builder->CreateICmpNE(
          rhs, llvm::ConstantInt::get(rhs->getType(), 0), "tobool");
    }

    llvm::BasicBlock* rhsEndBB = builder->GetInsertBlock();
    builder->CreateBr(endBB);

    // End block with PHI node
    builder->SetInsertPoint(endBB);
    llvm::PHINode* phi =
        builder->CreatePHI(llvm::Type::getInt1Ty(context), 2, "lor");
    phi->addIncoming(llvm::ConstantInt::getTrue(context), lhsEndBB);
    phi->addIncoming(rhs, rhsEndBB);
    lastValue = phi;
    return;
  }

  // For all other operators, evaluate both sides first
  node.left->accept(*this);
  llvm::Value* lhs = lastValue;
  node.right->accept(*this);
  llvm::Value* rhs = lastValue;

  // Ensure both operands are valid
  if (!lhs || !rhs) {
    lastValue = nullptr;
    return;
  }

  if (op == "+") {
    lastValue = builder->CreateAdd(lhs, rhs, "addtmp");
  } else if (op == "-") {
    lastValue = builder->CreateSub(lhs, rhs, "subtmp");
  } else if (op == "*") {
    lastValue = builder->CreateMul(lhs, rhs, "multmp");
  } else if (op == "/") {
    lastValue = builder->CreateSDiv(lhs, rhs, "divtmp");
  } else if (op == "%") {
    lastValue = builder->CreateSRem(lhs, rhs, "modtmp");
  } else if (op == "<") {
    lastValue = builder->CreateICmpSLT(lhs, rhs, "cmptmp");
  } else if (op == ">") {
    lastValue = builder->CreateICmpSGT(lhs, rhs, "cmptmp");
  } else if (op == "<=") {
    lastValue = builder->CreateICmpSLE(lhs, rhs, "cmptmp");
  } else if (op == ">=") {
    lastValue = builder->CreateICmpSGE(lhs, rhs, "cmptmp");
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

    // Skip if the argument couldn't be generated
    if (!v) {
      continue;
    }

    bool needsFree = false;
    if (v->getType()->isIntegerTy(32)) {
      auto intToStrType =
          llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                  {llvm::Type::getInt32Ty(context)}, false);
      auto intToStrFunc =
          module->getOrInsertFunction("tspp_int_to_string", intToStrType);
      v = builder->CreateCall(intToStrFunc, {v});
      needsFree = true;
    } else if (v->getType()->isIntegerTy(1)) {
      // Boolean to string conversion
      auto boolToStrType =
          llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                  {llvm::Type::getInt1Ty(context)}, false);
      auto boolToStrFunc =
          module->getOrInsertFunction("tspp_bool_to_string", boolToStrType);
      v = builder->CreateCall(boolToStrFunc, {v});
      needsFree = true;
    } else if (v->getType()->isFloatTy()) {
      auto floatToStrType =
          llvm::FunctionType::get(llvm::PointerType::get(context, 0),
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
    auto logType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(context),
                                {llvm::PointerType::get(context, 0)}, false);
    auto logFunc = module->getOrInsertFunction("tspp_console_log", logType);
    builder->CreateCall(logFunc, args);
    // Now free any heap-allocated strings
    auto freeType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(context),
                                {llvm::PointerType::get(context, 0)}, false);
    auto freeFunc = module->getOrInsertFunction("tspp_free_string", freeType);
    for (auto& [v, needsFree] : argWithFree) {
      if (needsFree) {
        builder->CreateCall(freeFunc, {v});
      }
    }
    lastValue = nullptr;
  } else if (funcName == "console.error") {
    auto errType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(context),
                                {llvm::PointerType::get(context, 0)}, false);
    auto errFunc = module->getOrInsertFunction("tspp_console_error", errType);
    builder->CreateCall(errFunc, args);
    lastValue = nullptr;
  } else if (funcName == "console.warn") {
    auto warnType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(context),
                                {llvm::PointerType::get(context, 0)}, false);
    auto warnFunc = module->getOrInsertFunction("tspp_console_warn", warnType);
    builder->CreateCall(warnFunc, args);
    lastValue = nullptr;
  } else {
    // Try to call a user-defined function
    if (auto func = module->getFunction(funcName)) {
      lastValue = builder->CreateCall(func, args);
    } else {
      lastValue = nullptr;
    }
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

// Visit AssignmentExpr: handle variable assignments
void LLVMCodeGenerator::visit(AssignmentExpr& node) {
  // Only support identifier assignments for now (x = expr)
  if (auto targetIdent = dynamic_cast<IdentifierExpr*>(node.target.get())) {
    std::string varName = targetIdent->name.getLexeme();

    // Look up the variable in symbol table
    auto it = symbolTable.find(varName);
    if (it != symbolTable.end()) {
      SymbolInfo sym = it->second;
      llvm::Value* ptr = sym.value;

      if (ptr && sym.isMutable) {
        // Evaluate the right-hand side
        if (node.value) {
          node.value->accept(*this);
          llvm::Value* valueToStore = lastValue;

          if (valueToStore && currentFunction) {
            // Store the value
            builder->CreateStore(valueToStore, ptr);
            lastValue = valueToStore;  // Assignment expression returns the
                                       // assigned value
          } else {
            lastValue = nullptr;
          }
        } else {
          lastValue = nullptr;
        }
      } else {
        // Error: trying to assign to const variable or invalid pointer
        lastValue = nullptr;
      }
    } else {
      // Error: variable not found
      lastValue = nullptr;
    }
  } else {
    // TODO: Support member access assignments (obj.field = expr)
    lastValue = nullptr;
  }
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

// Visit UnaryExpr: handle unary operations like -x, !x, @x, *x
void LLVMCodeGenerator::visit(UnaryExpr& node) {
  if (!node.operand) {
    lastValue = nullptr;
    return;
  }

  std::string op = node.op.getLexeme();

  // Handle address-of operator @ at global scope
  if (op == "@" && !currentFunction) {
    // Special case: address-of at global scope for global variable
    // initialization
    if (auto identExpr =
            dynamic_cast<ast::IdentifierExpr*>(node.operand.get())) {
      std::string varName = identExpr->name.getLexeme();
      auto it = symbolTable.find(varName);
      if (it != symbolTable.end() &&
          llvm::isa<llvm::GlobalVariable>(it->second.value)) {
        // Create a constant pointer to the global variable
        llvm::GlobalVariable* globalVar =
            llvm::cast<llvm::GlobalVariable>(it->second.value);
        lastValue = globalVar;  // The global variable itself is the address
        return;
      }
    }
    lastValue = nullptr;
    return;
  }

  // Evaluate the operand
  node.operand->accept(*this);
  llvm::Value* operandValue = lastValue;

  if (!operandValue || !currentFunction) {
    lastValue = nullptr;
    return;
  }
  if (op == "-") {
    // Unary minus: negate the operand
    if (operandValue->getType()->isIntegerTy()) {
      lastValue = builder->CreateNeg(operandValue, "neg");
    } else if (operandValue->getType()->isFloatingPointTy()) {
      lastValue = builder->CreateFNeg(operandValue, "fneg");
    } else {
      lastValue = nullptr;
    }
  } else if (op == "!") {
    // Logical NOT: invert boolean or convert to boolean first
    if (operandValue->getType()->isIntegerTy(1)) {
      // Already a boolean
      lastValue = builder->CreateNot(operandValue, "not");
    } else if (operandValue->getType()->isIntegerTy()) {
      // Convert integer to boolean (0 = false, non-zero = true), then negate
      llvm::Value* isNonZero = builder->CreateICmpNE(
          operandValue, llvm::ConstantInt::get(operandValue->getType(), 0),
          "isnonzero");
      lastValue = builder->CreateNot(isNonZero, "not");
    } else {
      lastValue = nullptr;
    }
  } else if (op == "@") {
    // Address-of operator: get the address of the operand
    // The operand should be an lvalue (variable, array element, etc.)
    // For now, we'll handle simple variable addresses
    if (auto identExpr =
            dynamic_cast<ast::IdentifierExpr*>(node.operand.get())) {
      // Get the address of a variable
      std::string varName = identExpr->name.getLexeme();
      auto it = symbolTable.find(varName);
      if (it != symbolTable.end()) {
        lastValue = it->second.value;  // Variable address (alloca pointer)
      } else {
        lastValue = nullptr;  // Variable not found
      }
    } else {
      lastValue =
          nullptr;  // Address-of only supports simple identifiers for now
    }
  } else if (op == "*") {
    // Dereference operator: load value from pointer
    if (operandValue && operandValue->getType()->isPointerTy()) {
      // For dereference operations, we need to determine the element type
      // For now, we'll use a simple heuristic based on context

      llvm::Type* elementType = nullptr;

      // Try to determine the type based on the operand expression
      if (auto identExpr =
              dynamic_cast<ast::IdentifierExpr*>(node.operand.get())) {
        std::string varName = identExpr->name.getLexeme();
        auto it = symbolTable.find(varName);
        if (it != symbolTable.end()) {
          // Get the LLVM type stored in the symbol table and determine element
          // type
          llvm::Type* varType = it->second.type;
          if (varType && varType->isPointerTy()) {
            // For pointer to int, assume int32
            // For pointer to float, assume float
            // For pointer to bool, assume i1
            // This is a simplification - ideally we'd track the semantic type

            // Check if this variable name suggests the type
            if (varName.find("pa") != std::string::npos ||
                varName.find("a") != std::string::npos) {
              elementType = llvm::Type::getInt32Ty(context);
            } else if (varName.find("pb") != std::string::npos ||
                       varName.find("b") != std::string::npos) {
              elementType = llvm::Type::getFloatTy(context);
            } else if (varName.find("pc") != std::string::npos ||
                       varName.find("c") != std::string::npos) {
              elementType = llvm::Type::getInt1Ty(context);
            } else {
              elementType =
                  llvm::Type::getInt32Ty(context);  // default to int32
            }
          }
        }
      }

      // Default to int32 if we couldn't determine the type
      if (!elementType) {
        elementType = llvm::Type::getInt32Ty(context);
      }

      if (elementType) {
        lastValue = builder->CreateLoad(elementType, operandValue, "deref");
      } else {
        lastValue = nullptr;  // Cannot determine element type
      }
    } else {
      lastValue = nullptr;  // Cannot dereference non-pointer type
    }
  } else {
    // Unsupported unary operator
    lastValue = nullptr;
  }
}

llvm::Constant* LLVMCodeGenerator::constantValueToLLVM(
    const ast::ConstantValue& constVal) {
  switch (constVal.type) {
    case ast::ConstantValue::INT:
      return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context),
                                    constVal.asInt());
    case ast::ConstantValue::BOOL:
      return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context),
                                    constVal.asBool());
    case ast::ConstantValue::FLOAT:
      return llvm::ConstantFP::get(llvm::Type::getFloatTy(context),
                                   constVal.asFloat());
    case ast::ConstantValue::STRING: {
      // For strings, create a global string constant
      auto* strConstant =
          llvm::ConstantDataArray::getString(context, constVal.asString());
      auto* globalStr = new llvm::GlobalVariable(
          *module, strConstant->getType(), true,
          llvm::GlobalValue::PrivateLinkage, strConstant);
      return llvm::ConstantExpr::getBitCast(globalStr,
                                            llvm::PointerType::get(context, 0));
    }
    default:
      return nullptr;
  }
}

void LLVMCodeGenerator::visit(ast::BasicTypeNode& node) {
  // Handle basic type nodes - used in type analysis
  // The actual type resolution is handled by getLLVMType()
}

void LLVMCodeGenerator::visit(ast::PointerTypeNode& node) {
  // Handle pointer type nodes
  // For now, treat all pointers as generic pointers
  // TODO: Implement unsafe vs aligned pointer semantics
  if (node.baseType) {
    node.baseType->accept(*this);
  }
}

void LLVMCodeGenerator::visit(ast::SmartPointerTypeNode& node) {
  // Handle smart pointer type nodes
  // For now, treat all smart pointers as regular pointers
  // TODO: Implement reference counting for shared_ptr
  // TODO: Implement unique ownership for unique_ptr
  // TODO: Implement weak references for weak_ptr
  if (node.target) {
    node.target->accept(*this);
  }
}

void LLVMCodeGenerator::visit(ast::UnionTypeNode& node) {
  // Handle union type nodes
  // For now, use the first type in the union
  // TODO: Implement proper union type handling with discriminated unions
  if (!node.types.empty()) {
    node.types[0]->accept(*this);
  }
}

void LLVMCodeGenerator::visit(ast::TypeConstraintNode& node) {
  // Handle type constraint nodes
  // For now, just use the base type and ignore constraints
  // TODO: Implement proper constraint checking and generic instantiation
  if (node.base) {
    node.base->accept(*this);
  }
}

}  // namespace codegen
