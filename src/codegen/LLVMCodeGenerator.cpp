#include "LLVMCodeGenerator.h"

#include <gc.h>
#include <llvm/IR/DerivedTypes.h>  // Required for PointerType
#include <llvm/IR/Instructions.h>
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
#include "parser/nodes/misc_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/nodes/type_nodes.h"

using namespace ast;

namespace codegen {

// Symbol table now stores SymbolInfo (defined in class header)

// Visit ExprStmt: emit code for expression statements (e.g., function calls)
void LLVMCodeGenerator::visit(ExprStmt& node) {
  if (node.expression) {
    // If the expression is actually a VarDecl (parser sometimes wraps decls
    // as expressions), handle it explicitly so locals get allocated and
    // registered in the symbol table before use.
    if (auto varDecl = dynamic_cast<ast::VarDecl*>(node.expression.get())) {
      varDecl->accept(*this);
    } else {
      node.expression->accept(*this);
    }
  }
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
  } else if (typeName == "bool" || typeName == "boolean") {
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

  // If this matches a known class, return pointer to struct for now
  auto itCls = classes.find(typeName);
  if (itCls != classes.end()) {
    return llvm::PointerType::get(itCls->second.structTy, 0);
  }

  // Default to int32 for unknown types
  // Unknown type: default to int32 silently
  return llvm::Type::getInt32Ty(context);
}

void LLVMCodeGenerator::generate(ast::BaseNode* root,
                                 const std::string& outFile) {
  if (!root) return;

  // Ensure GC_init constructor exists and is registered; reuse if present
  auto ensureCtor = [&]() -> llvm::Function* {
    if (auto* existing = module->getFunction("__tspp_gc_ctor")) return existing;
    auto gcInitType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(context), {}, false);
    auto gcInitFunc = module->getOrInsertFunction("GC_init", gcInitType);
    llvm::Function* ctorFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(context), false),
        llvm::GlobalValue::InternalLinkage, "__tspp_gc_ctor", module.get());
    llvm::BasicBlock* ctorBB =
        llvm::BasicBlock::Create(context, "entry", ctorFunc);
    llvm::IRBuilder<> ctorBuilder(ctorBB);
    ctorBuilder.CreateCall(gcInitFunc);
    ctorBuilder.CreateRetVoid();
    // Register in llvm.global_ctors once
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
    new llvm::GlobalVariable(
        *module, ctorArrayTy, true, llvm::GlobalValue::AppendingLinkage,
        llvm::ConstantArray::get(ctorArrayTy, {ctorStruct}),
        "llvm.global_ctors");
    return ctorFunc;
  };
  ensureCtor();

  root->accept(*this);
  // After visiting root, emit any deferred global initializers inside ctor
  emitPendingGlobalInits();
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

void LLVMCodeGenerator::emitPendingGlobalInits() {
  if (pendingGlobalInits.empty()) return;
  auto* ctor = module->getFunction("__tspp_gc_ctor");
  if (!ctor) return;  // should exist
  llvm::BasicBlock* entry = nullptr;
  if (ctor->empty()) {
    entry = llvm::BasicBlock::Create(context, "entry", ctor);
    builder->SetInsertPoint(entry);
  } else {
    entry = &ctor->getEntryBlock();
    // Replace final ret void with our stores + ret
    ctor->back().getTerminator()->eraseFromParent();
    builder->SetInsertPoint(&ctor->back());
  }

  auto* savedFunc = currentFunction;
  currentFunction = ctor;
  for (auto& init : pendingGlobalInits) {
    if (!init.expr || !init.gvar || !init.type) continue;
    init.expr->accept(*this);
    llvm::Value* v = lastValue;
    if (!v) continue;
    // Cast if needed
    if (v->getType() != init.type) {
      if (v->getType()->isPointerTy() && init.type->isPointerTy()) {
        v = builder->CreatePointerCast(v, init.type);
      } else if (v->getType()->isIntegerTy() && init.type->isIntegerTy()) {
        v = builder->CreateIntCast(v, init.type, true);
      }
    }
    builder->CreateStore(v, init.gvar);
  }
  builder->CreateRetVoid();
  currentFunction = savedFunc;
  pendingGlobalInits.clear();
}

// Visit ProgramNode: emit all declarations
void LLVMCodeGenerator::visit(ProgramNode& node) {
  // Generate globals (VarDecl) first so they are available to subsequent
  // function codegen and calls.
  for (auto& decl : node.declarations) {
    if (!decl) continue;
    if (auto* v = dynamic_cast<ast::VarDecl*>(decl.get())) {
      v->accept(*this);
    }
  }
  for (auto& stmt : node.statements) {
    if (!stmt) continue;
    if (auto* v = dynamic_cast<ast::VarDecl*>(stmt.get())) {
      v->accept(*this);
    } else if (auto* exprStmt = dynamic_cast<ast::ExprStmt*>(stmt.get())) {
      if (exprStmt->expression) {
        if (auto* ve =
                dynamic_cast<ast::VarDecl*>(exprStmt->expression.get())) {
          ve->accept(*this);
        }
      }
    }
  }

  // Then handle non-variable declarations (functions, classes, etc.)
  for (auto& decl : node.declarations) {
    if (!decl) continue;
    if (!dynamic_cast<ast::VarDecl*>(decl.get())) {
      decl->accept(*this);
    }
  }

  // Finally, handle remaining statements (including function calls at top
  // level, if any)
  for (auto& stmt : node.statements) {
    if (!stmt) continue;
    // Skip VarDecl already handled above
    if (dynamic_cast<ast::VarDecl*>(stmt.get())) continue;
    if (auto* exprStmt = dynamic_cast<ast::ExprStmt*>(stmt.get())) {
      if (exprStmt->expression &&
          dynamic_cast<ast::VarDecl*>(exprStmt->expression.get()))
        continue;
    }
    stmt->accept(*this);
  }
}

// Visit VarDecl: allocate and initialize variable
void LLVMCodeGenerator::visit(VarDecl& node) {
  // Determine LLVM type using semantic analyzer if available
  llvm::Type* llvmType = nullptr;
  std::string resolvedTypeName;
  // If there's an initializer, evaluate once up-front so we can refine type
  // decisions (especially for pointers) before allocating storage.
  llvm::Value* initValEarly = nullptr;
  if (node.initializer) {
    node.initializer->accept(*this);
    initValEarly = lastValue;
  }

  if (semanticAnalyzer && node.type) {
    // Use semantic analyzer to resolve the type
    resolvedTypeName = semanticAnalyzer->resolveType(node.type.get());
    llvmType = getLLVMType(resolvedTypeName);
    // If declared type is a union (semantic resolves to first branch), but we
    // have an initializer with a different primitive kind, prefer the
    // initializer's type to avoid lossy casts (e.g., int|float = 5.5)
    if (initValEarly && llvmType &&
        (initValEarly->getType()->isFloatingPointTy() !=
         llvmType->isFloatingPointTy())) {
      if (initValEarly->getType()->isFloatingPointTy()) {
        llvmType = initValEarly->getType();
        resolvedTypeName = llvmType->isDoubleTy() ? std::string("double")
                                                  : std::string("float");
      }
    }
    // If the initializer was a pointer but semantic said non-pointer, trust the
    // initializer and treat as pointer to avoid bad allocas/stores.
    if (initValEarly && initValEarly->getType()->isPointerTy() &&
        !llvmType->isPointerTy()) {
      llvmType = llvm::PointerType::get(context, 0);
      resolvedTypeName = "ptr";
    }
  } else {
    // Fallback to old behavior: infer type from initializer
    bool isString = false;
    (void)isString;
    if (node.initializer) {
      // Use the previously computed initializer value
      if (initValEarly) {
        lastValue = initValEarly;
      } else {
        node.initializer->accept(*this);
      }
      if (lastValue) {
        // If initializer is an object literal, treat as string (stringified
        // object)
        if (dynamic_cast<ast::ObjectLiteralExpr*>(node.initializer.get())) {
          llvmType = llvm::PointerType::get(context, 0);
          resolvedTypeName = "string";
          isString = true;
        }
        // Pointers: preserve pointer-ness instead of falling back to int
        if (lastValue->getType()->isPointerTy()) {
          // If the initializer is a string literal, treat as string
          if (auto lit = dynamic_cast<LiteralExpr*>(node.initializer.get())) {
            using tokens::TokenType;
            if (lit->value.getType() == TokenType::STRING_LITERAL) {
              llvmType = llvm::PointerType::get(context, 0);
              resolvedTypeName = "string";
              isString = true;
            } else {
              llvmType = lastValue->getType();
              resolvedTypeName = "ptr";  // unknown base; treated as pointer
            }
          } else if (dynamic_cast<ast::ObjectLiteralExpr*>(
                         node.initializer.get())) {
            // already handled above
          } else {
            llvmType = lastValue->getType();
            resolvedTypeName = "ptr";
          }
        } else if (lastValue->getType()->isIntegerTy(1)) {
          llvmType = llvm::Type::getInt1Ty(context);
          resolvedTypeName = "bool";
        } else if (lastValue->getType()->isIntegerTy()) {
          llvmType = llvm::Type::getInt32Ty(context);
          resolvedTypeName = "int";
        } else if (lastValue->getType()->isFloatingPointTy()) {
          // Default to float for now
          llvmType = llvm::Type::getFloatTy(context);
          resolvedTypeName = "float";
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
    // Always allocate a stack slot for the variable itself, regardless of
    // whether it's a pointer type. The slot holds the value (which may be a
    // pointer) and prevents incorrect sized heap allocations for pointer
    // variables.
    alloca = tmpBuilder.CreateAlloca(llvmType, nullptr, node.name.getLexeme());
    symbolTable[node.name.getLexeme()] =
        SymbolInfo{alloca, llvmType, isMutable, resolvedTypeName};

    // Initialize if there's an initializer
    if (node.initializer) {
      // Evaluate the initializer
      llvm::Value* initVal = initValEarly;
      if (!initVal) {
        node.initializer->accept(*this);
        initVal = lastValue;
      }
      if (initVal) {
        builder->CreateStore(initVal, alloca);
      }
    }
  } else {
    // Global variable allocation
    llvm::Constant* init = nullptr;
    bool isString = (resolvedTypeName == "string");
    bool initWasConst =
        false;  // track whether initializer was a foldable constant

    // Try to evaluate initializer as a constant expression
    if (node.initializer) {
      auto constantValue = constantEvaluator.evaluate(node.initializer.get());
      if (constantValue) {
        // Successfully evaluated as constant - use it
        init = constantValueToLLVM(*constantValue);
        initWasConst = true;

        // Add this constant to the evaluator's context for future references
        constantEvaluator.addConstant(node.name.getLexeme(), *constantValue);
      } else {
        // Non-constant initializer: will emit at runtime inside module ctor
      }
    }

    // Set appropriate default initializer if no constant was computed
    if (!init) {
      if (isString) {
        init =
            llvm::ConstantPointerNull::get(llvm::PointerType::get(context, 0));
      } else if (llvmType->isPointerTy()) {
        // Default for any pointer-typed global: null pointer of the same type
        init = llvm::ConstantPointerNull::get(
            llvm::cast<llvm::PointerType>(llvmType));
      } else if (resolvedTypeName == "bool") {
        init = llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 0);
      } else if (resolvedTypeName == "float") {
        init = llvm::ConstantFP::get(llvm::Type::getFloatTy(context), 0.0);
      } else if (resolvedTypeName == "double") {
        init = llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), 0.0);
      } else if (llvmType->isIntegerTy()) {
        // Default for integer-typed globals
        init = llvm::ConstantInt::get(llvmType, 0);
      } else {
        // Generic zero initializer for any other type
        init = llvm::Constant::getNullValue(llvmType);
      }
    }

    // Use internal linkage for #static qualified globals
    llvm::GlobalValue::LinkageTypes linkage =
        (node.qualifier == ast::StorageQualifier::Static)
            ? llvm::GlobalValue::InternalLinkage
            : llvm::GlobalValue::ExternalLinkage;
    // If we managed to evaluate a constant but its type doesn't match the
    // global type (e.g., float const for an int global), coerce it.
    if (init) {
      if (auto* cfp = llvm::dyn_cast<llvm::ConstantFP>(init)) {
        if (llvmType->isIntegerTy()) {
          // Truncate toward zero by casting through double->i32 conservatively
          double d = cfp->getValueAPF().convertToDouble();
          long long iv = static_cast<long long>(d);
          init = llvm::ConstantInt::get(llvmType, iv);
        }
      }
    }
    auto* gvar = new llvm::GlobalVariable(*module, llvmType, false, linkage,
                                          init, node.name.getLexeme());
    symbolTable[node.name.getLexeme()] =
        SymbolInfo{gvar, llvmType, isMutable, resolvedTypeName};

    // If initializer was not a constant expression but exists, defer it
    if (node.initializer && !initWasConst) {
      pendingGlobalInits.push_back(
          PendingGlobalInit{node.initializer.get(), gvar, llvmType});
    }
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
    // Strip surrounding quotes and simple escaped quotes for internal storage
    if (val.size() >= 2 && (val.front() == '"' && val.back() == '"')) {
      val = val.substr(1, val.size() - 2);
    }
    // Replace escaped quotes \" with " (basic unescape)
    size_t pos = 0;
    while ((pos = val.find("\\\"", pos)) != std::string::npos) {
      val.replace(pos, 2, "\"");
      pos += 1;
    }
    // For string literals, we need different handling at global vs function
    // scope
    if (currentFunction) {
      // Inside a function: emit a global string pointer of unquoted raw content
      lastValue = builder->CreateGlobalString(val, "");
    } else {
      // At global scope: create a constant global string of unquoted raw
      // content
      auto* strConstant =
          llvm::ConstantDataArray::getString(context, val, true);
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

// Visit NullExpr: produce a null pointer (i8*) that can stand in for 'null'
void LLVMCodeGenerator::visit(ast::NullExpr& node) {
  (void)node;
  lastValue =
      llvm::ConstantPointerNull::get(llvm::PointerType::get(context, 0));
}

// Visit ObjectLiteralExpr: build a string like "{ key: val, ... }"
void LLVMCodeGenerator::visit(ast::ObjectLiteralExpr& node) {
  // If we do not have a current function, this is a global initializer.
  // Emit best-effort: return null (non-constant), and let deferred init handle
  // it if used.
  if (!currentFunction) {
    lastValue =
        llvm::ConstantPointerNull::get(llvm::PointerType::get(context, 0));
    return;
  }

  auto concatFnTy = llvm::FunctionType::get(
      llvm::PointerType::get(context, 0),
      {llvm::PointerType::get(context, 0), llvm::PointerType::get(context, 0)},
      false);
  auto concatFn = module->getOrInsertFunction("tspp_string_concat", concatFnTy);

  auto intToStrTy =
      llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                              {llvm::Type::getInt32Ty(context)}, false);
  auto intToStr = module->getOrInsertFunction("tspp_int_to_string", intToStrTy);

  auto floatToStrTy =
      llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                              {llvm::Type::getFloatTy(context)}, false);
  auto floatToStr =
      module->getOrInsertFunction("tspp_float_to_string", floatToStrTy);

  auto boolToStrTy =
      llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                              {llvm::Type::getInt1Ty(context)}, false);
  auto boolToStr =
      module->getOrInsertFunction("tspp_bool_to_string", boolToStrTy);

  auto ptrToStrTy =
      llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                              {llvm::PointerType::get(context, 0)}, false);
  auto ptrToStr = module->getOrInsertFunction("tspp_ptr_to_string", ptrToStrTy);

  // Helper to convert an expr+value to an i8* string
  auto toCString = [&](ast::Expr* expr, llvm::Value* v) -> llvm::Value* {
    if (!v) return builder->CreateGlobalString("", "");
    // Strings: leave as-is if we can identify them
    bool isString = false;
    if (auto lit = dynamic_cast<ast::LiteralExpr*>(expr)) {
      using tokens::TokenType;
      isString = (lit->value.getType() == TokenType::STRING_LITERAL);
    } else if (auto id = dynamic_cast<ast::IdentifierExpr*>(expr)) {
      auto it = symbolTable.find(id->name.getLexeme());
      if (it != symbolTable.end())
        isString = (it->second.typeName == std::string("string"));
    } else if (auto call = dynamic_cast<ast::CallExpr*>(expr)) {
      std::string innerName;
      if (auto innerIdent =
              dynamic_cast<ast::IdentifierExpr*>(call->callee.get())) {
        innerName = innerIdent->name.getLexeme();
      } else if (auto innerMember =
                     dynamic_cast<ast::MemberAccessExpr*>(call->callee.get())) {
        if (auto objIdent =
                dynamic_cast<ast::IdentifierExpr*>(innerMember->object.get())) {
          innerName = objIdent->name.getLexeme() + "." +
                      innerMember->member.getLexeme();
        }
      }
      auto itFn = functionReturnTypes.find(innerName);
      if (itFn != functionReturnTypes.end())
        isString = (itFn->second == std::string("string"));
    }
    if (v->getType()->isPointerTy()) {
      if (isString) return v;
      // Treat generic pointer as address string
      return builder->CreateCall(ptrToStr, {v});
    }
    if (v->getType()->isIntegerTy()) {
      if (v->getType()->isIntegerTy(1)) {
        return builder->CreateCall(boolToStr, {v});
      }
      llvm::Value* i32 = v;
      if (!v->getType()->isIntegerTy(32)) {
        i32 = builder->CreateIntCast(v, llvm::Type::getInt32Ty(context), true);
      }
      return builder->CreateCall(intToStr, {i32});
    }
    if (v->getType()->isFloatingPointTy()) {
      llvm::Value* f = v;
      if (v->getType()->isDoubleTy()) {
        f = builder->CreateFPTrunc(v, llvm::Type::getFloatTy(context));
      } else if (!v->getType()->isFloatTy()) {
        f = builder->CreateSIToFP(v, llvm::Type::getFloatTy(context));
      }
      return builder->CreateCall(floatToStr, {f});
    }
    // Fallback: make empty
    return builder->CreateGlobalString("", "");
  };

  // Begin with "{ "
  llvm::Value* result = builder->CreateGlobalString("{ ", "");

  for (size_t i = 0; i < node.fields.size(); ++i) {
    const auto& f = node.fields[i];
    // Append key (as-is, without quotes)
    std::string keyLex = f.key.getLexeme();
    // If key was quoted string literal, strip quotes
    if (!keyLex.empty() && keyLex.front() == '"' && keyLex.back() == '"') {
      keyLex = keyLex.substr(1, keyLex.size() - 2);
    }
    llvm::Value* keyStr = builder->CreateGlobalString(keyLex, "");
    result = builder->CreateCall(concatFn, {result, keyStr});
    // Append ': '
    result = builder->CreateCall(
        concatFn, {result, builder->CreateGlobalString(": ", "")});

    // Evaluate value and convert to string
    lastValue = nullptr;
    if (f.value) f.value->accept(*this);
    llvm::Value* v = lastValue;
    llvm::Value* valStr = toCString(f.value.get(), v);

    // For string values, wrap in quotes
    bool valueIsStringLiteral = false;
    if (auto lit = dynamic_cast<ast::LiteralExpr*>(f.value.get())) {
      using tokens::TokenType;
      valueIsStringLiteral =
          (lit->value.getType() == TokenType::STRING_LITERAL);
    } else if (auto id = dynamic_cast<ast::IdentifierExpr*>(f.value.get())) {
      auto it = symbolTable.find(id->name.getLexeme());
      if (it != symbolTable.end())
        valueIsStringLiteral = (it->second.typeName == std::string("string"));
    }

    if (valueIsStringLiteral) {
      auto quote = builder->CreateGlobalString("\"", "");
      result = builder->CreateCall(concatFn, {result, quote});
      result = builder->CreateCall(concatFn, {result, valStr});
      result = builder->CreateCall(concatFn, {result, quote});
    } else {
      result = builder->CreateCall(concatFn, {result, valStr});
    }

    // Append ", " if not last
    if (i + 1 < node.fields.size()) {
      result = builder->CreateCall(
          concatFn, {result, builder->CreateGlobalString(", ", "")});
    }
  }

  // Close with " }" (add space only if any fields)
  if (!node.fields.empty()) {
    result = builder->CreateCall(
        concatFn, {result, builder->CreateGlobalString(" }", "")});
  } else {
    result = builder->CreateGlobalString("{}", "");
  }
  lastValue = result;
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

  // Defensive: ensure both operands exist before visiting
  if (!node.left || !node.right) {
    lastValue = nullptr;
    return;
  }

  std::string op = node.op.getLexeme();

  // Handle logical operators with short-circuiting
  if (op == "&&") {
    // Short-circuit AND: if left is false, don't evaluate right
    if (!node.left) {
      lastValue = nullptr;
      return;
    }
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
    if (!node.right) {
      lastValue = nullptr;
      return;
    }
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
    if (!node.left) {
      lastValue = nullptr;
      return;
    }
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
    if (!node.right) {
      lastValue = nullptr;
      return;
    }
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
  if (!node.left) {
    lastValue = nullptr;
    return;
  }
  node.left->accept(*this);
  llvm::Value* lhs = lastValue;
  if (!node.right) {
    lastValue = nullptr;
    return;
  }
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
    // Special-case string concatenation only if either operand is a string
    // Recursive detector for whether an expression should be treated as string
    // in '+'
    std::function<bool(ast::Expr*)> isStringOperand =
        [&](ast::Expr* expr) -> bool {
      // Quick path: string literal
      if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
        using tokens::TokenType;
        return lit->value.getType() == TokenType::STRING_LITERAL;
      }
      // Recursive: nested binary '+' that itself represents string concat
      if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        if (bin->op.getLexeme() == "+") {
          // If either side qualifies as string operand, treat this as string
          return isStringOperand(bin->left.get()) ||
                 isStringOperand(bin->right.get());
        }
      }
      // Identifier with semantic type 'string'
      if (auto id = dynamic_cast<IdentifierExpr*>(expr)) {
        auto it = symbolTable.find(id->name.getLexeme());
        if (it != symbolTable.end()) {
          return it->second.typeName == std::string("string");
        }
      }
      // Call expression whose return type is known to be string
      if (auto call = dynamic_cast<CallExpr*>(expr)) {
        std::string innerName;
        if (auto innerIdent =
                dynamic_cast<IdentifierExpr*>(call->callee.get())) {
          innerName = innerIdent->name.getLexeme();
        } else if (auto innerMember =
                       dynamic_cast<MemberAccessExpr*>(call->callee.get())) {
          if (auto objIdent =
                  dynamic_cast<IdentifierExpr*>(innerMember->object.get())) {
            innerName = objIdent->name.getLexeme() + "." +
                        innerMember->member.getLexeme();
          }
        }
        auto itFn = functionReturnTypes.find(innerName);
        if (itFn != functionReturnTypes.end()) {
          return itFn->second == std::string("string");
        }
      }
      return false;
    };

    auto toCString = [&](ast::Expr* expr, llvm::Value* v) -> llvm::Value* {
      // If it's already an i8* (string literal or string var), just use it
      if (v->getType()->isPointerTy()) {
        bool isStr = false;
        // Determine semantic if possible
        if (auto id = dynamic_cast<IdentifierExpr*>(expr)) {
          auto it = symbolTable.find(id->name.getLexeme());
          if (it != symbolTable.end())
            isStr = (it->second.typeName == "string");
        } else if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
          using tokens::TokenType;
          isStr = (lit->value.getType() == TokenType::STRING_LITERAL);
        } else if (auto call = dynamic_cast<CallExpr*>(expr)) {
          std::string innerName;
          if (auto innerIdent =
                  dynamic_cast<IdentifierExpr*>(call->callee.get())) {
            innerName = innerIdent->name.getLexeme();
          } else if (auto innerMember =
                         dynamic_cast<MemberAccessExpr*>(call->callee.get())) {
            if (auto objIdent =
                    dynamic_cast<IdentifierExpr*>(innerMember->object.get())) {
              innerName = objIdent->name.getLexeme() + "." +
                          innerMember->member.getLexeme();
            }
          }
          auto itFn = functionReturnTypes.find(innerName);
          if (itFn != functionReturnTypes.end())
            isStr = (itFn->second == "string");
        }
        if (isStr || dynamic_cast<BinaryExpr*>(expr) != nullptr) {
          return v;  // already a (concatenated) string pointer
        }
        // Treat any other pointer as raw pointer -> convert to string address
        auto ptrToStrType = llvm::FunctionType::get(
            llvm::PointerType::get(context, 0),
            {llvm::PointerType::get(context, 0)}, false);
        auto ptrToStrFunc =
            module->getOrInsertFunction("tspp_ptr_to_string", ptrToStrType);
        return builder->CreateCall(ptrToStrFunc, {v});
      }

      // Integer (bool handled separately)
      if (v->getType()->isIntegerTy()) {
        if (v->getType()->isIntegerTy(1)) {
          auto boolToStrType =
              llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                      {llvm::Type::getInt1Ty(context)}, false);
          auto boolToStrFunc =
              module->getOrInsertFunction("tspp_bool_to_string", boolToStrType);
          return builder->CreateCall(boolToStrFunc, {v});
        }
        // Cast integers to i32 for printing
        llvm::Value* i32 = v;
        if (!v->getType()->isIntegerTy(32)) {
          i32 =
              builder->CreateIntCast(v, llvm::Type::getInt32Ty(context), true);
        }
        auto intToStrType =
            llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                    {llvm::Type::getInt32Ty(context)}, false);
        auto intToStrFunc =
            module->getOrInsertFunction("tspp_int_to_string", intToStrType);
        return builder->CreateCall(intToStrFunc, {i32});
      }

      // Floating point: support float/double by converting double -> float for
      // now
      if (v->getType()->isFloatingPointTy()) {
        llvm::Value* f = v;
        if (v->getType()->isDoubleTy()) {
          f = builder->CreateFPTrunc(v, llvm::Type::getFloatTy(context));
        } else if (!v->getType()->isFloatTy()) {
          // Fallback cast
          f = builder->CreateSIToFP(v, llvm::Type::getFloatTy(context));
        }
        auto floatToStrType =
            llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                    {llvm::Type::getFloatTy(context)}, false);
        auto floatToStrFunc =
            module->getOrInsertFunction("tspp_float_to_string", floatToStrType);
        return builder->CreateCall(floatToStrFunc, {f});
      }

      // Unknown type: best-effort cast to pointer for address printing
      if (v->getType()->isPointerTy()) {
        auto ptrToStrType = llvm::FunctionType::get(
            llvm::PointerType::get(context, 0),
            {llvm::PointerType::get(context, 0)}, false);
        auto ptrToStrFunc =
            module->getOrInsertFunction("tspp_ptr_to_string", ptrToStrType);
        return builder->CreateCall(ptrToStrFunc, {v});
      }
      // As a last resort, cast to i32 and print
      llvm::Value* i32 = v;
      if (!v->getType()->isIntegerTy(32)) {
        if (v->getType()->isIntegerTy()) {
          i32 =
              builder->CreateIntCast(v, llvm::Type::getInt32Ty(context), true);
        } else {
          // create a poison i32 to avoid crashes (shouldn't happen normally)
          i32 = llvm::PoisonValue::get(llvm::Type::getInt32Ty(context));
        }
      }
      auto intToStrType =
          llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                  {llvm::Type::getInt32Ty(context)}, false);
      auto intToStrFunc =
          module->getOrInsertFunction("tspp_int_to_string", intToStrType);
      return builder->CreateCall(intToStrFunc, {i32});
    };

    bool leftIsString = isStringOperand(node.left.get());
    bool rightIsString = isStringOperand(node.right.get());
    // Treat '+' as string concatenation if either side is a string literal/expr
    // OR one side is already an i8* and the other is a primitive
    // (int/float/bool/pointer)
    auto looksConcatenable = [&](llvm::Value* V, ast::Expr* expr) -> bool {
      if (!V) return false;
      if (V->getType()->isPointerTy()) {
        // Consider pointer concatenable if semantic type says string OR we will
        // stringify it
        if (auto id = dynamic_cast<IdentifierExpr*>(expr)) {
          auto it = symbolTable.find(id->name.getLexeme());
          if (it != symbolTable.end() && it->second.typeName == "string")
            return true;
        }
        return true;  // generic pointer can be stringified
      }
      if (V->getType()->isIntegerTy() || V->getType()->isFloatingPointTy())
        return true;
      return false;
    };
    bool forceConcat = (!leftIsString && !rightIsString) &&
                       ((lhs->getType()->isPointerTy() &&
                         looksConcatenable(rhs, node.right.get())) ||
                        (rhs->getType()->isPointerTy() &&
                         looksConcatenable(lhs, node.left.get())));
    if (leftIsString || rightIsString || forceConcat) {
      // Convert both operands to C-strings before concatenating
      llvm::Value* lstr = toCString(node.left.get(), lhs);
      llvm::Value* rstr = toCString(node.right.get(), rhs);
      // tspp_string_concat(const char*, const char*) -> char*
      auto concatTy =
          llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                  {llvm::PointerType::get(context, 0),
                                   llvm::PointerType::get(context, 0)},
                                  false);
      auto concatFn =
          module->getOrInsertFunction("tspp_string_concat", concatTy);
      lastValue = builder->CreateCall(concatFn, {lstr, rstr});
      return;
    }
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
    // If it's a union type, lower to string for now
    if (dynamic_cast<ast::UnionTypeNode*>(node.returnType.get()) != nullptr) {
      resolvedRetType = "string";
      retType = getLLVMType(resolvedRetType);  // i8*
    } else {
      resolvedRetType = semanticAnalyzer->resolveType(node.returnType.get());
      retType = getLLVMType(resolvedRetType);
    }
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
  // Attach non-invasive string attributes to preserve source modifiers
  switch (node.modifier) {
    case ast::FunctionModifier::Const:
      func->addFnAttr("tspp.const");
      break;
    case ast::FunctionModifier::Constexpr:
      func->addFnAttr("tspp.constexpr");
      break;
    case ast::FunctionModifier::Zerocast:
      func->addFnAttr("tspp.zerocast");
      break;
    case ast::FunctionModifier::Simd:
      func->addFnAttr("tspp.simd");
      break;
    case ast::FunctionModifier::Prefetch:
      func->addFnAttr("tspp.prefetch");
      break;
    case ast::FunctionModifier::Atomic:
      func->addFnAttr("tspp.atomic");
      break;
    case ast::FunctionModifier::Pinned:
      func->addFnAttr("tspp.pinned");
      break;
    default:
      break;
  }
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
    if (lastValue) {
      // If the function expects a pointer (e.g., string), convert scalars
      // appropriately
      llvm::Type* expectedTy = nullptr;
      if (currentFunction) {
        expectedTy = currentFunction->getReturnType();
      }
      llvm::Value* retVal = lastValue;
      if (expectedTy && expectedTy->isPointerTy()) {
        // Convert ints/bools/floats to strings
        if (retVal->getType()->isIntegerTy()) {
          if (retVal->getType()->isIntegerTy(1)) {
            auto boolToStrType = llvm::FunctionType::get(
                llvm::PointerType::get(context, 0),
                {llvm::Type::getInt1Ty(context)}, false);
            auto boolToStr = module->getOrInsertFunction("tspp_bool_to_string",
                                                         boolToStrType);
            retVal = builder->CreateCall(boolToStr, {retVal});
          } else {
            // Cast to i32
            llvm::Value* i32 = retVal;
            if (!retVal->getType()->isIntegerTy(32)) {
              i32 = builder->CreateIntCast(
                  retVal, llvm::Type::getInt32Ty(context), true);
            }
            auto intToStrType = llvm::FunctionType::get(
                llvm::PointerType::get(context, 0),
                {llvm::Type::getInt32Ty(context)}, false);
            auto intToStr =
                module->getOrInsertFunction("tspp_int_to_string", intToStrType);
            retVal = builder->CreateCall(intToStr, {i32});
          }
        } else if (retVal->getType()->isFloatingPointTy()) {
          llvm::Value* f = retVal;
          if (retVal->getType()->isDoubleTy()) {
            f = builder->CreateFPTrunc(retVal, llvm::Type::getFloatTy(context));
          } else if (!retVal->getType()->isFloatTy()) {
            f = builder->CreateSIToFP(retVal, llvm::Type::getFloatTy(context));
          }
          auto floatToStrType =
              llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                      {llvm::Type::getFloatTy(context)}, false);
          auto floatToStr = module->getOrInsertFunction("tspp_float_to_string",
                                                        floatToStrType);
          retVal = builder->CreateCall(floatToStr, {f});
        } else if (retVal->getType()->isPointerTy()) {
          // Assume pointer already represents string if the function expects
          // string
        } else {
          // Fallback: cast to i32 and stringify
          llvm::Value* i32 = retVal;
          if (!retVal->getType()->isIntegerTy(32)) {
            if (retVal->getType()->isIntegerTy()) {
              i32 = builder->CreateIntCast(
                  retVal, llvm::Type::getInt32Ty(context), true);
            } else {
              i32 = llvm::PoisonValue::get(llvm::Type::getInt32Ty(context));
            }
          }
          auto intToStrType =
              llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                      {llvm::Type::getInt32Ty(context)}, false);
          auto intToStr =
              module->getOrInsertFunction("tspp_int_to_string", intToStrType);
          retVal = builder->CreateCall(intToStr, {i32});
        }
      }
      builder->CreateRet(retVal);
    }
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
    // Handle console.* specially below; otherwise resolve class method
    // Determine class name
    std::string className;
    if (auto objIdent = dynamic_cast<IdentifierExpr*>(member->object.get())) {
      // Builtin console
      if (objIdent->name.getLexeme() == "console") {
        funcName = "console." + member->member.getLexeme();
      } else {
        auto itSym = symbolTable.find(objIdent->name.getLexeme());
        if (itSym != symbolTable.end()) {
          className = itSym->second.typeName;
        }
      }
    } else if (dynamic_cast<ThisExpr*>(member->object.get())) {
      if (!classNameStack.empty()) className = classNameStack.back();
    }
    if (!funcName.size()) {
      // Extract base class name if it's a pointer type name
      if (!className.empty() && className.back() == '*') {
        className.pop_back();
      }
      if (!className.empty()) {
        funcName = className + "." + member->member.getLexeme();
      }
    }
  }
  // Identify stdlib console functions where we need to stringify arguments
  bool isConsole = (funcName == "console.log" || funcName == "console.error" ||
                    funcName == "console.warn");
  std::vector<llvm::Value*> args;
  std::vector<std::pair<llvm::Value*, bool>> argWithFree;
  // If this is a method call on an object (member access), evaluate the object
  // and push as first arg
  if (!isConsole) {
    if (auto member = dynamic_cast<MemberAccessExpr*>(node.callee.get())) {
      bool passThis = true;
      // Determine class and method name
      std::string className;
      if (auto objIdent = dynamic_cast<IdentifierExpr*>(member->object.get())) {
        auto itSym = symbolTable.find(objIdent->name.getLexeme());
        if (itSym != symbolTable.end()) className = itSym->second.typeName;
        if (!className.empty() && className.back() == '*') className.pop_back();
      } else if (dynamic_cast<ThisExpr*>(member->object.get())) {
        if (!classNameStack.empty()) className = classNameStack.back();
      }
      if (!className.empty()) {
        auto itCls = classes.find(className);
        if (itCls != classes.end()) {
          if (itCls->second.staticMethods.count(member->member.getLexeme()) >
              0) {
            passThis = false;
          }
        }
      }
      if (passThis) {
        // Evaluate object expression to get pointer
        lastValue = nullptr;
        member->object->accept(*this);
        llvm::Value* objPtr = lastValue;
        if (objPtr) {
          // Methods expect i8* as first argument; bitcast if needed
          objPtr = ensureBitcast(objPtr, getPointerTy());
          args.push_back(objPtr);
        }
      }
    }
  }
  for (auto& arg : node.arguments) {
    // Reset lastValue defensively before evaluating each arg
    lastValue = nullptr;
    arg->accept(*this);
    llvm::Value* v = lastValue;

    // Skip if the argument couldn't be generated
    if (!v) {
      // Fallback: if the argument is an identifier that refers to a global
      // variable, load it directly from the module
      if (auto idArg = dynamic_cast<IdentifierExpr*>(arg.get())) {
        if (auto* gvar =
                module->getGlobalVariable(idArg->name.getLexeme(), true)) {
          llvm::Type* valTy = gvar->getValueType();
          v = builder->CreateLoad(valTy, gvar);
        }
      }
      if (!v) {
        continue;
      }
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

    if (isConsole) {
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
      // For console logging, print string arguments as-is without extra quotes
    }
    args.push_back(v);
    argWithFree.push_back({v, needsFree});
  }
  if (funcName == "console.log") {
    auto logType =
        llvm::FunctionType::get(llvm::Type::getVoidTy(context),
                                {llvm::PointerType::get(context, 0)}, false);
    auto logFunc = module->getOrInsertFunction("tspp_console_log", logType);
    // If no args were produced but there were AST arguments, try a robust
    // fallback for globals (e.g., static globals not yet in symbol table)
    if (args.empty()) {
      if (!node.arguments.empty()) {
        if (auto idArg =
                dynamic_cast<IdentifierExpr*>(node.arguments[0].get())) {
          if (auto* gvar =
                  module->getGlobalVariable(idArg->name.getLexeme(), true)) {
            llvm::Type* valTy = gvar->getValueType();
            llvm::Value* loaded = builder->CreateLoad(valTy, gvar);
            // Convert to string
            llvm::Value* vstr = nullptr;
            bool needsFree = false;
            if (loaded->getType()->isIntegerTy(1)) {
              auto boolToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::Type::getInt1Ty(context)}, false);
              auto boolToStrFunc = module->getOrInsertFunction(
                  "tspp_bool_to_string", boolToStrType);
              vstr = builder->CreateCall(boolToStrFunc, {loaded});
              needsFree = true;
            } else if (loaded->getType()->isIntegerTy()) {
              llvm::Value* i32 = loaded;
              if (!loaded->getType()->isIntegerTy(32)) {
                i32 = builder->CreateIntCast(
                    loaded, llvm::Type::getInt32Ty(context), true);
              }
              auto intToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::Type::getInt32Ty(context)}, false);
              auto intToStrFunc = module->getOrInsertFunction(
                  "tspp_int_to_string", intToStrType);
              vstr = builder->CreateCall(intToStrFunc, {i32});
              needsFree = true;
            } else if (loaded->getType()->isFloatingPointTy()) {
              llvm::Value* f = loaded;
              if (loaded->getType()->isDoubleTy()) {
                f = builder->CreateFPTrunc(loaded,
                                           llvm::Type::getFloatTy(context));
              } else if (!loaded->getType()->isFloatTy()) {
                f = builder->CreateSIToFP(loaded,
                                          llvm::Type::getFloatTy(context));
              }
              auto floatToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::Type::getFloatTy(context)}, false);
              auto floatToStrFunc = module->getOrInsertFunction(
                  "tspp_float_to_string", floatToStrType);
              vstr = builder->CreateCall(floatToStrFunc, {f});
              needsFree = true;
            } else if (loaded->getType()->isPointerTy()) {
              auto ptrToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::PointerType::get(context, 0)}, false);
              auto ptrToStrFunc = module->getOrInsertFunction(
                  "tspp_ptr_to_string", ptrToStrType);
              vstr = builder->CreateCall(ptrToStrFunc, {loaded});
              needsFree = true;
            }
            if (vstr) {
              args.push_back(vstr);
              argWithFree.push_back({vstr, needsFree});
            }
          }
        }
      }
      // As a last resort, emit an empty line
      if (args.empty()) {
        llvm::Value* emptyStr = builder->CreateGlobalString("", "");
        args.push_back(emptyStr);
      }
    }
    // Concatenate all arguments into a single string
    llvm::Value* finalMsg = nullptr;
    if (args.size() == 1) {
      finalMsg = args[0];
    } else {
      auto concatTy =
          llvm::FunctionType::get(llvm::PointerType::get(context, 0),
                                  {llvm::PointerType::get(context, 0),
                                   llvm::PointerType::get(context, 0)},
                                  false);
      auto concatFn =
          module->getOrInsertFunction("tspp_string_concat", concatTy);
      // Start from first arg, then append each subsequent arg
      finalMsg = args[0];
      for (size_t i = 1; i < args.size(); ++i) {
        finalMsg = builder->CreateCall(concatFn, {finalMsg, args[i]});
      }
    }
    // Ensure we always pass exactly one argument to the runtime logger
    builder->CreateCall(logFunc, {finalMsg});
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
    // Try to call a user-defined function or a lowered method
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
  // llvm::BasicBlock* initBB = nullptr;  // unused
  llvm::BasicBlock* condBB =
      llvm::BasicBlock::Create(context, "for.cond", currentFunction);
  llvm::BasicBlock* bodyBB =
      llvm::BasicBlock::Create(context, "for.body", currentFunction);
  llvm::BasicBlock* incBB =
      llvm::BasicBlock::Create(context, "for.inc", currentFunction);
  llvm::BasicBlock* mergeBB =
      llvm::BasicBlock::Create(context, "for.end", currentFunction);

  // Emit initialization if present (can be a VarDecl or an ExprStmt)
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
  // Identifier assignment
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
  } else if (auto memberTarget =
                 dynamic_cast<MemberAccessExpr*>(node.target.get())) {
    // Support 'obj.field = value' and 'this.field = value'
    // Evaluate RHS first
    if (node.value) {
      node.value->accept(*this);
    }
    llvm::Value* rhs = lastValue;
    // Evaluate object
    if (!memberTarget->object) {
      lastValue = nullptr;
      return;
    }
    memberTarget->object->accept(*this);
    llvm::Value* base = lastValue;
    // Determine class name if possible
    std::string className;
    if (auto id = dynamic_cast<IdentifierExpr*>(memberTarget->object.get())) {
      auto itSym = symbolTable.find(id->name.getLexeme());
      if (itSym != symbolTable.end()) className = itSym->second.typeName;
      // Strip trailing '*' if it's a pointer type
      if (!className.empty() && className.back() == '*') className.pop_back();
    } else if (dynamic_cast<ast::ThisExpr*>(memberTarget->object.get())) {
      if (!classNameStack.empty()) className = classNameStack.back();
    }
    if (base && !className.empty()) {
      llvm::Value* fieldPtr =
          emitFieldPtr(base, className, memberTarget->member.getLexeme());
      if (fieldPtr && rhs) {
        // If types mismatch, try to cast pointer types to i8*
        llvm::Type* elemTy = nullptr;
        if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(fieldPtr)) {
          elemTy = gep->getResultElementType();
        }
        if (!elemTy) {
          // Fallback to declared field type if available
          auto itCls = classes.find(
              classNameStack.empty() ? std::string() : classNameStack.back());
          if (itCls != classes.end()) {
            auto ftIt =
                itCls->second.fieldTypes.find(memberTarget->member.getLexeme());
            if (ftIt != itCls->second.fieldTypes.end()) elemTy = ftIt->second;
          }
        }
        if (!elemTy) elemTy = rhs->getType();
        llvm::Value* toStore = rhs;
        if (rhs->getType() != elemTy) {
          if (rhs->getType()->isPointerTy() && elemTy->isPointerTy()) {
            toStore = builder->CreatePointerCast(rhs, elemTy);
          } else if (rhs->getType()->isIntegerTy() && elemTy->isIntegerTy()) {
            toStore = builder->CreateIntCast(rhs, elemTy, true);
          }
        }
        builder->CreateStore(toStore, fieldPtr);
        lastValue = toStore;
        return;
      }
    }
    lastValue = nullptr;
    return;
  }
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
    // We can only emit loads inside a function (builder needs an insert point)
    if (!currentFunction) {
      lastValue = nullptr;  // non-constant global initializer; skip codegen
      return;
    }

    if (node.operand) {
      node.operand->accept(*this);
      llvm::Value* ptr = lastValue;
      if (ptr && ptr->getType()->isPointerTy()) {
        // Ensure we have a valid insert point
        if (!builder->GetInsertBlock()) {
          llvm::BasicBlock* entry = &currentFunction->getEntryBlock();
          if (!entry)
            entry = llvm::BasicBlock::Create(context, "entry", currentFunction);
          builder->SetInsertPoint(entry);
        }

        // Try to determine the element type:
        // 1) If pointer is an alloca, use allocated type
        // 2) If pointer is a global, use its value type
        // 3) If operand is an identifier for a pointer variable, infer base
        // type from symbol table 4) Fallback to i32
        llvm::Type* elementType = nullptr;

        if (auto* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(ptr)) {
          elementType = allocaInst->getAllocatedType();
        } else if (auto* gv = llvm::dyn_cast<llvm::GlobalVariable>(ptr)) {
          elementType = gv->getValueType();
        } else {
          if (auto* identExpr =
                  dynamic_cast<IdentifierExpr*>(node.operand.get())) {
            auto it = symbolTable.find(identExpr->name.getLexeme());
            if (it != symbolTable.end()) {
              const std::string& tname = it->second.typeName;
              if (!tname.empty() && tname.back() == '*') {
                std::string base = tname.substr(0, tname.size() - 1);
                elementType = getLLVMType(base);
              }
            }
          }
        }

        if (!elementType) {
          elementType =
              llvm::Type::getInt32Ty(context);  // conservative default
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
  (void)node;
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

void LLVMCodeGenerator::visit(ast::GenericTypeNode& node) {
  // Generics are erased in codegen for now
}

void LLVMCodeGenerator::visit(ast::TypeParam& node) {
  // Type parameters have no runtime representation in current codegen
}

void LLVMCodeGenerator::visit(ast::IntersectionTypeNode& node) {
  // For now, no runtime representation distinct from first member type
  if (!node.types.empty()) node.types[0]->accept(*this);
}

void LLVMCodeGenerator::visit(ast::ObjectTypeNode& node) {
  // Object type literals have no runtime type; treat as string-friendly
  (void)node;
}

void LLVMCodeGenerator::visit(ast::ConstructorDecl& node) {
  // We'll lower constructor as a function: __ctor_<ClassName>(i8* this, ...)
  // This visitor is invoked inside ClassDecl emission where current class name
  // is known
  if (classNameStack.empty()) return;
  std::string className = classNameStack.back();

  // Build parameter types: first param is i8* (this)
  std::vector<llvm::Type*> argTypes;
  argTypes.push_back(getPointerTy());
  for (auto& p : node.params) {
    std::string t = "int";
    if (semanticAnalyzer && p->type)
      t = semanticAnalyzer->resolveType(p->type.get());
    argTypes.push_back(getLLVMType(t));
  }
  auto fnTy =
      llvm::FunctionType::get(llvm::Type::getVoidTy(context), argTypes, false);
  std::string ctorName = std::string("__ctor_") + className;
  auto func = module->getFunction(ctorName);
  if (!func) {
    func = llvm::Function::Create(fnTy, llvm::Function::ExternalLinkage,
                                  ctorName, module.get());
  }
  auto entry = llvm::BasicBlock::Create(context, "entry", func);
  builder->SetInsertPoint(entry);
  auto* prevFunction = currentFunction;
  currentFunction = func;

  // Map parameters to allocas
  unsigned idx = 0;
  for (auto& arg : func->args()) {
    std::string pname;
    if (idx == 0)
      pname = "this";
    else
      pname = node.params[idx - 1]->name.getLexeme();
    arg.setName(pname);
    llvm::IRBuilder<> tmpBuilder(&entry->getParent()->getEntryBlock(),
                                 entry->getParent()->getEntryBlock().begin());
    llvm::Value* alloca =
        tmpBuilder.CreateAlloca(arg.getType(), nullptr, pname);
    builder->CreateStore(&arg, alloca);
    symbolTable[pname] =
        SymbolInfo{alloca, arg.getType(), true,
                   (idx == 0 ? std::string(className + "*") : std::string(""))};
    idx++;
  }

  if (node.body) node.body->accept(*this);
  if (!entry->getTerminator()) builder->CreateRetVoid();
  currentFunction = prevFunction;
}

void LLVMCodeGenerator::visit(ast::ClassDecl& node) {
  std::string className = node.name.getLexeme();
  // First pass: define struct type and collect fields
  auto it = classes.find(className);
  if (it == classes.end()) {
    ClassInfo info;
    // Create opaque struct first to allow self-references later if needed
    info.structTy = llvm::StructType::create(context, className);
    classes[className] = info;
  }
  ClassInfo& info = classes[className];

  // Field types: default to i32 when unknown
  std::vector<llvm::Type*> fieldTypesOrdered;
  info.fieldIndex.clear();
  info.fieldTypes.clear();
  for (size_t i = 0; i < node.fields.size(); ++i) {
    auto& f = node.fields[i];
    if (!f) continue;
    std::string tname = "int";
    if (semanticAnalyzer && f->type)
      tname = semanticAnalyzer->resolveType(f->type.get());
    llvm::Type* fty = getLLVMType(tname);
    // For generic pointers, use i8* to make stores flexible
    if (fty && fty->isPointerTy() &&
        fty == llvm::PointerType::get(context, 0)) {
      fty = getPointerTy();
    }
    if (!fty) fty = llvm::Type::getInt32Ty(context);
    info.fieldIndex[f->name.getLexeme()] =
        static_cast<int>(fieldTypesOrdered.size());
    info.fieldTypes[f->name.getLexeme()] = fty;
    fieldTypesOrdered.push_back(fty);
  }
  // Use packed layout if class declared with #packed (either legacy single or
  // in modifiers list)
  bool isPacked = (node.modifier == ast::ClassModifier::Packed);
  if (!isPacked) {
    for (auto m : node.modifiers) {
      if (m == ast::ClassModifier::Packed) {
        isPacked = true;
        break;
      }
    }
  }
  info.structTy->setBody(fieldTypesOrdered, /*isPacked*/ isPacked);

  // Emit methods and constructor
  classNameStack.push_back(className);
  if (node.constructor) {
    info.hasConstructor = true;
    node.constructor->accept(*this);
  }
  for (auto& m : node.methods) {
    if (!m) continue;
    // Lower method as a free function with first arg 'this'
    std::string retName = "int";
    llvm::Type* retTy = llvm::Type::getInt32Ty(context);
    if (semanticAnalyzer && m->returnType) {
      retName = semanticAnalyzer->resolveType(m->returnType.get());
      retTy = getLLVMType(retName);
    }
    // Determine if method is static via attributes
    bool isStatic = false;
    for (auto a : m->methodAttributes) {
      if (a == ast::MethodAttribute::Static) {
        isStatic = true;
        break;
      }
    }
    std::vector<llvm::Type*> argTys;
    if (!isStatic) {
      argTys.push_back(getPointerTy());
    }
    for (auto& p : m->params) {
      std::string t = "int";
      if (semanticAnalyzer && p->type)
        t = semanticAnalyzer->resolveType(p->type.get());
      argTys.push_back(getLLVMType(t));
    }
    auto fnTy = llvm::FunctionType::get(retTy, argTys, false);
    std::string fnName = className + std::string(".") + m->name.getLexeme();
    auto func = module->getFunction(fnName);
    if (!func)
      func = llvm::Function::Create(fnTy, llvm::Function::ExternalLinkage,
                                    fnName, module.get());
    // Attach method attributes for reference
    for (auto a : m->methodAttributes) {
      switch (a) {
        case ast::MethodAttribute::Inline:
          func->addFnAttr("tspp.inline");
          break;
        case ast::MethodAttribute::Virtual:
          func->addFnAttr("tspp.virtual");
          break;
        case ast::MethodAttribute::Override:
          func->addFnAttr("tspp.override");
          break;
        case ast::MethodAttribute::Static:
          info.staticMethods.insert(m->name.getLexeme());
          func->addFnAttr("tspp.static");
          break;
        case ast::MethodAttribute::Abstract:
          func->addFnAttr("tspp.abstract");
          break;
        default:
          break;
      }
    }
    auto entry = llvm::BasicBlock::Create(context, "entry", func);
    builder->SetInsertPoint(entry);
    auto* prevFunction = currentFunction;
    currentFunction = func;

    unsigned idx = 0;
    for (auto& arg : func->args()) {
      std::string pname;
      if (!isStatic && idx == 0)
        pname = "this";
      else
        pname = m->params[isStatic ? idx : (idx - 1)]->name.getLexeme();
      arg.setName(pname);
      llvm::IRBuilder<> tmpBuilder(&entry->getParent()->getEntryBlock(),
                                   entry->getParent()->getEntryBlock().begin());
      llvm::Value* alloca =
          tmpBuilder.CreateAlloca(arg.getType(), nullptr, pname);
      builder->CreateStore(&arg, alloca);
      symbolTable[pname] =
          SymbolInfo{alloca, arg.getType(), true,
                     (!isStatic && idx == 0) ? std::string(className + "*")
                                             : std::string("")};
      idx++;
    }
    if (m->body) m->body->accept(*this);
    if (!entry->getTerminator()) {
      if (retTy->isVoidTy())
        builder->CreateRetVoid();
      else if (retTy->isIntegerTy())
        builder->CreateRet(llvm::ConstantInt::get(retTy, 0));
      else if (retTy->isFloatingPointTy())
        builder->CreateRet(llvm::ConstantFP::get(retTy, 0.0));
      else if (retTy->isPointerTy())
        builder->CreateRet(llvm::ConstantPointerNull::get(
            llvm::cast<llvm::PointerType>(retTy)));
      else
        builder->CreateRet(llvm::Constant::getNullValue(retTy));
    }
    currentFunction = prevFunction;
  }
  classNameStack.pop_back();
}

void LLVMCodeGenerator::visit(ast::InterfaceDecl& node) {
  // Basic interface declaration handling
  // Not implemented in codegen yet
}

void LLVMCodeGenerator::visit(ast::TypeAliasDecl& node) {
  // Handle type alias declarations - these are handled by semantic analysis
  // No code generation needed for type aliases themselves
}

// --- Helpers ---
llvm::Type* LLVMCodeGenerator::getPointerTy() {
  return llvm::PointerType::get(context, 0);  // i8*
}

llvm::Value* LLVMCodeGenerator::emitSizeOfClass(const std::string& className) {
  auto it = classes.find(className);
  if (it == classes.end() || !it->second.structTy) {
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0);
  }
  auto* structTy = it->second.structTy;
  auto* nullPtr =
      llvm::ConstantPointerNull::get(llvm::PointerType::get(structTy, 0));
  llvm::Value* idxOne =
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 1);
  llvm::Value* gep =
      builder->CreateGEP(structTy, nullPtr, {idxOne}, "size.gep");
  return builder->CreatePtrToInt(gep, llvm::Type::getInt64Ty(context), "size");
}

llvm::Value* LLVMCodeGenerator::ensureBitcast(llvm::Value* value,
                                              llvm::Type* toTy) {
  if (!value) return nullptr;
  if (value->getType() == toTy) return value;
  if (value->getType()->isPointerTy() && toTy->isPointerTy()) {
    return builder->CreatePointerCast(value, toTy);
  }
  return value;  // fallback: no-op
}

llvm::Value* LLVMCodeGenerator::emitFieldPtr(llvm::Value* objPtr,
                                             const std::string& className,
                                             const std::string& fieldName) {
  auto it = classes.find(className);
  if (it == classes.end()) return nullptr;
  auto& info = it->second;
  auto idxIt = info.fieldIndex.find(fieldName);
  if (idxIt == info.fieldIndex.end()) return nullptr;
  int index = idxIt->second;
  llvm::Value* typed =
      ensureBitcast(objPtr, llvm::PointerType::get(info.structTy, 0));
  llvm::Value* zero =
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value* idx =
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), index);
  return builder->CreateGEP(info.structTy, typed, {zero, idx}, "field.ptr");
}

// --- New, This, MemberAccess ---
void LLVMCodeGenerator::visit(ast::ThisExpr& node) {
  // Load the current 'this' pointer (stored as local alloca named "this")
  auto it = symbolTable.find("this");
  if (it == symbolTable.end()) {
    lastValue = nullptr;
    return;
  }
  llvm::Value* ptr = it->second.value;
  if (!ptr) {
    lastValue = nullptr;
    return;
  }
  // 'this' local is an alloca of i8*; load it
  if (auto* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(ptr)) {
    llvm::Type* elemTy = allocaInst->getAllocatedType();
    lastValue = builder->CreateLoad(elemTy, ptr);
  } else {
    lastValue = builder->CreateLoad(getPointerTy(), ptr);
  }
}

void LLVMCodeGenerator::visit(ast::MemberAccessExpr& node) {
  // Support only field access on objects for now; method calls handled in
  // CallExpr
  if (!node.object) {
    lastValue = nullptr;
    return;
  }
  node.object->accept(*this);
  llvm::Value* base = lastValue;
  // Determine class name from semantic or from classNameStack if 'this'
  std::string className;
  if (auto id = dynamic_cast<ast::IdentifierExpr*>(node.object.get())) {
    auto itSym = symbolTable.find(id->name.getLexeme());
    if (itSym != symbolTable.end()) className = itSym->second.typeName;
    if (!className.empty() && className.back() == '*') className.pop_back();
  } else if (dynamic_cast<ast::ThisExpr*>(node.object.get())) {
    if (!classNameStack.empty()) className = classNameStack.back();
  }
  if (!base || className.empty()) {
    lastValue = nullptr;
    return;
  }
  llvm::Value* fieldPtr =
      emitFieldPtr(base, className, node.member.getLexeme());
  if (!fieldPtr) {
    lastValue = nullptr;
    return;
  }
  llvm::Type* elemTy = nullptr;
  if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(fieldPtr)) {
    elemTy = gep->getResultElementType();
  }
  if (!elemTy) {
    // Try resolve from class metadata
    auto itCls = classes.find(className);
    if (itCls != classes.end()) {
      auto ftIt = itCls->second.fieldTypes.find(node.member.getLexeme());
      if (ftIt != itCls->second.fieldTypes.end()) elemTy = ftIt->second;
    }
  }
  if (!elemTy) elemTy = getPointerTy();
  lastValue = builder->CreateLoad(elemTy, fieldPtr);
}

void LLVMCodeGenerator::visit(ast::NewExpr& node) {
  // Allocate memory using GC_malloc and call constructor if present
  if (!currentFunction) {
    lastValue = nullptr;
    return;
  }
  std::string className = node.className.getLexeme();
  auto it = classes.find(className);
  if (it == classes.end()) {
    lastValue = nullptr;
    return;
  }
  // size_t arg type depends; we use i64 and rely on platform to widen
  auto gcMallocTy = llvm::FunctionType::get(
      getPointerTy(), {llvm::Type::getInt64Ty(context)}, false);
  // Boehm GC provides GC_malloc (lowercase). Use that symbol for linking.
  auto gcMalloc = module->getOrInsertFunction("GC_malloc", gcMallocTy);
  llvm::Value* sizeV = emitSizeOfClass(className);
  llvm::Value* raw = builder->CreateCall(gcMalloc, {sizeV}, "obj");
  // If constructor exists, call __ctor_ClassName(obj, ...)
  auto& info = it->second;
  if (info.hasConstructor) {
    std::vector<llvm::Value*> args;
    args.push_back(raw);
    for (auto& a : node.arguments) {
      a->accept(*this);
      if (!lastValue) {
        args.push_back(llvm::PoisonValue::get(getPointerTy()));
      } else
        args.push_back(lastValue);
    }
    auto fn = module->getFunction(std::string("__ctor_") + className);
    if (fn) builder->CreateCall(fn, args);
  }
  lastValue = raw;
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
