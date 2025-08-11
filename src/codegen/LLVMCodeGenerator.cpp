#include "LLVMCodeGenerator.h"

#include <gc.h>
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
  // Unknown type: default to int32 silently
  return llvm::Type::getInt32Ty(context);
}

void LLVMCodeGenerator::generate(ast::BaseNode* root,
                                 const std::string& outFile) {
  if (!root) return;

  // Emit GC_INIT() at program startup (global constructor)
  // Only emit once per module
  auto gcInitType =
      llvm::FunctionType::get(llvm::Type::getVoidTy(context), {}, false);
  // Boehm GC exposes GC_init; GC_INIT is a macro in C headers, so target
  // GC_init here
  auto gcInitFunc = module->getOrInsertFunction("GC_init", gcInitType);
  // Create a global constructor function that calls GC_INIT
  llvm::Function* ctorFunc = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(context), false),
      llvm::GlobalValue::InternalLinkage, "__tspp_gc_ctor", module.get());
  llvm::BasicBlock* ctorBB =
      llvm::BasicBlock::Create(context, "entry", ctorFunc);
  llvm::IRBuilder<> ctorBuilder(ctorBB);
  ctorBuilder.CreateCall(gcInitFunc);
  ctorBuilder.CreateRetVoid();
  // Register the constructor in llvm.global_ctors (3-field form: {i32, ptr,
  // ptr})
  llvm::Constant* ctorPriority =
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 65535);
  llvm::PointerType* int8PtrTy =
      llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0);
  llvm::Constant* ctorCast =
      llvm::ConstantExpr::getBitCast(ctorFunc, int8PtrTy);
  llvm::Constant* nullPtr = llvm::ConstantPointerNull::get(int8PtrTy);
  llvm::StructType* ctorStructTy = llvm::StructType::get(
      context, {llvm::Type::getInt32Ty(context), int8PtrTy, int8PtrTy});
  llvm::Constant* ctorStruct = llvm::ConstantStruct::get(
      ctorStructTy, {ctorPriority, ctorCast, nullPtr});
  llvm::ArrayType* ctorArrayTy = llvm::ArrayType::get(ctorStructTy, 1);
  auto* gCtor = new llvm::GlobalVariable(
      *module, ctorArrayTy, true, llvm::GlobalValue::AppendingLinkage,
      llvm::ConstantArray::get(ctorArrayTy, {ctorStruct}), "llvm.global_ctors");
  (void)gCtor;

  root->accept(*this);
  llvm::verifyModule(*module, &llvm::errs());
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

  // Explicitly reset smart pointers to force destruction before exit (fixes
  // LeakSanitizer false positive)
  builder.reset();
  module.reset();
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
    llvm::Value* alloca = nullptr;
    if (llvmType->isPointerTy()) {
      // Emit call to GC_MALLOC for pointer types
      auto mallocType = llvm::FunctionType::get(
          llvmType, {llvm::Type::getInt64Ty(context)}, false);
      auto gcMallocFunc = module->getOrInsertFunction("GC_MALLOC", mallocType);
      // Use DataLayout to get the correct allocation size
      llvm::Type* pointeeType =
          llvm::Type::getInt8Ty(context);  // Default to i8
      if (auto ptrTy = llvm::dyn_cast<llvm::PointerType>(llvmType)) {
        // Try to get the element type from semantic analysis
        if (node.type && semanticAnalyzer) {
          std::string baseTypeName =
              semanticAnalyzer->resolveType(node.type.get());
          if (!baseTypeName.empty() && baseTypeName.back() == '*') {
            baseTypeName = baseTypeName.substr(0, baseTypeName.size() - 1);
          }
          llvm::Type* baseType = getLLVMType(baseTypeName);
          if (baseType) pointeeType = baseType;
        }
      }
      const llvm::DataLayout& dl = module->getDataLayout();
      uint64_t typeSize = dl.getTypeAllocSize(pointeeType);
      llvm::Value* sizeVal =
          llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), typeSize);
      alloca =
          builder->CreateCall(gcMallocFunc, {sizeVal}, node.name.getLexeme());
    } else {
      alloca =
          tmpBuilder.CreateAlloca(llvmType, nullptr, node.name.getLexeme());
    }
    symbolTable[node.name.getLexeme()] =
        SymbolInfo{alloca, llvmType, isMutable, resolvedTypeName};

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
    symbolTable[node.name.getLexeme()] =
        SymbolInfo{gvar, llvmType, isMutable, resolvedTypeName};

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

  // Determine if we should use floating-point math by inspecting operand types
  auto lhsTy = lhs->getType();
  auto rhsTy = rhs->getType();
  bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();

  // Simple promotion: if either side is double, convert the other to double;
  // else if either is float, convert the other to float
  if (isFP) {
    if (lhsTy->isDoubleTy() || rhsTy->isDoubleTy()) {
      if (!lhsTy->isDoubleTy())
        lhs = builder->CreateSIToFP(lhs, llvm::Type::getDoubleTy(context),
                                    "sitofp");
      if (!rhsTy->isDoubleTy())
        rhs = builder->CreateSIToFP(rhs, llvm::Type::getDoubleTy(context),
                                    "sitofp");
      lhsTy = rhsTy = llvm::Type::getDoubleTy(context);
    } else {
      // Use float
      if (!lhsTy->isFloatTy())
        lhs = builder->CreateSIToFP(lhs, llvm::Type::getFloatTy(context),
                                    "sitofp");
      if (!rhsTy->isFloatTy())
        rhs = builder->CreateSIToFP(rhs, llvm::Type::getFloatTy(context),
                                    "sitofp");
      lhsTy = rhsTy = llvm::Type::getFloatTy(context);
    }
  }

  if (op == "+") {
    lastValue = isFP ? builder->CreateFAdd(lhs, rhs, "faddtmp")
                     : builder->CreateAdd(lhs, rhs, "addtmp");
  } else if (op == "-") {
    lastValue = isFP ? builder->CreateFSub(lhs, rhs, "fsubtmp")
                     : builder->CreateSub(lhs, rhs, "subtmp");
  } else if (op == "*") {
    lastValue = isFP ? builder->CreateFMul(lhs, rhs, "fmultmp")
                     : builder->CreateMul(lhs, rhs, "multmp");
  } else if (op == "/") {
    lastValue = isFP ? builder->CreateFDiv(lhs, rhs, "fdivtmp")
                     : builder->CreateSDiv(lhs, rhs, "divtmp");
  } else if (op == "%") {
    lastValue = isFP ? builder->CreateFRem(lhs, rhs, "fmodtmp")
                     : builder->CreateSRem(lhs, rhs, "modtmp");
  } else if (op == "<") {
    lastValue = isFP ? builder->CreateFCmpOLT(lhs, rhs, "fcmpolt")
                     : builder->CreateICmpSLT(lhs, rhs, "icmpolt");
  } else if (op == ">") {
    lastValue = isFP ? builder->CreateFCmpOGT(lhs, rhs, "fcmpogt")
                     : builder->CreateICmpSGT(lhs, rhs, "icmpogt");
  } else if (op == "<=") {
    lastValue = isFP ? builder->CreateFCmpOLE(lhs, rhs, "fcmpole")
                     : builder->CreateICmpSLE(lhs, rhs, "icmpole");
  } else if (op == ">=") {
    lastValue = isFP ? builder->CreateFCmpOGE(lhs, rhs, "fcmpoge")
                     : builder->CreateICmpSGE(lhs, rhs, "icmpoge");
  } else if (op == "==") {
    lastValue = isFP ? builder->CreateFCmpOEQ(lhs, rhs, "fcmpeq")
                     : builder->CreateICmpEQ(lhs, rhs, "icmpeq");
  } else if (op == "!=") {
    lastValue = isFP ? builder->CreateFCmpONE(lhs, rhs, "fcmpne")
                     : builder->CreateICmpNE(lhs, rhs, "icmpne");
  } else {
    lastValue = nullptr;
  }
}

// Visit FunctionDecl: emit function definition
void LLVMCodeGenerator::visit(FunctionDecl& node) {
  // Resolve return type using semantic analyzer if available
  std::string resolvedRetType = "int";
  llvm::Type* retType = llvm::Type::getInt32Ty(context);
  if (semanticAnalyzer && node.returnType) {
    resolvedRetType = semanticAnalyzer->resolveType(node.returnType.get());
    retType = getLLVMType(resolvedRetType);
  }
  // Record function return type for later call-site conversions
  functionReturnTypes[node.name.getLexeme()] = resolvedRetType;

  std::vector<llvm::Type*> argTypes;
  for (auto& param : node.params) {
    std::string paramTypeName = "int";
    llvm::Type* paramType = llvm::Type::getInt32Ty(context);
    if (semanticAnalyzer && param->type) {
      paramTypeName = semanticAnalyzer->resolveType(param->type.get());
      paramType = getLLVMType(paramTypeName);
    }
    argTypes.push_back(paramType);
  }

  auto funcType = llvm::FunctionType::get(retType, argTypes, false);
  auto func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage,
                                     node.name.getLexeme(), module.get());
  auto entry = llvm::BasicBlock::Create(context, "entry", func);
  builder->SetInsertPoint(entry);

  auto* prevFunction = currentFunction;
  currentFunction = func;

  // Bind parameters to local allocas and add to symbol table
  unsigned idx = 0;
  for (auto& param : node.params) {
    llvm::Argument* arg = func->getArg(idx++);
    arg->setName(param->name.getLexeme());
    llvm::IRBuilder<> tmpBuilder(&entry->getParent()->getEntryBlock(),
                                 entry->getParent()->getEntryBlock().begin());
    llvm::Value* alloca =
        tmpBuilder.CreateAlloca(arg->getType(), nullptr, arg->getName());
    builder->CreateStore(arg, alloca);
    // Store parameter with resolved semantic type name
    std::string paramResolvedName = "int";
    if (semanticAnalyzer && param->type) {
      paramResolvedName = semanticAnalyzer->resolveType(param->type.get());
    }
    symbolTable[param->name.getLexeme()] =
        SymbolInfo{alloca, arg->getType(), true, paramResolvedName};
  }

  if (node.body) node.body->accept(*this);

  // If no explicit return, emit default value for the return type
  if (!entry->getTerminator()) {
    if (retType->isIntegerTy(1)) {
      builder->CreateRet(llvm::ConstantInt::getFalse(context));
    } else if (retType->isIntegerTy()) {
      builder->CreateRet(llvm::ConstantInt::get(retType, 0));
    } else if (retType->isFloatTy()) {
      builder->CreateRet(llvm::ConstantFP::get(retType, 0.0));
    } else if (retType->isDoubleTy()) {
      builder->CreateRet(llvm::ConstantFP::get(retType, 0.0));
    } else if (retType->isPointerTy()) {
      builder->CreateRet(llvm::ConstantPointerNull::get(
          llvm::cast<llvm::PointerType>(retType)));
    } else if (retType->isVoidTy()) {
      builder->CreateRetVoid();
    } else {
      // Default to zero for unknown types
      builder->CreateRet(llvm::Constant::getNullValue(retType));
    }
  }

  currentFunction = prevFunction;
}
// Visit ReturnStmt: emit return
void LLVMCodeGenerator::visit(ReturnStmt& node) {
  // do nothing if we’re not inside a block or the block already has a
  // terminator
  auto* currentBB = builder->GetInsertBlock();
  if (!currentBB || currentBB->getTerminator()) {
    return;
  }
  // evaluate the returned expression if present
  if (node.value) {
    node.value->accept(*this);
    if (lastValue) builder->CreateRet(lastValue);
  } else {
    builder->CreateRetVoid();
  }
}

// Visit BlockStmt: emit all statements
void LLVMCodeGenerator::visit(BlockStmt& node) {
  // run through each statement, but stop after emitting a terminator
  for (auto& stmt : node.statements) {
    auto* bb = builder->GetInsertBlock();
    if (bb && bb->getTerminator()) break;
    if (stmt) stmt->accept(*this);
  }
}

// Visit CallExpr: emit function call (including stdlib like console.log)
void LLVMCodeGenerator::visit(CallExpr& node) {
  // Only support direct identifier or member access for now
  // If at global scope, skip generating calls (invalid outside functions)
  if (!currentFunction) {
    lastValue = nullptr;
    return;
  }
  std::string funcName;
  // Attempt to determine callee's return type to help with argument conversions
  std::string calleeRetType;
  if (auto ident = dynamic_cast<IdentifierExpr*>(node.callee.get())) {
    funcName = ident->name.getLexeme();
    auto itFn = functionReturnTypes.find(funcName);
    if (itFn != functionReturnTypes.end()) {
      calleeRetType = itFn->second;
    }
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
    // Look up a semantic type name if available (identifiers, calls, literals)
    std::string semanticTypeName;
    if (auto idArg = dynamic_cast<IdentifierExpr*>(arg.get())) {
      auto itSym = symbolTable.find(idArg->name.getLexeme());
      if (itSym != symbolTable.end()) {
        semanticTypeName = itSym->second.typeName;
      }
    } else if (auto callArg = dynamic_cast<CallExpr*>(arg.get())) {
      // Determine inner callee name and look up its return type
      std::string innerName;
      if (auto innerIdent =
              dynamic_cast<IdentifierExpr*>(callArg->callee.get())) {
        innerName = innerIdent->name.getLexeme();
      } else if (auto innerMember =
                     dynamic_cast<MemberAccessExpr*>(callArg->callee.get())) {
        if (auto objIdent =
                dynamic_cast<IdentifierExpr*>(innerMember->object.get())) {
          innerName = objIdent->name.getLexeme() + "." +
                      innerMember->member.getLexeme();
        }
      }
      auto itFn2 = functionReturnTypes.find(innerName);
      if (itFn2 != functionReturnTypes.end()) {
        semanticTypeName = itFn2->second;
      }
    } else if (auto litArg = dynamic_cast<LiteralExpr*>(arg.get())) {
      using tokens::TokenType;
      if (litArg->value.getType() == TokenType::STRING_LITERAL) {
        semanticTypeName = "string";
      }
    }

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
    } else if (v->getType()->isPointerTy()) {
      // Decide conversion for pointers based on semantic type name
      bool isString =
          (!semanticTypeName.empty() && semanticTypeName == "string");
      bool isSemanticPointer =
          (!semanticTypeName.empty() && semanticTypeName.back() == '*');
      bool shouldConvert =
          (!isString) && (semanticTypeName.empty() || isSemanticPointer);
      if (shouldConvert) {
        // Convert raw pointer (non-string) to string representation
        auto ptrToStrType = llvm::FunctionType::get(
            llvm::PointerType::get(context, 0),
            {llvm::PointerType::get(context, 0)}, false);
        auto ptrToStrFunc =
            module->getOrInsertFunction("tspp_ptr_to_string", ptrToStrType);
        v = builder->CreateCall(ptrToStrFunc, {v});
        needsFree = true;
      }
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
  llvm::BasicBlock* mergeBB =
      llvm::BasicBlock::Create(context, "ifcont", currentFunction);

  llvm::BasicBlock* elseBB = nullptr;
  if (node.elseBranch) {
    elseBB = llvm::BasicBlock::Create(context, "else", currentFunction);
  }

  // Create conditional branch
  if (node.elseBranch) {
    builder->CreateCondBr(condValue, thenBB, elseBB);
  } else {
    builder->CreateCondBr(condValue, thenBB, mergeBB);
  }

  // Emit then block
  builder->SetInsertPoint(thenBB);
  if (node.thenBranch) {
    node.thenBranch->accept(*this);
  }
  // Only create branch if block doesn't already have a terminator
  if (!thenBB->getTerminator()) {
    builder->CreateBr(mergeBB);
  }

  // Emit else block (if it exists)
  if (node.elseBranch && elseBB) {
    builder->SetInsertPoint(elseBB);
    node.elseBranch->accept(*this);
    // Only create branch if block doesn't already have a terminator
    if (!elseBB->getTerminator()) {
      builder->CreateBr(mergeBB);
    }
  }

  // Continue with merge block
  builder->SetInsertPoint(mergeBB);
}

// Visit WhileStmt: emit while loop
void LLVMCodeGenerator::visit(WhileStmt& node) {
  if (!currentFunction) {
    // Can't emit control flow at global scope
    return;
  }

  // Create basic blocks for condition, body, and merge
  llvm::BasicBlock* condBB =
      llvm::BasicBlock::Create(context, "while.cond", currentFunction);
  llvm::BasicBlock* bodyBB =
      llvm::BasicBlock::Create(context, "while.body", currentFunction);
  llvm::BasicBlock* mergeBB =
      llvm::BasicBlock::Create(context, "while.end", currentFunction);

  // Branch to condition block
  builder->CreateBr(condBB);

  // Emit condition block
  builder->SetInsertPoint(condBB);
  if (node.condition) {
    node.condition->accept(*this);
    llvm::Value* condValue = lastValue;

    if (condValue) {
      // Create conditional branch: if true goto body, else goto merge
      builder->CreateCondBr(condValue, bodyBB, mergeBB);
    } else {
      // Invalid condition, exit loop
      builder->CreateBr(mergeBB);
    }
  } else {
    // No condition means infinite loop, always go to body
    builder->CreateBr(bodyBB);
  }

  // Emit body block
  builder->SetInsertPoint(bodyBB);
  if (node.body) {
    node.body->accept(*this);
  }

  // After body, branch back to condition (if no terminator exists)
  if (!bodyBB->getTerminator()) {
    builder->CreateBr(condBB);
  }

  // Continue with merge block
  builder->SetInsertPoint(mergeBB);
}

// Visit ForStmt: emit for loop
void LLVMCodeGenerator::visit(ForStmt& node) {
  if (!currentFunction) {
    // Can't emit control flow at global scope
    return;
  }

  // Create basic blocks for init, condition, body, increment, and merge
  llvm::BasicBlock* initBB = nullptr;
  llvm::BasicBlock* condBB =
      llvm::BasicBlock::Create(context, "for.cond", currentFunction);
  llvm::BasicBlock* bodyBB =
      llvm::BasicBlock::Create(context, "for.body", currentFunction);
  llvm::BasicBlock* incBB =
      llvm::BasicBlock::Create(context, "for.inc", currentFunction);
  llvm::BasicBlock* mergeBB =
      llvm::BasicBlock::Create(context, "for.end", currentFunction);

  // Emit initialization if present
  if (node.init) {
    node.init->accept(*this);
  }

  // Branch to condition block
  builder->CreateBr(condBB);

  // Emit condition block
  builder->SetInsertPoint(condBB);
  if (node.condition) {
    node.condition->accept(*this);
    llvm::Value* condValue = lastValue;

    if (condValue) {
      // Create conditional branch: if true goto body, else goto merge
      builder->CreateCondBr(condValue, bodyBB, mergeBB);
    } else {
      // Invalid condition, exit loop
      builder->CreateBr(mergeBB);
    }
  } else {
    // No condition means infinite loop, always go to body
    builder->CreateBr(bodyBB);
  }

  // Emit body block
  builder->SetInsertPoint(bodyBB);
  if (node.body) {
    node.body->accept(*this);
  }

  // After body, branch to increment (if no terminator exists)
  if (!bodyBB->getTerminator()) {
    builder->CreateBr(incBB);
  }

  // Emit increment block
  builder->SetInsertPoint(incBB);
  if (node.increment) {
    node.increment->accept(*this);
  }

  // After increment, branch back to condition
  if (!incBB->getTerminator()) {
    builder->CreateBr(condBB);
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

          if (valueToStore && ptr) {
            builder->CreateStore(valueToStore, ptr);
            lastValue = valueToStore;  // Assignment expression returns the
                                       // assigned value
          }
        }
      }
    }
  }
  // TODO: Support more complex left-hand sides (dereferencing, member access,
  // etc.)
}

// Visit UnaryExpr: handle unary operators
void LLVMCodeGenerator::visit(UnaryExpr& node) {
  std::string op = node.op.getLexeme();

  if (op == "@") {
    // Address-of operator: get the address of the operand
    if (auto identExpr = dynamic_cast<IdentifierExpr*>(node.operand.get())) {
      std::string varName = identExpr->name.getLexeme();
      auto it = symbolTable.find(varName);
      if (it != symbolTable.end()) {
        lastValue = it->second.value;  // Return the pointer/address
        return;
      }
    }
    // Fallback: could not get address
    lastValue = nullptr;
    return;
  }

  if (op == "*") {
    // Dereference operator: load value from pointer
    if (node.operand) {
      node.operand->accept(*this);
      llvm::Value* ptr = lastValue;
      if (ptr && ptr->getType()->isPointerTy()) {
        // For LLVM 15+, we need to explicitly specify the element type
        llvm::Type* elementType =
            llvm::Type::getInt32Ty(context);  // Default to int32
        if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(ptr)) {
          elementType = allocaInst->getAllocatedType();
        }
        lastValue = builder->CreateLoad(elementType, ptr, "deref");
        return;
      }
    }
    // Fallback: could not dereference
    lastValue = nullptr;
    return;
  }

  // Handle other unary operators
  if (node.operand) {
    node.operand->accept(*this);
    llvm::Value* operandValue = lastValue;

    if (!operandValue) {
      lastValue = nullptr;
      return;
    }

    if (op == "-") {
      if (operandValue->getType()->isIntegerTy()) {
        lastValue = builder->CreateNeg(operandValue, "neg");
      } else if (operandValue->getType()->isFloatingPointTy()) {
        lastValue = builder->CreateFNeg(operandValue, "fneg");
      }
    } else if (op == "!") {
      lastValue = builder->CreateNot(operandValue, "not");
    }
  }
}

// Missing type node visitors
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
  // For now, just process the first type in the union
  // TODO: Implement proper union type handling
  if (!node.types.empty()) {
    node.types[0]->accept(*this);
  }
}

void LLVMCodeGenerator::visit(ast::TypeConstraintNode& node) {
  // Handle type constraint nodes
  // For now, just process the base type
  // TODO: Implement constraint validation
  if (node.base) {
    node.base->accept(*this);
  }
}

void LLVMCodeGenerator::visit(ast::ClassDecl& node) {
  // Basic class declaration handling
  // Not implemented in codegen yet
}

void LLVMCodeGenerator::visit(ast::InterfaceDecl& node) {
  // Basic interface declaration handling
  // Not implemented in codegen yet
}

void LLVMCodeGenerator::visit(ast::TypeAliasDecl& node) {
  // Handle type alias declarations - these are handled by semantic analysis
  // No code generation needed for type aliases themselves
}

// Helper method to convert ConstantValue to LLVM Constant
llvm::Constant* LLVMCodeGenerator::constantValueToLLVM(
    const ast::ConstantValue& constVal) {
  if (constVal.type == ast::ConstantValue::INT) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context),
                                  constVal.asInt());
  } else if (constVal.type == ast::ConstantValue::FLOAT) {
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(context),
                                 constVal.asFloat());
  } else if (constVal.type == ast::ConstantValue::BOOL) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context),
                                  constVal.asBool() ? 1 : 0);
  } else if (constVal.type == ast::ConstantValue::STRING) {
    // For strings, create a global constant and return pointer to it
    llvm::Constant* strConstant =
        llvm::ConstantDataArray::getString(context, constVal.asString());
    auto* globalStr = new llvm::GlobalVariable(
        *module, strConstant->getType(), true,
        llvm::GlobalValue::PrivateLinkage, strConstant, "");
    return llvm::ConstantExpr::getBitCast(globalStr,
                                          llvm::PointerType::get(context, 0));
  }

  // Default case
  return nullptr;
}

}  // namespace codegen
