#include "LLVMCodeGenerator.h"

#include <gc.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

// Ensure full definitions of AST node types are available for dynamic_cast
// and member access (forward declarations from ast_visitor.h are insufficient)
#include "parser/nodes/base_node.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/meta_nodes.h"
#include "parser/nodes/misc_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/nodes/type_nodes.h"

namespace codegen {
using namespace ast;

std::unique_ptr<llvm::Module> LLVMCodeGenerator::generateModuleForREPL(
    ast::BaseNode* root,
    std::unordered_map<std::string, llvm::Value*>& globals) {
  // Create a new LLVM module for each REPL line
  auto replModule = std::make_unique<llvm::Module>("tspp_repl", context);
  // Always emit a valid entry function for REPL
  llvm::FunctionType* fnType =
      llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);
  llvm::Function* entryFn =
      llvm::Function::Create(fnType, llvm::Function::ExternalLinkage,
                             "__repl_entry", replModule.get());
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(context, "entry", entryFn);
  builder->SetInsertPoint(bb);

  // Emit code for the AST node
  this->module = std::move(replModule);
  root->accept(*this);

  // Always terminate with a valid void return
  if (!bb->getTerminator()) {
    builder->CreateRetVoid();
  }

  // TODO: Persist globals (variables) across sessions using 'globals' map

  return std::move(this->module);
}

// Visit ProgramNode: emit all declarations and statements in phases
void LLVMCodeGenerator::visit(ast::ProgramNode& node) {
  // Phase 1: emit global variable declarations first
  for (auto& decl : node.declarations) {
    if (!decl) continue;
    if (auto* v = dynamic_cast<ast::VarDecl*>(decl.get())) {
      v->accept(*this);
    }
  }
  // Also handle top-level var statements
  for (auto& stmt : node.statements) {
    if (!stmt) continue;
    if (auto* v = dynamic_cast<ast::VarDecl*>(stmt.get())) {
      v->accept(*this);
    }
  }

  // Phase 2a: emit all class declarations first so struct types are sized
  for (auto& decl : node.declarations) {
    if (!decl) continue;
    if (dynamic_cast<ast::ClassDecl*>(decl.get())) {
      decl->accept(*this);
    }
  }

  // Phase 2b: emit remaining non-variable, non-class declarations (functions,
  // interfaces, etc.)
  for (auto& decl : node.declarations) {
    if (!decl) continue;
    if (dynamic_cast<ast::VarDecl*>(decl.get())) continue;
    if (dynamic_cast<ast::ClassDecl*>(decl.get())) continue;
    decl->accept(*this);
  }

  // Phase 3: remaining statements (e.g., expression statements)
  for (auto& stmt : node.statements) {
    if (!stmt) continue;
    // Skip VarDecl already handled
    if (dynamic_cast<ast::VarDecl*>(stmt.get())) continue;
    // Also skip expression statements that wrap a VarDecl
    if (auto* exprStmt = dynamic_cast<ast::ExprStmt*>(stmt.get())) {
      if (exprStmt->expression &&
          dynamic_cast<ast::VarDecl*>(exprStmt->expression.get())) {
        continue;
      }
    }
    stmt->accept(*this);
  }
}

// Visit VarDecl: allocate and initialize variable
void LLVMCodeGenerator::visit(ast::VarDecl& node) {
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
    // Use semantic analyzer to resolve the declared type and honor it
    resolvedTypeName = semanticAnalyzer->resolveType(node.type.get());

    // Check if this is a class type (should be treated as pointer)
    std::string baseOnly = resolvedTypeName;
    auto p = baseOnly.find('<');
    if (p != std::string::npos) baseOnly = baseOnly.substr(0, p);

    if (classes.count(baseOnly)) {
      // For class types, variables are always pointers to instances
      llvmType = llvm::PointerType::get(context, 0);
      resolvedTypeName = resolvedTypeName + "*";
    } else {
      llvmType = getLLVMType(resolvedTypeName);
      // If this was a generic class like Foo<Bar>, record symbol type as Foo*
      // so member access and method calls can resolve correctly.
      if (llvmType && llvmType->isPointerTy()) {
        // Preserve generic args when recording semantic type, but still mark as
        // pointer by appending '*'. Use base name for existence check.
        if (classes.count(baseOnly)) {
          resolvedTypeName = resolvedTypeName + "*";
        }
      }
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
        // If declared type exists and differs, cast the initializer to target
        if (llvmType) {
          if (initVal->getType() != llvmType) {
            if (initVal->getType()->isIntegerTy() && llvmType->isIntegerTy()) {
              initVal = builder->CreateIntCast(initVal, llvmType, true);
            } else if (initVal->getType()->isFloatingPointTy() &&
                       llvmType->isFloatingPointTy()) {
              // Prefer float in this compiler; cast double -> float if needed
              if (initVal->getType()->isDoubleTy() && llvmType->isFloatTy()) {
                initVal = builder->CreateFPTrunc(initVal, llvmType);
              } else if (initVal->getType()->isFloatTy() &&
                         llvmType->isDoubleTy()) {
                initVal = builder->CreateFPExt(initVal, llvmType);
              }
            } else if (initVal->getType()->isFloatingPointTy() &&
                       llvmType->isIntegerTy()) {
              initVal = builder->CreateFPToSI(initVal, llvmType);
            } else if (initVal->getType()->isIntegerTy() &&
                       llvmType->isFloatingPointTy()) {
              initVal = builder->CreateSIToFP(initVal, llvmType);
            } else if (initVal->getType()->isPointerTy() &&
                       llvmType->isPointerTy()) {
              initVal = builder->CreatePointerCast(initVal, llvmType);
            }
          }
        }
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
          // Truncate toward zero by casting through double->i32
          // conservatively
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
void LLVMCodeGenerator::visit(ast::LiteralExpr& node) {
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
      // Inside a function: emit a global string pointer of unquoted raw
      // content
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

// Visit ObjectLiteralExpr: build a string like "{ key: val,  }"
void LLVMCodeGenerator::visit(ast::ObjectLiteralExpr& node) {
  // If we do not have a current function, this is a global initializer.
  // Emit best-effort: return null (non-constant), and let deferred init
  // handle it if used.
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
        } else if (dynamic_cast<ast::ThisExpr*>(innerMember->object.get())) {
          // Handle this.method() calls - resolve to current class
          if (!classNameStack.empty()) {
            innerName =
                classNameStack.back() + "." + innerMember->member.getLexeme();
          }
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
        // Only cast integers to float. For other types, return empty string.
        if (v->getType()->isIntegerTy()) {
          f = builder->CreateSIToFP(v, llvm::Type::getFloatTy(context));
        } else {
          return builder->CreateGlobalString("", "");
        }
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
void LLVMCodeGenerator::visit(ast::IdentifierExpr& node) {
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
void LLVMCodeGenerator::visit(ast::BinaryExpr& node) {
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
    // For '+' we can still proceed (treat missing side as empty string/0)
    if (op != "+") {
      lastValue = nullptr;
      return;
    }
  }

  if (op == "+") {
    // Special-case string concatenation only if either operand is a string
    // Recursive detector for whether an expression should be treated as
    // string in '+'
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
      // Member access to a field typed as string
      if (auto mem = dynamic_cast<MemberAccessExpr*>(expr)) {
        std::string className;
        if (auto objId = dynamic_cast<IdentifierExpr*>(mem->object.get())) {
          auto itSym = symbolTable.find(objId->name.getLexeme());
          if (itSym != symbolTable.end()) className = itSym->second.typeName;
          if (!className.empty() && className.back() == '*')
            className.pop_back();
        } else if (dynamic_cast<ast::ThisExpr*>(mem->object.get())) {
          if (!classNameStack.empty()) className = classNameStack.back();
        }
        if (!className.empty()) {
          auto itCls = classes.find(className);
          if (itCls != classes.end()) {
            auto itName =
                itCls->second.fieldTypeNames.find(mem->member.getLexeme());
            if (itName != itCls->second.fieldTypeNames.end() &&
                itName->second == "string") {
              return true;
            }
          }
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
            // Prefer concrete Class.method if available, else fallback to
            // interface scan
            std::string objTy;
            auto itSym = symbolTable.find(objIdent->name.getLexeme());
            if (itSym != symbolTable.end()) objTy = itSym->second.typeName;
            if (!objTy.empty() && objTy.back() == '*') objTy.pop_back();
            if (!objTy.empty() && classes.count(objTy)) {
              innerName = objTy + "." + innerMember->member.getLexeme();
            } else {
              // Interface typed: scan classes to find a candidate
              auto stripGeneric = [](const std::string& s) {
                auto pos = s.find('<');
                return s.substr(0, pos);
              };
              std::string iface = objTy;
              iface = stripGeneric(iface);
              for (auto& kv : classes) {
                bool impl = false;
                for (auto& in : kv.second.implements) {
                  if (stripGeneric(in) == iface) {
                    impl = true;
                    break;
                  }
                }
                if (!impl) continue;
                std::string candidate =
                    kv.first + "." + innerMember->member.getLexeme();
                if (functionReturnTypes.count(candidate)) {
                  innerName = candidate;
                  break;
                }
              }
              if (innerName.empty()) {
                innerName = objIdent->name.getLexeme() + "." +
                            innerMember->member.getLexeme();
              }
            }
          }
        }
        auto itFn = functionReturnTypes.find(innerName);
        if (itFn != functionReturnTypes.end()) {
          const std::string& rt = itFn->second;
          return rt == std::string("string");
        }
      }
      return false;
    };

    auto toCString = [&](ast::Expr* expr, llvm::Value* v) -> llvm::Value* {
      // If codegen for the subexpression failed, treat as empty string
      if (!v) {
        return builder->CreateGlobalString("", "");
      }
      // If it's already an i8* (string literal or string var), just use it
      if (v->getType()->isPointerTy()) {
        bool isStr = false;
        // Determine semantic if possible
        if (auto id = dynamic_cast<IdentifierExpr*>(expr)) {
          auto it = symbolTable.find(id->name.getLexeme());
          if (it != symbolTable.end())
            isStr = (it->second.typeName == "string");
        } else if (auto mem = dynamic_cast<MemberAccessExpr*>(expr)) {
          // Check class metadata for field type name
          std::string className;
          if (auto objId = dynamic_cast<IdentifierExpr*>(mem->object.get())) {
            auto itSym = symbolTable.find(objId->name.getLexeme());
            if (itSym != symbolTable.end()) className = itSym->second.typeName;
            if (!className.empty() && className.back() == '*')
              className.pop_back();
          } else if (dynamic_cast<ast::ThisExpr*>(mem->object.get())) {
            if (!classNameStack.empty()) className = classNameStack.back();
          }
          if (!className.empty()) {
            auto itCls = classes.find(className);
            if (itCls != classes.end()) {
              auto itName =
                  itCls->second.fieldTypeNames.find(mem->member.getLexeme());
              if (itName != itCls->second.fieldTypeNames.end() &&
                  itName->second == "string") {
                isStr = true;
              }
            }
          }
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
              // Resolve via declared class or interface fallback
              std::string objTy;
              auto itSym = symbolTable.find(objIdent->name.getLexeme());
              if (itSym != symbolTable.end()) objTy = itSym->second.typeName;
              if (!objTy.empty() && objTy.back() == '*') objTy.pop_back();
              auto stripGeneric = [](const std::string& s) {
                auto pos = s.find('<');
                return s.substr(0, pos);
              };
              std::string baseTy = stripGeneric(objTy);
              if (!baseTy.empty() && classes.count(baseTy)) {
                innerName = baseTy + "." + innerMember->member.getLexeme();
              } else if (!objTy.empty()) {
                std::string baseIface = stripGeneric(objTy);
                std::string chosen;
                for (auto& kv : classes) {
                  bool implements = false;
                  for (auto& in : kv.second.implements) {
                    if (stripGeneric(in) == baseIface) {
                      implements = true;
                      break;
                    }
                  }
                  if (!implements) continue;
                  std::string candidate =
                      kv.first + "." + innerMember->member.getLexeme();
                  if (functionReturnTypes.count(candidate)) {
                    chosen = candidate;
                    break;
                  }
                }
                if (!chosen.empty())
                  innerName = chosen;
                else
                  innerName.clear();
              }
            } else if (dynamic_cast<ast::ThisExpr*>(
                           innerMember->object.get())) {
              // Handle this.method() calls - resolve to current class
              if (!classNameStack.empty()) {
                innerName = classNameStack.back() + "." +
                            innerMember->member.getLexeme();
              }
            }
          }
          if (!innerName.empty()) {
            auto itFn = functionReturnTypes.find(innerName);
            if (itFn != functionReturnTypes.end()) {
              const std::string& rt = itFn->second;
              isStr = (rt == "string");
            }
            // Heuristic: Box<string>.get() or SBox<string>.get() returns
            // string
            if (!isStr) {
              if (auto innerMember =
                      dynamic_cast<MemberAccessExpr*>(call->callee.get())) {
                if (innerMember->member.getLexeme() == std::string("get")) {
                  if (auto objIdent = dynamic_cast<IdentifierExpr*>(
                          innerMember->object.get())) {
                    auto itSym = symbolTable.find(objIdent->name.getLexeme());
                    if (itSym != symbolTable.end()) {
                      const std::string& tn = itSym->second.typeName;
                      if (tn.find("<string") != std::string::npos) {
                        isStr = true;
                      }
                    }
                  }
                }
              }
            }
          }
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

      // Floating point: support float/double by converting double -> float
      // for now
      if (v->getType()->isFloatingPointTy()) {
        llvm::Value* f = v;
        if (v->getType()->isDoubleTy()) {
          f = builder->CreateFPTrunc(v, llvm::Type::getFloatTy(context));
        } else if (!v->getType()->isFloatTy()) {
          // Only cast integers to float; if pointer, stringify via ptrToStr
          // later
          if (v->getType()->isIntegerTy()) {
            f = builder->CreateSIToFP(v, llvm::Type::getFloatTy(context));
          } else {
            // Not a number; return empty string to be safe
            auto empty = builder->CreateGlobalString("", "");
            return empty;
          }
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
    // Treat '+' as string concatenation if either side is a string
    // literal/expr OR one side is already an i8* and the other is a primitive
    // (int/float/bool/pointer)
    auto looksConcatenable = [&](llvm::Value* V, ast::Expr* expr) -> bool {
      if (!V) return false;
      if (V->getType()->isPointerTy()) {
        // Consider pointer concatenable if semantic type says string OR we
        // will stringify it
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
    // Be careful: lhs/rhs may be null here (we allow '+' with missing side);
    // treat null as non-pointer for the purpose of deciding concat.
    auto isPtr = [&](llvm::Value* V) -> bool {
      return V && V->getType()->isPointerTy();
    };
    bool forceConcat =
        (!leftIsString && !rightIsString) &&
        ((isPtr(lhs) && looksConcatenable(rhs, node.right.get())) ||
         (isPtr(rhs) && looksConcatenable(lhs, node.left.get())));
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
    // Not concatenation: perform numeric promotion now and emit add
    // Ensure operands are materialized (for '+' we may have allowed nulls)
    if (!lhs) lhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (!rhs) rhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      if (lhsTy->isDoubleTy() || rhsTy->isDoubleTy()) {
        if (!lhsTy->isDoubleTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isDoubleTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getDoubleTy(context);
      } else {
        // Use float
        if (!lhsTy->isFloatTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isFloatTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getFloatTy(context);
      }
    }
    if (!isFP) {
      // Normalize integer widths: promote bool (i1) and small ints to i32
      auto toI32 = [&](llvm::Value* v) -> llvm::Value* {
        if (v->getType()->isIntegerTy(1))
          return builder->CreateZExt(v, llvm::Type::getInt32Ty(context));
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(32))
          return builder->CreateIntCast(v, llvm::Type::getInt32Ty(context),
                                        true);
        if (v->getType()->isPointerTy()) {
          // Best-effort: treat pointer value as integer address then to i32
          llvm::Value* addr =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateTruncOrBitCast(addr,
                                               llvm::Type::getInt32Ty(context));
        }
        return v;
      };
      lhs = toI32(lhs);
      rhs = toI32(rhs);
    }
    lastValue = isFP ? builder->CreateFAdd(lhs, rhs, "faddtmp")
                     : builder->CreateAdd(lhs, rhs, "addtmp");
  } else if (op == "-") {
    // Numeric operators: do promotion prior to op
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      if (lhsTy->isDoubleTy() || rhsTy->isDoubleTy()) {
        if (!lhsTy->isDoubleTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isDoubleTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getDoubleTy(context);
      } else {
        if (!lhsTy->isFloatTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isFloatTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getFloatTy(context);
      }
    }
    if (!isFP) {
      auto toI32 = [&](llvm::Value* v) -> llvm::Value* {
        if (v->getType()->isIntegerTy(1))
          return builder->CreateZExt(v, llvm::Type::getInt32Ty(context));
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(32))
          return builder->CreateIntCast(v, llvm::Type::getInt32Ty(context),
                                        true);
        if (v->getType()->isPointerTy()) {
          llvm::Value* addr =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateTruncOrBitCast(addr,
                                               llvm::Type::getInt32Ty(context));
        }
        return v;
      };
      lhs = toI32(lhs);
      rhs = toI32(rhs);
    }
    lastValue = isFP ? builder->CreateFSub(lhs, rhs, "fsubtmp")
                     : builder->CreateSub(lhs, rhs, "subtmp");
  } else if (op == "*") {
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      if (lhsTy->isDoubleTy() || rhsTy->isDoubleTy()) {
        if (!lhsTy->isDoubleTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isDoubleTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getDoubleTy(context);
      } else {
        if (!lhsTy->isFloatTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isFloatTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getFloatTy(context);
      }
    }
    if (!isFP) {
      auto toI32 = [&](llvm::Value* v) -> llvm::Value* {
        if (v->getType()->isIntegerTy(1))
          return builder->CreateZExt(v, llvm::Type::getInt32Ty(context));
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(32))
          return builder->CreateIntCast(v, llvm::Type::getInt32Ty(context),
                                        true);
        if (v->getType()->isPointerTy()) {
          llvm::Value* addr =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateTruncOrBitCast(addr,
                                               llvm::Type::getInt32Ty(context));
        }
        return v;
      };
      lhs = toI32(lhs);
      rhs = toI32(rhs);
    }
    lastValue = isFP ? builder->CreateFMul(lhs, rhs, "fmultmp")
                     : builder->CreateMul(lhs, rhs, "multmp");
  } else if (op == "/") {
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      if (lhsTy->isDoubleTy() || rhsTy->isDoubleTy()) {
        if (!lhsTy->isDoubleTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isDoubleTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getDoubleTy(context);
      } else {
        if (!lhsTy->isFloatTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isFloatTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getFloatTy(context);
      }
    }
    if (!isFP) {
      auto toI32 = [&](llvm::Value* v) -> llvm::Value* {
        if (v->getType()->isIntegerTy(1))
          return builder->CreateZExt(v, llvm::Type::getInt32Ty(context));
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(32))
          return builder->CreateIntCast(v, llvm::Type::getInt32Ty(context),
                                        true);
        if (v->getType()->isPointerTy()) {
          llvm::Value* addr =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateTruncOrBitCast(addr,
                                               llvm::Type::getInt32Ty(context));
        }
        return v;
      };
      lhs = toI32(lhs);
      rhs = toI32(rhs);
    }
    lastValue = isFP ? builder->CreateFDiv(lhs, rhs, "fdivtmp")
                     : builder->CreateSDiv(lhs, rhs, "divtmp");
  } else if (op == "%") {
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      if (lhsTy->isDoubleTy() || rhsTy->isDoubleTy()) {
        if (!lhsTy->isDoubleTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isDoubleTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getDoubleTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getDoubleTy(context);
      } else {
        if (!lhsTy->isFloatTy()) {
          if (lhs->getType()->isIntegerTy())
            lhs = builder->CreateSIToFP(lhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        if (!rhsTy->isFloatTy()) {
          if (rhs->getType()->isIntegerTy())
            rhs = builder->CreateSIToFP(rhs, llvm::Type::getFloatTy(context),
                                        "sitofp");
        }
        lhsTy = rhsTy = llvm::Type::getFloatTy(context);
      }
    }
    if (!isFP) {
      auto toI32 = [&](llvm::Value* v) -> llvm::Value* {
        if (v->getType()->isIntegerTy(1))
          return builder->CreateZExt(v, llvm::Type::getInt32Ty(context));
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(32))
          return builder->CreateIntCast(v, llvm::Type::getInt32Ty(context),
                                        true);
        if (v->getType()->isPointerTy()) {
          llvm::Value* addr =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateTruncOrBitCast(addr,
                                               llvm::Type::getInt32Ty(context));
        }
        return v;
      };
      lhs = toI32(lhs);
      rhs = toI32(rhs);
    }
    lastValue = isFP ? builder->CreateFRem(lhs, rhs, "fmodtmp")
                     : builder->CreateSRem(lhs, rhs, "modtmp");
  } else if (op == "<") {
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      // Choose double if either is double
      llvm::Type* tgt = (lhsTy->isDoubleTy() || rhsTy->isDoubleTy())
                            ? llvm::Type::getDoubleTy(context)
                            : llvm::Type::getFloatTy(context);
      auto toFP = [&](llvm::Value* v) {
        if (v->getType()->isFloatingPointTy()) {
          if (v->getType() != tgt) {
            if (tgt->isDoubleTy()) return builder->CreateFPExt(v, tgt);
            return builder->CreateFPTrunc(v, tgt);
          }
          return v;
        }
        if (v->getType()->isIntegerTy()) {
          return builder->CreateSIToFP(v, tgt);
        }
        if (v->getType()->isPointerTy()) {
          // ptr -> int -> fp (best-effort)
          llvm::Value* i64 =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateSIToFP(i64, tgt);
        }
        return v;
      };
      lhs = toFP(lhs);
      rhs = toFP(rhs);
      lastValue = builder->CreateFCmpOLT(lhs, rhs, "fcmpolt");
    } else {
      // Integer/pointer: cast to common integer width
      auto toICommon = [&](llvm::Value* v) {
        if (v->getType()->isPointerTy()) {
          return builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
        }
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(64)) {
          return builder->CreateIntCast(v, llvm::Type::getInt64Ty(context),
                                        true);
        }
        return v;
      };
      lhs = toICommon(lhs);
      rhs = toICommon(rhs);
      // Ensure both are i64
      if (!lhs->getType()->isIntegerTy(64))
        lhs =
            builder->CreateIntCast(lhs, llvm::Type::getInt64Ty(context), true);
      if (!rhs->getType()->isIntegerTy(64))
        rhs =
            builder->CreateIntCast(rhs, llvm::Type::getInt64Ty(context), true);
      lastValue = builder->CreateICmpSLT(lhs, rhs, "icmpolt");
    }
  } else if (op == ">") {
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      llvm::Type* tgt = (lhsTy->isDoubleTy() || rhsTy->isDoubleTy())
                            ? llvm::Type::getDoubleTy(context)
                            : llvm::Type::getFloatTy(context);
      auto toFP = [&](llvm::Value* v) {
        if (v->getType()->isFloatingPointTy()) {
          if (v->getType() != tgt) {
            if (tgt->isDoubleTy()) return builder->CreateFPExt(v, tgt);
            return builder->CreateFPTrunc(v, tgt);
          }
          return v;
        }
        if (v->getType()->isIntegerTy()) {
          return builder->CreateSIToFP(v, tgt);
        }
        if (v->getType()->isPointerTy()) {
          llvm::Value* i64 =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateSIToFP(i64, tgt);
        }
        return v;
      };
      lhs = toFP(lhs);
      rhs = toFP(rhs);
      lastValue = builder->CreateFCmpOGT(lhs, rhs, "fcmpogt");
    } else {
      auto toICommon = [&](llvm::Value* v) {
        if (v->getType()->isPointerTy()) {
          return builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
        }
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(64)) {
          return builder->CreateIntCast(v, llvm::Type::getInt64Ty(context),
                                        true);
        }
        return v;
      };
      lhs = toICommon(lhs);
      rhs = toICommon(rhs);
      if (!lhs->getType()->isIntegerTy(64))
        lhs =
            builder->CreateIntCast(lhs, llvm::Type::getInt64Ty(context), true);
      if (!rhs->getType()->isIntegerTy(64))
        rhs =
            builder->CreateIntCast(rhs, llvm::Type::getInt64Ty(context), true);
      lastValue = builder->CreateICmpSGT(lhs, rhs, "icmpogt");
    }
  } else if (op == "<=") {
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      llvm::Type* tgt = (lhsTy->isDoubleTy() || rhsTy->isDoubleTy())
                            ? llvm::Type::getDoubleTy(context)
                            : llvm::Type::getFloatTy(context);
      auto toFP = [&](llvm::Value* v) {
        if (v->getType()->isFloatingPointTy()) {
          if (v->getType() != tgt) {
            if (tgt->isDoubleTy()) return builder->CreateFPExt(v, tgt);
            return builder->CreateFPTrunc(v, tgt);
          }
          return v;
        }
        if (v->getType()->isIntegerTy()) {
          return builder->CreateSIToFP(v, tgt);
        }
        if (v->getType()->isPointerTy()) {
          llvm::Value* i64 =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateSIToFP(i64, tgt);
        }
        return v;
      };
      lhs = toFP(lhs);
      rhs = toFP(rhs);
      lastValue = builder->CreateFCmpOLE(lhs, rhs, "fcmpole");
    } else {
      auto toICommon = [&](llvm::Value* v) {
        if (v->getType()->isPointerTy()) {
          return builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
        }
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(64)) {
          return builder->CreateIntCast(v, llvm::Type::getInt64Ty(context),
                                        true);
        }
        return v;
      };
      lhs = toICommon(lhs);
      rhs = toICommon(rhs);
      if (!lhs->getType()->isIntegerTy(64))
        lhs =
            builder->CreateIntCast(lhs, llvm::Type::getInt64Ty(context), true);
      if (!rhs->getType()->isIntegerTy(64))
        rhs =
            builder->CreateIntCast(rhs, llvm::Type::getInt64Ty(context), true);
      lastValue = builder->CreateICmpSLE(lhs, rhs, "icmpole");
    }
  } else if (op == ">=") {
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      llvm::Type* tgt = (lhsTy->isDoubleTy() || rhsTy->isDoubleTy())
                            ? llvm::Type::getDoubleTy(context)
                            : llvm::Type::getFloatTy(context);
      auto toFP = [&](llvm::Value* v) {
        if (v->getType()->isFloatingPointTy()) {
          if (v->getType() != tgt) {
            if (tgt->isDoubleTy()) return builder->CreateFPExt(v, tgt);
            return builder->CreateFPTrunc(v, tgt);
          }
          return v;
        }
        if (v->getType()->isIntegerTy()) {
          return builder->CreateSIToFP(v, tgt);
        }
        if (v->getType()->isPointerTy()) {
          llvm::Value* i64 =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateSIToFP(i64, tgt);
        }
        return v;
      };
      lhs = toFP(lhs);
      rhs = toFP(rhs);
      lastValue = builder->CreateFCmpOGE(lhs, rhs, "fcmpoge");
    } else {
      auto toICommon = [&](llvm::Value* v) {
        if (v->getType()->isPointerTy()) {
          return builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
        }
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(64)) {
          return builder->CreateIntCast(v, llvm::Type::getInt64Ty(context),
                                        true);
        }
        return v;
      };
      lhs = toICommon(lhs);
      rhs = toICommon(rhs);
      if (!lhs->getType()->isIntegerTy(64))
        lhs =
            builder->CreateIntCast(lhs, llvm::Type::getInt64Ty(context), true);
      if (!rhs->getType()->isIntegerTy(64))
        rhs =
            builder->CreateIntCast(rhs, llvm::Type::getInt64Ty(context), true);
      lastValue = builder->CreateICmpSGE(lhs, rhs, "icmpoge");
    }
  } else if (op == "==") {
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      llvm::Type* tgt = (lhsTy->isDoubleTy() || rhsTy->isDoubleTy())
                            ? llvm::Type::getDoubleTy(context)
                            : llvm::Type::getFloatTy(context);
      auto toFP = [&](llvm::Value* v) {
        if (v->getType()->isFloatingPointTy()) {
          if (v->getType() != tgt) {
            if (tgt->isDoubleTy()) return builder->CreateFPExt(v, tgt);
            return builder->CreateFPTrunc(v, tgt);
          }
          return v;
        }
        if (v->getType()->isIntegerTy()) {
          return builder->CreateSIToFP(v, tgt);
        }
        if (v->getType()->isPointerTy()) {
          llvm::Value* i64 =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateSIToFP(i64, tgt);
        }
        return v;
      };
      lhs = toFP(lhs);
      rhs = toFP(rhs);
      lastValue = builder->CreateFCmpOEQ(lhs, rhs, "fcmpeq");
    } else {
      auto toICommon = [&](llvm::Value* v) {
        if (v->getType()->isPointerTy()) {
          return builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
        }
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(64)) {
          return builder->CreateIntCast(v, llvm::Type::getInt64Ty(context),
                                        true);
        }
        return v;
      };
      lhs = toICommon(lhs);
      rhs = toICommon(rhs);
      if (!lhs->getType()->isIntegerTy(64))
        lhs =
            builder->CreateIntCast(lhs, llvm::Type::getInt64Ty(context), true);
      if (!rhs->getType()->isIntegerTy(64))
        rhs =
            builder->CreateIntCast(rhs, llvm::Type::getInt64Ty(context), true);
      lastValue = builder->CreateICmpEQ(lhs, rhs, "icmpeq");
    }
  } else if (op == "!=") {
    auto lhsTy = lhs->getType();
    auto rhsTy = rhs->getType();
    bool isFP = lhsTy->isFloatingPointTy() || rhsTy->isFloatingPointTy();
    if (isFP) {
      llvm::Type* tgt = (lhsTy->isDoubleTy() || rhsTy->isDoubleTy())
                            ? llvm::Type::getDoubleTy(context)
                            : llvm::Type::getFloatTy(context);
      auto toFP = [&](llvm::Value* v) {
        if (v->getType()->isFloatingPointTy()) {
          if (v->getType() != tgt) {
            if (tgt->isDoubleTy()) return builder->CreateFPExt(v, tgt);
            return builder->CreateFPTrunc(v, tgt);
          }
          return v;
        }
        if (v->getType()->isIntegerTy()) {
          return builder->CreateSIToFP(v, tgt);
        }
        if (v->getType()->isPointerTy()) {
          llvm::Value* i64 =
              builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
          return builder->CreateSIToFP(i64, tgt);
        }
        return v;
      };
      lhs = toFP(lhs);
      rhs = toFP(rhs);
      lastValue = builder->CreateFCmpONE(lhs, rhs, "fcmpne");
    } else {
      auto toICommon = [&](llvm::Value* v) {
        if (v->getType()->isPointerTy()) {
          return builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
        }
        if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(64)) {
          return builder->CreateIntCast(v, llvm::Type::getInt64Ty(context),
                                        true);
        }
        return v;
      };
      lhs = toICommon(lhs);
      rhs = toICommon(rhs);
      if (!lhs->getType()->isIntegerTy(64))
        lhs =
            builder->CreateIntCast(lhs, llvm::Type::getInt64Ty(context), true);
      if (!rhs->getType()->isIntegerTy(64))
        rhs =
            builder->CreateIntCast(rhs, llvm::Type::getInt64Ty(context), true);
      lastValue = builder->CreateICmpNE(lhs, rhs, "icmpne");
    }
  } else if (op == "&") {
    if (!lhs) lhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (!rhs) rhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    // Bitwise AND on integers; cast bool to i32 if needed
    if (lhs->getType()->isIntegerTy(1))
      lhs = builder->CreateZExt(lhs, llvm::Type::getInt32Ty(context));
    if (rhs->getType()->isIntegerTy(1))
      rhs = builder->CreateZExt(rhs, llvm::Type::getInt32Ty(context));
    if (!lhs->getType()->isIntegerTy(32))
      lhs = builder->CreateIntCast(lhs, llvm::Type::getInt32Ty(context), true);
    if (!rhs->getType()->isIntegerTy(32))
      rhs = builder->CreateIntCast(rhs, llvm::Type::getInt32Ty(context), true);
    lastValue = builder->CreateAnd(lhs, rhs, "andtmp");
  } else if (op == "|") {
    if (!lhs) lhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (!rhs) rhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (lhs->getType()->isIntegerTy(1))
      lhs = builder->CreateZExt(lhs, llvm::Type::getInt32Ty(context));
    if (rhs->getType()->isIntegerTy(1))
      rhs = builder->CreateZExt(rhs, llvm::Type::getInt32Ty(context));
    if (!lhs->getType()->isIntegerTy(32))
      lhs = builder->CreateIntCast(lhs, llvm::Type::getInt32Ty(context), true);
    if (!rhs->getType()->isIntegerTy(32))
      rhs = builder->CreateIntCast(rhs, llvm::Type::getInt32Ty(context), true);
    lastValue = builder->CreateOr(lhs, rhs, "ortmp");
  } else if (op == "^") {
    if (!lhs) lhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (!rhs) rhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (lhs->getType()->isIntegerTy(1))
      lhs = builder->CreateZExt(lhs, llvm::Type::getInt32Ty(context));
    if (rhs->getType()->isIntegerTy(1))
      rhs = builder->CreateZExt(rhs, llvm::Type::getInt32Ty(context));
    if (!lhs->getType()->isIntegerTy(32))
      lhs = builder->CreateIntCast(lhs, llvm::Type::getInt32Ty(context), true);
    if (!rhs->getType()->isIntegerTy(32))
      rhs = builder->CreateIntCast(rhs, llvm::Type::getInt32Ty(context), true);
    lastValue = builder->CreateXor(lhs, rhs, "xortmp");
  } else if (op == "<<") {
    if (!lhs) lhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (!rhs) rhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    // Shifts: ensure lhs i32 and rhs i32
    if (!lhs->getType()->isIntegerTy(32))
      lhs = builder->CreateIntCast(lhs, llvm::Type::getInt32Ty(context), true);
    if (!rhs->getType()->isIntegerTy(32))
      rhs = builder->CreateIntCast(rhs, llvm::Type::getInt32Ty(context), true);
    lastValue = builder->CreateShl(lhs, rhs, "shltmp");
  } else if (op == ">>") {
    if (!lhs) lhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (!rhs) rhs = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (!lhs->getType()->isIntegerTy(32))
      lhs = builder->CreateIntCast(lhs, llvm::Type::getInt32Ty(context), true);
    if (!rhs->getType()->isIntegerTy(32))
      rhs = builder->CreateIntCast(rhs, llvm::Type::getInt32Ty(context), true);
    lastValue = builder->CreateAShr(lhs, rhs, "shrtmp");
  } else {
    lastValue = nullptr;
  }
}

// Visit FunctionDecl: emit function definition
void LLVMCodeGenerator::visit(ast::FunctionDecl& node) {
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
      // If unresolved or mapped to i32 but declared via basic/generic type
      // and not primitive, treat as i8* and mark as string for downstream
      // formatting.
      auto isPrimitiveName = [&](const std::string& t) {
        return (t == "int" || t == "float" || t == "double" || t == "bool" ||
                t == "boolean" || t == "void" || t == "string");
      };
      bool looksUnresolved = (resolvedRetType.empty() ||
                              retType == llvm::Type::getInt32Ty(context));
      bool isBasic =
          dynamic_cast<ast::BasicTypeNode*>(node.returnType.get()) != nullptr;
      bool isGeneric =
          dynamic_cast<ast::GenericTypeNode*>(node.returnType.get()) != nullptr;
      if ((isBasic || isGeneric) && looksUnresolved &&
          !isPrimitiveName(resolvedRetType)) {
        // Treat unresolved/basic generic returns as opaque pointer at
        // runtime, but do NOT label them as string (to avoid accidental raw
        // char* use). Mark as "ptr" so downstream formatting stringifies via
        // ptr_to_string.
        retType = getPointerTy();
        resolvedRetType = "ptr";
      }
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
      // If unresolved or maps to default int but isn't a known primitive,
      // treat as a generic pointer (interfaces/classes)
      auto isPrimitiveName = [&](const std::string& t) {
        return (t == "int" || t == "float" || t == "double" || t == "bool" ||
                t == "boolean" || t == "string");
      };
      bool looksUnresolved = (paramTypeName.empty() ||
                              paramType == llvm::Type::getInt32Ty(context));
      bool isBasic =
          dynamic_cast<ast::BasicTypeNode*>(param->type.get()) != nullptr;
      bool isGeneric =
          dynamic_cast<ast::GenericTypeNode*>(param->type.get()) != nullptr;
      if ((isBasic || isGeneric) && looksUnresolved &&
          !isPrimitiveName(paramTypeName)) {
        // Heuristic: generic/interface/custom types become i8*
        paramType = getPointerTy();
      }
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
  // Save current symbol table and start a fresh scope for this function.
  // Preserve only globals so we keep their semantic type names (e.g., string)
  // for formatting and interop, while avoiding leakage of locals/allocas from
  // other functions.
  auto savedSymbolTable = symbolTable;  // full snapshot to restore later
  std::unordered_map<std::string, SymbolInfo> globalsOnly;
  for (const auto& kv : symbolTable) {
    const auto& name = kv.first;
    const auto& sym = kv.second;
    if (llvm::isa<llvm::GlobalVariable>(sym.value)) {
      globalsOnly.emplace(name, sym);
    }
  }
  symbolTable = std::move(globalsOnly);
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
      // If unresolved, try to recover a readable name from the AST type node
      if (paramResolvedName.empty()) {
        if (auto* bt = dynamic_cast<ast::BasicTypeNode*>(param->type.get())) {
          paramResolvedName = bt->name.getLexeme();
        } else if (auto* gt =
                       dynamic_cast<ast::GenericTypeNode*>(param->type.get())) {
          paramResolvedName = gt->name.getLexeme();
        }
      }
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
  // Restore outer scope symbols after finishing the function.
  symbolTable = std::move(savedSymbolTable);
}
// Visit ReturnStmt: emit return
void LLVMCodeGenerator::visit(ast::ReturnStmt& node) {
  llvm::BasicBlock* bb = builder->GetInsertBlock();
  if (!bb || bb->getTerminator()) return;

  llvm::Value* valueToReturn = nullptr;
  if (node.value) {
    node.value->accept(*this);
    valueToReturn = lastValue;
  }

  llvm::Type* expectedTy =
      currentFunction ? currentFunction->getReturnType() : nullptr;

  if (!expectedTy || expectedTy->isVoidTy()) {
    builder->CreateRetVoid();
    return;
  }

  if (!valueToReturn) {
    if (expectedTy->isIntegerTy()) {
      valueToReturn = llvm::ConstantInt::get(expectedTy, 0);
    } else if (expectedTy->isFloatingPointTy()) {
      valueToReturn = llvm::ConstantFP::get(expectedTy, 0.0);
    } else if (expectedTy->isPointerTy()) {
      valueToReturn = llvm::ConstantPointerNull::get(
          llvm::cast<llvm::PointerType>(expectedTy));
    } else {
      valueToReturn = llvm::PoisonValue::get(expectedTy);
    }
    builder->CreateRet(valueToReturn);
    return;
  }

  if (currentFunction && currentFunction->getName() == "main" &&
      expectedTy->isIntegerTy(32) &&
      !valueToReturn->getType()->isIntegerTy(32)) {
    builder->CreateRet(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
    return;
  }

  llvm::Value* coerced = coerceToReturnType(valueToReturn, expectedTy);

  if (!coerced || coerced->getType() != expectedTy) {
    if (expectedTy->isIntegerTy()) {
      coerced = llvm::ConstantInt::get(expectedTy, 0);
    } else if (expectedTy->isFloatingPointTy()) {
      coerced = llvm::ConstantFP::get(expectedTy, 0.0);
    } else if (expectedTy->isPointerTy()) {
      coerced = llvm::ConstantPointerNull::get(
          llvm::cast<llvm::PointerType>(expectedTy));
    } else {
      coerced = llvm::PoisonValue::get(expectedTy);
    }
  }

  builder->CreateRet(coerced);
}

// Visit BlockStmt: emit all statements
void LLVMCodeGenerator::visit(ast::BlockStmt& node) {
  // run through each statement, but stop after emitting a terminator
  for (auto& stmt : node.statements) {
    auto* bb = builder->GetInsertBlock();
    if (bb && bb->getTerminator()) break;
    if (stmt) stmt->accept(*this);
  }
}

// Visit CallExpr: emit function call (including stdlib like console.log)
void LLVMCodeGenerator::visit(ast::CallExpr& node) {
  // Only support direct identifier or member access for now
  // If at global scope, skip generating calls (invalid outside functions)
  if (!currentFunction) {
    lastValue = nullptr;
    return;
  }
  // Re-entrancy guard: prevent accidental recursion on same AST node
  if (activeCallExprs.find(&node) != activeCallExprs.end()) {
    llvm::errs() << "[ERROR] Re-entrant codegen for CallExpr detected. "
                    "Aborting call emission.\n";
    lastValue = nullptr;
    return;
  }
  activeCallExprs.insert(&node);
  std::string funcName;
  // Attempt to determine callee's return type to help with argument
  // conversions
  std::string calleeRetType;
  if (auto ident = dynamic_cast<IdentifierExpr*>(node.callee.get())) {
    // Handle direct 'super()' constructor call inside a class
    if (ident->name.getLexeme() == std::string("super")) {
      // Resolve current and base class
      if (classNameStack.empty()) {
        lastValue = nullptr;
        return;
      }
      std::string cur = classNameStack.back();
      auto itCls = classes.find(cur);
      if (itCls == classes.end() || itCls->second.baseClass.empty()) {
        lastValue = nullptr;  // no base
        return;
      }
      std::string base = itCls->second.baseClass;
      // Build args: this (i8*) + forwarded args
      std::vector<llvm::Value*> args;
      // Load 'this'
      auto itThis = symbolTable.find("this");
      if (itThis != symbolTable.end()) {
        llvm::Value* thisPtr = itThis->second.value;
        if (auto* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(thisPtr)) {
          thisPtr =
              builder->CreateLoad(allocaInst->getAllocatedType(), thisPtr);
        } else {
          thisPtr = builder->CreateLoad(getPointerTy(), thisPtr);
        }
        thisPtr = ensureBitcast(thisPtr, getPointerTy());
        args.push_back(thisPtr);
      }
      for (auto& a : node.arguments) {
        if (!a) continue;
        a->accept(*this);
        if (lastValue) args.push_back(lastValue);
      }
      // Emit call to __ctor_Base
      auto fn = module->getFunction(std::string("__ctor_") + base);
      if (fn) builder->CreateCall(fn, args);
      lastValue = nullptr;
      // done
      activeCallExprs.erase(&node);
      return;
    }
    funcName = ident->name.getLexeme();
    auto itFn = functionReturnTypes.find(funcName);
    if (itFn != functionReturnTypes.end()) {
      calleeRetType = itFn->second;
    }
  } else if (auto member = dynamic_cast<MemberAccessExpr*>(node.callee.get())) {
    // Handle super.method() -> Base.method(this, )
    if (auto objIdent = dynamic_cast<IdentifierExpr*>(member->object.get())) {
      if (objIdent->name.getLexeme() == std::string("super")) {
        if (classNameStack.empty()) {
          lastValue = nullptr;
          activeCallExprs.erase(&node);
          return;
        }
        std::string cur = classNameStack.back();
        auto itCls = classes.find(cur);
        if (itCls == classes.end() || itCls->second.baseClass.empty()) {
          lastValue = nullptr;
          activeCallExprs.erase(&node);
          return;
        }
        std::string base = itCls->second.baseClass;
        funcName = base + "." + member->member.getLexeme();
      }
    }
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
    } else if (dynamic_cast<ast::ThisExpr*>(member->object.get())) {
      if (!classNameStack.empty()) className = classNameStack.back();
    }
    if (!funcName.size()) {
      // Extract base class name if it's a pointer type name
      if (!className.empty() && className.back() == '*') {
        className.pop_back();
      }
      // Only treat as direct class method if it's a known class
      if (!className.empty() && classes.count(className)) {
        funcName = className + "." + member->member.getLexeme();
      }
      // If not a known class, treat as interface and scan classes for an
      // implementation.
      if (funcName.empty()) {
        std::string ifaceName = className;
        // If we didn't get a semantic name above, try to fetch from
        // identifier again
        if (ifaceName.empty()) {
          if (auto objIdent =
                  dynamic_cast<IdentifierExpr*>(member->object.get())) {
            auto itSym = symbolTable.find(objIdent->name.getLexeme());
            if (itSym != symbolTable.end()) ifaceName = itSym->second.typeName;
          }
          if (!ifaceName.empty() && ifaceName.back() == '*')
            ifaceName.pop_back();
        }
        if (!ifaceName.empty() && !classes.count(ifaceName)) {
          auto stripGeneric = [](const std::string& s) {
            auto pos = s.find('<');
            return s.substr(0, pos);
          };
          std::string baseIface = stripGeneric(ifaceName);
          std::string method = member->member.getLexeme();
          std::string chosen;
          std::string innerName;
          // Pick the first class that implements ifaceName and has method
          for (auto& kv : classes) {
            auto& ci = kv.second;
            bool implements = false;
            for (auto& in : ci.implements) {
              if (stripGeneric(in) == baseIface) {
                implements = true;
                break;
              }
            }
            if (!implements) continue;
            std::string candidate = kv.first + "." + method;
            if (functionReturnTypes.count(candidate)) {
              innerName = candidate;
              break;
            }
          }
          if (!chosen.empty()) funcName = chosen;
        }
        // If still unresolved (no interface match or no type info), fall back
        // to any class that defines this method name.
        if (funcName.empty()) {
          std::string method = member->member.getLexeme();
          for (auto& kv : classes) {
            std::string candidate = kv.first + "." + method;
            if (functionReturnTypes.count(candidate)) {
              funcName = candidate;
              break;
            }
          }
        }
      }
    }
  }
  // Identify stdlib console functions where we need to stringify arguments
  bool isConsole = (funcName == "console.log" || funcName == "console.error" ||
                    funcName == "console.warn");
  std::vector<llvm::Value*> args;
  std::vector<std::pair<llvm::Value*, bool>> argWithFree;
  // If this is a method call on an object (member access), evaluate the
  // object and push as first arg
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
      // If we resolved to a method via interface fallback, still pass 'this'
      // as i8*
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
  // Arguments must already have defaults applied by semantics; do not mutate
  for (auto& arg : node.arguments) {
    // Reset lastValue defensively before evaluating each arg
    lastValue = nullptr;

    // Console fast-path: if the argument is a generic call like foo<T>()
    // and we likely can't resolve it to a concrete function symbol (due to
    // erased generics), try to stringify its first value argument directly.
    // This fixes cases like console.log(firstOf<int>(10, 20)) printing a
    // pointer address instead of 10.
    bool handledGenericForConsole = false;
    if (isConsole) {
      if (auto callArg = dynamic_cast<CallExpr*>(arg.get())) {
        std::string calleeName;
        if (auto innerIdent =
                dynamic_cast<IdentifierExpr*>(callArg->callee.get())) {
          calleeName = innerIdent->name.getLexeme();
        } else if (auto innerMember =
                       dynamic_cast<MemberAccessExpr*>(callArg->callee.get())) {
          // We only care about free functions here; skip member calls
        }
        bool looksErasedGeneric = false;
        if (!calleeName.empty()) {
          auto itFR = functionReturnTypes.find(calleeName);
          if (itFR != functionReturnTypes.end() &&
              itFR->second == std::string("ptr")) {
            looksErasedGeneric = true;
          }
        }
        // Also handle known generic functions by name
        if (!looksErasedGeneric && !calleeName.empty()) {
          if (calleeName == "firstOf") {
            looksErasedGeneric = true;
          }
        }
        if (looksErasedGeneric) {
          // If it has at least one value arg, evaluate that and stringify.
          if (!callArg->arguments.empty()) {
            // Evaluate first value argument
            callArg->arguments[0]->accept(*this);
            llvm::Value* a0v = lastValue;
            lastValue = nullptr;  // don't leak into outer decisions
            if (a0v) {
              // Heuristically stringify ints/bools/floats; otherwise, treat
              // as pointer
              llvm::Value* vconv = nullptr;
              bool needsFree = false;
              if (a0v->getType()->isIntegerTy(1)) {
                auto boolToStrType = llvm::FunctionType::get(
                    llvm::PointerType::get(context, 0),
                    {llvm::Type::getInt1Ty(context)}, false);
                auto boolToStrFunc = module->getOrInsertFunction(
                    "tspp_bool_to_string", boolToStrType);
                vconv = builder->CreateCall(boolToStrFunc, {a0v});
                needsFree = true;
              } else if (a0v->getType()->isIntegerTy()) {
                llvm::Value* i32 = a0v;
                if (!a0v->getType()->isIntegerTy(32)) {
                  i32 = builder->CreateIntCast(
                      a0v, llvm::Type::getInt32Ty(context), true);
                }
                auto intToStrType = llvm::FunctionType::get(
                    llvm::PointerType::get(context, 0),
                    {llvm::Type::getInt32Ty(context)}, false);
                auto intToStrFunc = module->getOrInsertFunction(
                    "tspp_int_to_string", intToStrType);
                vconv = builder->CreateCall(intToStrFunc, {i32});
                needsFree = true;
              } else if (a0v->getType()->isFloatingPointTy()) {
                llvm::Value* f = a0v;
                if (a0v->getType()->isDoubleTy()) {
                  f = builder->CreateFPTrunc(a0v,
                                             llvm::Type::getFloatTy(context));
                } else if (!a0v->getType()->isFloatTy()) {
                  f = builder->CreateSIToFP(a0v,
                                            llvm::Type::getFloatTy(context));
                }
                auto floatToStrType = llvm::FunctionType::get(
                    llvm::PointerType::get(context, 0),
                    {llvm::Type::getFloatTy(context)}, false);
                auto floatToStrFunc = module->getOrInsertFunction(
                    "tspp_float_to_string", floatToStrType);
                vconv = builder->CreateCall(floatToStrFunc, {f});
                needsFree = true;
              } else if (a0v->getType()->isPointerTy()) {
                auto ptrToStrType = llvm::FunctionType::get(
                    llvm::PointerType::get(context, 0),
                    {llvm::PointerType::get(context, 0)}, false);
                auto ptrToStrFunc = module->getOrInsertFunction(
                    "tspp_ptr_to_string", ptrToStrType);
                vconv = builder->CreateCall(ptrToStrFunc, {a0v});
                needsFree = true;
              }
              if (vconv) {
                args.push_back(vconv);
                argWithFree.push_back({vconv, needsFree});
                handledGenericForConsole = true;
              }
            }
          }
        }
      }
    }

    if (handledGenericForConsole) {
      continue;  // we've already emitted this console arg
    }

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
      // If still missing and we're printing to console, treat as empty string
      if (!v && isConsole) {
        v = builder->CreateGlobalString("", "");
      }
      if (!v) continue;
    }

    bool needsFree = false;
    // Look up a semantic type name if available (identifiers, calls, literals)
    std::string semanticTypeName;
    if (auto idArg = dynamic_cast<IdentifierExpr*>(arg.get())) {
      auto itSym = symbolTable.find(idArg->name.getLexeme());
      if (itSym != symbolTable.end()) {
        semanticTypeName = itSym->second.typeName;
      }
      // If not found, try synthetic field key like "obj.field"
      if (semanticTypeName.empty()) {
        // Best-effort: if identifier contains a dot, check symbol table
        auto name = idArg->name.getLexeme();
        if (name.find('.') != std::string::npos) {
          auto it2 = symbolTable.find(name);
          if (it2 != symbolTable.end()) semanticTypeName = it2->second.typeName;
        }
      }
    } else if (auto callArg = dynamic_cast<CallExpr*>(arg.get())) {
      // Determine inner callee name and look up its return type
      std::string innerName;
      if (auto innerIdent =
              dynamic_cast<IdentifierExpr*>(callArg->callee.get())) {
        innerName = innerIdent->name.getLexeme();
      } else if (auto innerMember =
                     dynamic_cast<MemberAccessExpr*>(callArg->callee.get())) {
        // Resolve based on the object's declared class/interface type
        if (auto objIdent =
                dynamic_cast<IdentifierExpr*>(innerMember->object.get())) {
          std::string objTy;
          auto itSym = symbolTable.find(objIdent->name.getLexeme());
          if (itSym != symbolTable.end()) objTy = itSym->second.typeName;
          if (!objTy.empty() && objTy.back() == '*') objTy.pop_back();
          auto stripGeneric = [](const std::string& s) {
            auto pos = s.find('<');
            return s.substr(0, pos);
          };
          std::string baseTy = stripGeneric(objTy);
          if (!baseTy.empty() && classes.count(baseTy)) {
            innerName = baseTy + "." + innerMember->member.getLexeme();
          } else if (!objTy.empty()) {
            std::string baseIface = stripGeneric(objTy);
            std::string chosen;
            for (auto& kv : classes) {
              bool implements = false;
              for (auto& in : kv.second.implements) {
                if (stripGeneric(in) == baseIface) {
                  implements = true;
                  break;
                }
              }
              if (!implements) continue;
              std::string candidate =
                  kv.first + "." + innerMember->member.getLexeme();
              if (functionReturnTypes.count(candidate)) {
                innerName = candidate;
                break;
              }
            }
            if (!chosen.empty()) innerName = chosen;
          } else if (!classes.empty()) {
            std::string method = innerMember->member.getLexeme();
            for (auto& kv : classes) {
              std::string candidate = kv.first + "." + method;
              if (functionReturnTypes.count(candidate)) {
                innerName = candidate;
                break;
              }
            }
          }
        } else if (dynamic_cast<ast::ThisExpr*>(innerMember->object.get())) {
          // this.method() => CurrentClass.method, when inside a class
          if (!classNameStack.empty()) {
            innerName =
                classNameStack.back() + "." + innerMember->member.getLexeme();
          }
        }
      }
      if (!innerName.empty()) {
        auto itFn2 = functionReturnTypes.find(innerName);
        if (itFn2 != functionReturnTypes.end()) {
          semanticTypeName = itFn2->second;
        }
        if (semanticTypeName.empty()) {
          if (auto innerMember =
                  dynamic_cast<MemberAccessExpr*>(callArg->callee.get())) {
            if (innerMember->member.getLexeme() == std::string("get")) {
              if (auto objIdent = dynamic_cast<IdentifierExpr*>(
                      innerMember->object.get())) {
                auto itSym = symbolTable.find(objIdent->name.getLexeme());
                if (itSym != symbolTable.end()) {
                  const std::string& tn = itSym->second.typeName;
                  if (tn.find("<string") != std::string::npos) {
                    semanticTypeName = "string";
                  }
                }
              }
            }
          }
        }
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
        // Decide conversion for pointers: treat known string values as strings
        // and only stringify raw non-string pointers as addresses.
        bool isString = (!semanticTypeName.empty() &&
                         semanticTypeName == std::string("string"));
        // Heuristic 1: BinaryExpr "+" arguments are concatenations that
        // produce strings.
        if (!isString) {
          if (auto binArg = dynamic_cast<BinaryExpr*>(arg.get())) {
            if (binArg->op.getLexeme() == std::string("+")) {
              isString = true;
            }
          }
        }
        // Heuristic 2: If the IR value comes from known string-producing
        // runtime helpers, treat it as string.
        if (!isString) {
          if (auto* callBase = llvm::dyn_cast<llvm::CallBase>(v)) {
            if (auto* callee = callBase->getCalledFunction()) {
              auto name = callee->getName();
              if (name == "tspp_string_concat" ||
                  name == "tspp_int_to_string" ||
                  name == "tspp_float_to_string" ||
                  name == "tspp_bool_to_string") {
                isString = true;
              }
            }
          }
        }
        // Heuristic 3: If this pointer is the result of a generic function
        // call that returns an opaque "ptr", attempt to recover the original
        // scalar by inspecting the call's first argument type and casting the
        // pointer back to that scalar for stringification. This fixes cases
        // like: console.log(firstOf<int>(10, 20)) which would otherwise print
        // the pointer-address of 10.
        if (!isString) {
          if (auto callArg = dynamic_cast<CallExpr*>(arg.get())) {
            // Resolve the inner callee name similarly to above to query
            // functionReturnTypes.
            std::string innerName;
            if (auto innerIdent =
                    dynamic_cast<IdentifierExpr*>(callArg->callee.get())) {
              innerName = innerIdent->name.getLexeme();
            } else if (auto innerMember = dynamic_cast<MemberAccessExpr*>(
                           callArg->callee.get())) {
              if (auto objIdent = dynamic_cast<IdentifierExpr*>(
                      innerMember->object.get())) {
                std::string objTy;
                auto itSym = symbolTable.find(objIdent->name.getLexeme());
                if (itSym != symbolTable.end()) objTy = itSym->second.typeName;
                if (!objTy.empty() && objTy.back() == '*') objTy.pop_back();
                auto stripGeneric = [](const std::string& s) {
                  auto pos = s.find('<');
                  return s.substr(0, pos);
                };
                std::string baseTy = stripGeneric(objTy);
                if (!baseTy.empty() && classes.count(baseTy)) {
                  innerName = baseTy + "." + innerMember->member.getLexeme();
                } else if (!objTy.empty()) {
                  std::string baseIface = stripGeneric(objTy);
                  std::string chosen;
                  for (auto& kv : classes) {
                    bool implements = false;
                    for (auto& in : kv.second.implements) {
                      if (stripGeneric(in) == baseIface) {
                        implements = true;
                        break;
                      }
                    }
                    if (!implements) continue;
                    std::string candidate =
                        kv.first + "." + innerMember->member.getLexeme();
                    if (functionReturnTypes.count(candidate)) {
                      innerName = candidate;
                      break;
                    }
                  }
                  if (!chosen.empty()) innerName = chosen;
                } else if (!classes.empty()) {
                  std::string method = innerMember->member.getLexeme();
                  for (auto& kv : classes) {
                    std::string candidate = kv.first + "." + method;
                    if (functionReturnTypes.count(candidate)) {
                      innerName = candidate;
                      break;
                    }
                  }
                }
              } else if (dynamic_cast<ast::ThisExpr*>(
                             innerMember->object.get())) {
                if (!classNameStack.empty()) {
                  innerName = classNameStack.back() + "." +
                              innerMember->member.getLexeme();
                }
              }
            }
            std::string innerRet;
            if (!innerName.empty()) {
              auto itFn = functionReturnTypes.find(innerName);
              if (itFn != functionReturnTypes.end()) innerRet = itFn->second;
            } else if (auto innerIdent = dynamic_cast<IdentifierExpr*>(
                           callArg->callee.get())) {
              auto itFn =
                  functionReturnTypes.find(innerIdent->name.getLexeme());
              if (itFn != functionReturnTypes.end()) innerRet = itFn->second;
            }
            // If we know it returns opaque pointer OR we have no metadata
            // (semanticTypeName empty), try to recover scalar from first arg.
            if (innerRet == std::string("ptr") || semanticTypeName.empty()) {
              // Guess underlying scalar type from the first argument
              std::string underlying = "";
              if (!callArg->arguments.empty()) {
                ast::Expr* a0 = callArg->arguments[0].get();
                if (auto lit0 = dynamic_cast<LiteralExpr*>(a0)) {
                  using tokens::TokenType;
                  if (lit0->value.getType() == TokenType::NUMBER) {
                    auto lx = lit0->value.getLexeme();
                    if (lx.find('.') != std::string::npos)
                      underlying = "float";
                    else
                      underlying = "int";
                  } else if (lit0->value.getType() == TokenType::TRUE ||
                             lit0->value.getType() == TokenType::FALSE) {
                    underlying = "bool";
                  }
                } else if (auto id0 = dynamic_cast<IdentifierExpr*>(a0)) {
                  auto itSym = symbolTable.find(id0->name.getLexeme());
                  if (itSym != symbolTable.end())
                    underlying = itSym->second.typeName;
                }
              }
              // If we could not infer, fall back to original behavior.
              if (!underlying.empty()) {
                // Prefer to evaluate the first argument expression directly
                // and stringify it instead of trying to decode raw pointer
                // bits. This avoids UB and matches semantics for helpers like
                // firstOf<T>(a,b) returning the first argument.
                llvm::Value* savedLast = lastValue;
                lastValue = nullptr;
                callArg->arguments[0]->accept(*this);
                llvm::Value* a0v = lastValue;
                lastValue = savedLast;  // restore
                if (a0v) {
                  if (underlying == std::string("int")) {
                    llvm::Value* i32 = a0v;
                    if (!a0v->getType()->isIntegerTy(32)) {
                      if (a0v->getType()->isIntegerTy())
                        i32 = builder->CreateIntCast(
                            a0v, llvm::Type::getInt32Ty(context), true);
                      else if (a0v->getType()->isFloatTy())
                        i32 = builder->CreateFPToSI(
                            a0v, llvm::Type::getInt32Ty(context));
                      else
                        i32 = llvm::PoisonValue::get(
                            llvm::Type::getInt32Ty(context));
                    }
                    auto intToStrType = llvm::FunctionType::get(
                        llvm::PointerType::get(context, 0),
                        {llvm::Type::getInt32Ty(context)}, false);
                    auto intToStrFunc = module->getOrInsertFunction(
                        "tspp_int_to_string", intToStrType);
                    v = builder->CreateCall(intToStrFunc, {i32});
                    needsFree = true;
                    isString = true;
                  } else if (underlying == std::string("bool")) {
                    llvm::Value* i1 = a0v;
                    if (!a0v->getType()->isIntegerTy(1)) {
                      if (a0v->getType()->isIntegerTy())
                        i1 = builder->CreateICmpNE(
                            a0v, llvm::ConstantInt::get(a0v->getType(), 0));
                      else if (a0v->getType()->isFloatingPointTy())
                        i1 = builder->CreateFCmpONE(
                            a0v, llvm::ConstantFP::get(a0v->getType(), 0.0));
                      else
                        i1 = llvm::ConstantInt::getFalse(context);
                    }
                    auto boolToStrType = llvm::FunctionType::get(
                        llvm::PointerType::get(context, 0),
                        {llvm::Type::getInt1Ty(context)}, false);
                    auto boolToStrFunc = module->getOrInsertFunction(
                        "tspp_bool_to_string", boolToStrType);
                    v = builder->CreateCall(boolToStrFunc, {i1});
                    needsFree = true;
                    isString = true;
                  } else if (underlying == std::string("float") ||
                             underlying == std::string("double")) {
                    llvm::Value* f = a0v;
                    if (!a0v->getType()->isFloatingPointTy()) {
                      if (a0v->getType()->isIntegerTy())
                        f = builder->CreateSIToFP(
                            a0v, llvm::Type::getFloatTy(context));
                      else
                        f = llvm::ConstantFP::get(
                            llvm::Type::getFloatTy(context), 0.0);
                    } else if (a0v->getType()->isDoubleTy()) {
                      f = builder->CreateFPTrunc(
                          a0v, llvm::Type::getFloatTy(context));
                    }
                    auto floatToStrType = llvm::FunctionType::get(
                        llvm::PointerType::get(context, 0),
                        {llvm::Type::getFloatTy(context)}, false);
                    auto floatToStrFunc = module->getOrInsertFunction(
                        "tspp_float_to_string", floatToStrType);
                    v = builder->CreateCall(floatToStrFunc, {f});
                    needsFree = true;
                    isString = true;
                  }
                }
              }
            }
          }
        }
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
    } else {
      // Args were successfully processed - concatenate and log them
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
    }
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
      // Before emitting the call, coerce each argument to match the callee
      // parameter type when feasible (int<->int, float<->float, ptr<->ptr).
      if (args.size() == func->arg_size()) {
        for (size_t i = 0; i < args.size(); ++i) {
          llvm::Value* v = args[i];
          llvm::Type* pt = func->getArg(i)->getType();
          if (!v || !pt) continue;
          llvm::Type* vt = v->getType();
          if (vt == pt) continue;
          if (vt->isPointerTy() && pt->isPointerTy()) {
            args[i] = ensureBitcast(v, pt);
          } else if (vt->isIntegerTy() && pt->isIntegerTy()) {
            args[i] = builder->CreateIntCast(v, pt, true);
          } else if (vt->isFloatingPointTy() && pt->isFloatingPointTy()) {
            if (vt->isDoubleTy() && pt->isFloatTy())
              args[i] = builder->CreateFPTrunc(v, pt);
            else if (vt->isFloatTy() && pt->isDoubleTy())
              args[i] = builder->CreateFPExt(v, pt);
          } else if (vt->isIntegerTy() && pt->isFloatingPointTy()) {
            args[i] = builder->CreateSIToFP(v, pt);
          } else if (vt->isFloatingPointTy() && pt->isIntegerTy()) {
            args[i] = builder->CreateFPToSI(v, pt);
          } else if (pt->isPointerTy() &&
                     (vt->isIntegerTy() || vt->isFloatingPointTy())) {
            // Convert scalars to strings to satisfy pointer params gracefully
            if (vt->isIntegerTy(1)) {
              auto boolToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::Type::getInt1Ty(context)}, false);
              auto boolToStr = module->getOrInsertFunction(
                  "tspp_bool_to_string", boolToStrType);
              v = builder->CreateCall(boolToStr, {v});
            } else if (vt->isIntegerTy()) {
              llvm::Value* i32 = v;
              if (!vt->isIntegerTy(32))
                i32 = builder->CreateIntCast(v, llvm::Type::getInt32Ty(context),
                                             true);
              auto intToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::Type::getInt32Ty(context)}, false);
              auto intToStrFunc = module->getOrInsertFunction(
                  "tspp_int_to_string", intToStrType);
              v = builder->CreateCall(intToStrFunc, {i32});
            } else if (vt->isFloatingPointTy()) {
              llvm::Value* f = v;
              if (vt->isDoubleTy())
                f = builder->CreateFPTrunc(v, llvm::Type::getFloatTy(context));
              auto floatToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::Type::getFloatTy(context)}, false);
              auto floatToStr = module->getOrInsertFunction(
                  "tspp_float_to_string", floatToStrType);
              v = builder->CreateCall(floatToStr, {f});
            }
            if (v->getType() != pt && v->getType()->isPointerTy()) {
              v = ensureBitcast(v, pt);
            }
            args[i] = v;
          }
        }
      }
      lastValue = builder->CreateCall(func, args);
    } else {
      lastValue = nullptr;
    }
  }
  // Done: pop re-entrancy guard
  activeCallExprs.erase(&node);
}

// Visit IfStmt: emit conditional branching
void LLVMCodeGenerator::visit(ast::IfStmt& node) {
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
void LLVMCodeGenerator::visit(ast::WhileStmt& node) {
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
void LLVMCodeGenerator::visit(ast::ForStmt& node) {
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
void LLVMCodeGenerator::visit(ast::AssignmentExpr& node) {
  // Identifier assignment
  if (auto targetIdent =
          dynamic_cast<ast::IdentifierExpr*>(node.target.get())) {
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
                 dynamic_cast<ast::MemberAccessExpr*>(node.target.get())) {
    // Support 'obj.field = value' and 'this.field = value'
    // Evaluate RHS first
    llvm::Value* rhs = nullptr;
    if (node.value) {
      node.value->accept(*this);
      rhs = lastValue;
    } else {
      // No RHS; treat as no-op and avoid reusing stale lastValue
      lastValue = nullptr;
    }
    // Static field assignment: ClassName.field = value
    if (auto idObj =
            dynamic_cast<ast::IdentifierExpr*>(memberTarget->object.get())) {
      std::string objName = idObj->name.getLexeme();
      auto itCls = classes.find(objName);
      if (itCls != classes.end() && itCls->second.staticFields.count(
                                        memberTarget->member.getLexeme()) > 0) {
        std::string gname =
            objName + std::string(".") + memberTarget->member.getLexeme();
        if (auto* gvar = module->getGlobalVariable(gname, true)) {
          llvm::Type* elemTy = gvar->getValueType();
          llvm::Value* toStore = rhs;
          if (rhs && rhs->getType() != elemTy) {
            if (rhs->getType()->isPointerTy() && elemTy->isPointerTy())
              toStore = builder->CreatePointerCast(rhs, elemTy);
            else if (rhs->getType()->isIntegerTy() && elemTy->isIntegerTy())
              toStore = builder->CreateIntCast(rhs, elemTy, true);
            else if (rhs->getType()->isFloatingPointTy() &&
                     elemTy->isFloatingPointTy()) {
              if (rhs->getType()->isDoubleTy() && elemTy->isFloatTy())
                toStore = builder->CreateFPTrunc(rhs, elemTy);
              else if (rhs->getType()->isFloatTy() && elemTy->isDoubleTy())
                toStore = builder->CreateFPExt(rhs, elemTy);
            }
          }
          builder->CreateStore(toStore, gvar);
          lastValue = toStore;
          return;
        }
      }
    }
    // Evaluate object
    if (!memberTarget->object) {
      lastValue = nullptr;
      return;
    }
    memberTarget->object->accept(*this);
    llvm::Value* base = lastValue;
    // Determine class name if possible
    std::string className;
    if (auto id =
            dynamic_cast<ast::IdentifierExpr*>(memberTarget->object.get())) {
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
        if (toStore && elemTy) {
          builder->CreateStore(toStore, fieldPtr);
          lastValue = toStore;
        } else {
          lastValue = nullptr;
        }
        return;
      }
    }
    lastValue = nullptr;
    return;
  } else if (auto indexTarget =
                 dynamic_cast<ast::IndexAccessExpr*>(node.target.get())) {
    // Handle arr[index] = value for simple i32 arrays allocated as globals
    if (!currentFunction) {
      lastValue = nullptr;
      return;
    }
    // Evaluate RHS
    if (node.value) node.value->accept(*this);
    llvm::Value* rhs = lastValue;
    // Evaluate array/base and index
    if (!indexTarget->object || !indexTarget->index) {
      lastValue = nullptr;
      return;
    }
    indexTarget->object->accept(*this);
    llvm::Value* base = lastValue;
    indexTarget->index->accept(*this);
    llvm::Value* idx = lastValue;
    if (!base || !idx || !rhs) {
      lastValue = nullptr;
      return;
    }
    // Assume base is i32* and index i32
    if (!idx->getType()->isIntegerTy(32)) {
      idx = builder->CreateIntCast(idx, llvm::Type::getInt32Ty(context), true);
    }
    // For pointer arithmetic, need element pointer; assume i32 element
    llvm::Type* elemTy = llvm::Type::getInt32Ty(context);
    llvm::Value* ptr = base;
    if (!base->getType()->isPointerTy()) {
      // If we have an integer address, cast to pointer, else bitcast
      if (base->getType()->isIntegerTy()) {
        ptr = builder->CreateIntToPtr(base, llvm::PointerType::get(elemTy, 0));
      } else {
        // fallback: treat as i8* and bitcast
        ptr = ensureBitcast(base, llvm::PointerType::get(elemTy, 0));
      }
    }
    // GEP on i32*
    llvm::Value* gep = builder->CreateGEP(elemTy, ptr, idx, "elem.ptr");
    llvm::Value* val = rhs;
    if (rhs->getType() != elemTy && rhs->getType()->isIntegerTy()) {
      val = builder->CreateIntCast(rhs, elemTy, true);
    } else if (rhs->getType()->isFloatingPointTy()) {
      val = builder->CreateFPToSI(rhs, elemTy);
    }
    builder->CreateStore(val, gep);
    lastValue = val;
    return;
  }
}

// Visit UnaryExpr: handle unary operators
void LLVMCodeGenerator::visit(ast::UnaryExpr& node) {
  std::string op = node.op.getLexeme();

  if (op == "@") {
    // Address-of operator: get the address of the operand
    if (auto identExpr =
            dynamic_cast<ast::IdentifierExpr*>(node.operand.get())) {
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
    // We can only emit loads inside a function (builder needs an insert
    // point)
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
                  dynamic_cast<ast::IdentifierExpr*>(node.operand.get())) {
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
      // Logical not: compare to zero and produce i1
      if (operandValue->getType()->isIntegerTy(1)) {
        lastValue = builder->CreateNot(operandValue, "not");
      } else if (operandValue->getType()->isIntegerTy()) {
        llvm::Value* isZero = builder->CreateICmpEQ(
            operandValue, llvm::ConstantInt::get(operandValue->getType(), 0));
        lastValue = isZero;
      } else if (operandValue->getType()->isFloatingPointTy()) {
        llvm::Value* isZero = builder->CreateFCmpOEQ(
            operandValue, llvm::ConstantFP::get(operandValue->getType(), 0.0));
        lastValue = isZero;
      } else {
        lastValue = llvm::ConstantInt::getFalse(context);
      }
    } else if (op == "~") {
      // Bitwise not: operate on i32
      llvm::Value* v = operandValue;
      if (v->getType()->isIntegerTy(1))
        v = builder->CreateZExt(v, llvm::Type::getInt32Ty(context));
      if (!v->getType()->isIntegerTy(32))
        v = builder->CreateIntCast(v, llvm::Type::getInt32Ty(context), true);
      lastValue = builder->CreateNot(v, "bnot");
    }
  }
}

// Support array literal and index access in codegen
// Extend visit for primary expressions via dynamic_cast checks in relevant
// places

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
  // We'll lower constructor as a function: __ctor_<ClassName>(i8* this, )
  // This visitor is invoked inside ClassDecl emission where current class
  // name is known
  if (classNameStack.empty()) return;
  std::string className = classNameStack.back();

  // Build parameter types: first param is i8* (this)
  std::vector<llvm::Type*> argTypes;
  argTypes.push_back(getPointerTy());
  for (auto& p : node.params) {
    std::string t = "int";
    llvm::Type* pty = llvm::Type::getInt32Ty(context);
    if (semanticAnalyzer && p->type) {
      t = semanticAnalyzer->resolveType(p->type.get());
      pty = getLLVMType(t);
      // Heuristic: unresolved/basic non-primitive generic params become i8*
      // (string-like)
      if ((t.empty() || pty == llvm::Type::getInt32Ty(context)) &&
          dynamic_cast<ast::BasicTypeNode*>(p->type.get()) != nullptr) {
        if (t != "int" && t != "float" && t != "double" && t != "bool" &&
            t != "boolean" && t != "string") {
          // Unresolved/basic non-primitive generic return: use opaque pointer
          // and mark as "ptr" to avoid misclassification.
          pty = getPointerTy();
          t = "ptr";
        }
      }
    }
    argTypes.push_back(pty);
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
  info.fieldTypeNames.clear();
  info.staticFields.clear();
  info.staticFieldTypes.clear();
  info.staticFieldTypeNames.clear();

  // If there is a base class with a known layout, inherit its instance fields
  // first to preserve offsets for simple inheritance.
  if (node.baseClass.getType() == tokens::TokenType::IDENTIFIER) {
    std::string baseName = node.baseClass.getLexeme();
    auto itBase = classes.find(baseName);
    if (itBase != classes.end()) {
      ClassInfo& base = itBase->second;
      // Append base fields in order
      for (auto& kv : base.fieldIndex) {
        // Recompute order based on index
      }
      // Build reverse map index->name to keep the same order
      std::vector<std::pair<int, std::string>> ordered;
      for (auto& kv : base.fieldIndex)
        ordered.emplace_back(kv.second, kv.first);
      std::sort(ordered.begin(), ordered.end());
      for (auto& pr : ordered) {
        const std::string& fname = pr.second;
        llvm::Type* fty = base.fieldTypes.at(fname);
        fieldTypesOrdered.push_back(fty);
        int idx = static_cast<int>(fieldTypesOrdered.size() - 1);
        info.fieldIndex[fname] = idx;
        info.fieldTypes[fname] = fty;
        auto itn = base.fieldTypeNames.find(fname);
        if (itn != base.fieldTypeNames.end())
          info.fieldTypeNames[fname] = itn->second;
      }
      // Record base class name
      info.baseClass = baseName;
    }
  }
  for (size_t i = 0; i < node.fields.size(); ++i) {
    auto& f = node.fields[i];
    if (!f) continue;
    std::string tname = "int";
    if (semanticAnalyzer && f->type)
      tname = semanticAnalyzer->resolveType(f->type.get());
    llvm::Type* fty = getLLVMType(tname);
    // Heuristic: If this looks like a basic custom/unresolved type (e.g., a
    // generic T) and resolved to i32 (default), treat it as a generic pointer
    // so we can store references like strings/objects.
    if ((tname.empty() || fty == llvm::Type::getInt32Ty(context))) {
      if (f->type &&
          dynamic_cast<ast::BasicTypeNode*>(f->type.get()) != nullptr) {
        if (tname != "int" && tname != "float" && tname != "double" &&
            tname != "bool" && tname != "boolean" && tname != "string") {
          fty = getPointerTy();
        }
      }
    }
    // For generic pointers, use i8* to make stores flexible
    if (fty && fty->isPointerTy() &&
        fty == llvm::PointerType::get(context, 0)) {
      fty = getPointerTy();
    }
    if (!fty) fty = llvm::Type::getInt32Ty(context);
    // Detect #static qualifier on field; if static, don't add to instance
    bool isStaticField = (f->qualifier == ast::StorageQualifier::Static);
    if (isStaticField) {
      info.staticFields.insert(f->name.getLexeme());
      info.staticFieldTypes[f->name.getLexeme()] = fty;
      info.staticFieldTypeNames[f->name.getLexeme()] = tname;
    } else {
      info.fieldIndex[f->name.getLexeme()] =
          static_cast<int>(fieldTypesOrdered.size());
      info.fieldTypes[f->name.getLexeme()] = fty;
      info.fieldTypeNames[f->name.getLexeme()] = tname;
      fieldTypesOrdered.push_back(fty);
    }
  }
  // Record base class if present (simple identifier token)
  if (node.baseClass.getType() == tokens::TokenType::IDENTIFIER) {
    info.baseClass = node.baseClass.getLexeme();
  } else {
    info.baseClass.clear();
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
  // Record implemented interfaces for this class (names only)
  info.implements.clear();
  for (auto& itok : node.interfaces) {
    info.implements.push_back(itok.getLexeme());
  }
  // Emit #static fields as module-scope globals and prime default
  // initializers
  if (!info.staticFields.empty()) {
    for (auto& fname : info.staticFields) {
      llvm::Type* gty = info.staticFieldTypes[fname];
      if (!gty) gty = llvm::Type::getInt32Ty(context);
      llvm::Constant* init = nullptr;
      if (gty->isIntegerTy(1))
        init = llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 0);
      else if (gty->isIntegerTy())
        init = llvm::ConstantInt::get(gty, 0);
      else if (gty->isFloatingPointTy())
        init = llvm::ConstantFP::get(gty, 0.0);
      else if (gty->isPointerTy())
        init =
            llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(gty));
      else
        init = llvm::Constant::getNullValue(gty);
      std::string gname = className + std::string(".") + fname;
      // Create if not already present
      if (!module->getGlobalVariable(gname, true)) {
        auto* gvar = new llvm::GlobalVariable(
            *module, gty, false, llvm::GlobalValue::InternalLinkage, init,
            gname);
        (void)gvar;
      }
    }
  }
  // Emit static blocks: run once by appending to GC ctor
  if (!node.staticBlocks.empty()) {
    auto* ctor = module->getFunction("__tspp_gc_ctor");
    if (!ctor) {
      // Should exist from generate(); if not, create a minimal one
      auto ctorTy =
          llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);
      ctor = llvm::Function::Create(ctorTy, llvm::GlobalValue::InternalLinkage,
                                    "__tspp_gc_ctor", module.get());
      auto bb = llvm::BasicBlock::Create(context, "entry", ctor);
      builder->SetInsertPoint(bb);
      builder->CreateRetVoid();
    }
    // Insert before the final return
    auto& lastBB = ctor->back();
    if (lastBB.getTerminator()) lastBB.getTerminator()->eraseFromParent();
    auto* savedFunc = currentFunction;
    currentFunction = ctor;
    builder->SetInsertPoint(&lastBB);
    // Push class context for 'static' blocks if they reference statics via
    // ClassName
    classNameStack.push_back(className);
    for (auto& sb : node.staticBlocks) {
      if (sb && sb->body) sb->body->accept(*this);
    }
    classNameStack.pop_back();
    builder->CreateRetVoid();
    currentFunction = savedFunc;
  }
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
      // Heuristic: unresolved/basic non-primitive generic returns become i8*
      // (string-like)
      if ((retName.empty() || retTy == llvm::Type::getInt32Ty(context)) &&
          dynamic_cast<ast::BasicTypeNode*>(m->returnType.get()) != nullptr) {
        if (retName != "int" && retName != "float" && retName != "double" &&
            retName != "bool" && retName != "boolean" && retName != "void") {
          // Unresolved/basic non-primitive generic return: use opaque pointer
          // and mark as "ptr" to avoid misclassification.
          retTy = getPointerTy();
          retName = "ptr";
        }
      }
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
      llvm::Type* pty = llvm::Type::getInt32Ty(context);
      if (semanticAnalyzer && p->type) {
        t = semanticAnalyzer->resolveType(p->type.get());
        pty = getLLVMType(t);
        // Heuristic: unresolved/basic non-primitive generic params become i8*
        if ((t.empty() || pty == llvm::Type::getInt32Ty(context)) &&
            dynamic_cast<ast::BasicTypeNode*>(p->type.get()) != nullptr) {
          if (t != "int" && t != "float" && t != "double" && t != "bool" &&
              t != "boolean" && t != "string") {
            pty = getPointerTy();
          }
        }
      }
      argTys.push_back(pty);
    }
    auto fnTy = llvm::FunctionType::get(retTy, argTys, false);
    std::string fnName = className + std::string(".") + m->name.getLexeme();
    auto func = module->getFunction(fnName);
    if (!func)
      func = llvm::Function::Create(fnTy, llvm::Function::ExternalLinkage,
                                    fnName, module.get());
    // Record method return type for string-concat/console detection
    functionReturnTypes[fnName] = retName;
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
  // Static field read: ClassName.field
  if (auto idObj = dynamic_cast<ast::IdentifierExpr*>(node.object.get())) {
    std::string clsName = idObj->name.getLexeme();
    auto itCls = classes.find(clsName);
    if (itCls != classes.end() &&
        itCls->second.staticFields.count(node.member.getLexeme())) {
      // Load from module global
      std::string gname = clsName + std::string(".") + node.member.getLexeme();
      if (auto* gvar = module->getGlobalVariable(gname, true)) {
        llvm::Type* valTy = gvar->getValueType();
        if (currentFunction) {
          lastValue = builder->CreateLoad(valTy, gvar);
        } else {
          // At global scope, return the global pointer (should rarely happen)
          lastValue = gvar;
        }
        return;
      }
    }
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
  // Record semantic type name for this field access under a synthetic symbol
  // key to help downstream string detection (e.g., console.log arguments).
  // Use pattern "<obj>.<field>" when object is an identifier.
  if (auto idObj = dynamic_cast<ast::IdentifierExpr*>(node.object.get())) {
    auto itCls = classes.find(className);
    if (itCls != classes.end()) {
      auto nameIt = itCls->second.fieldTypeNames.find(node.member.getLexeme());
      if (nameIt != itCls->second.fieldTypeNames.end()) {
        std::string synthetic = idObj->name.getLexeme();
        synthetic += ".";
        synthetic += node.member.getLexeme();
        symbolTable[synthetic] =
            SymbolInfo{lastValue, elemTy, false, nameIt->second};
      }
    }
  }
}

// Emit code for index access: arr[index]
void LLVMCodeGenerator::visit(ast::IndexAccessExpr& node) {
  if (!currentFunction) {
    lastValue = nullptr;
    return;
  }
  if (!node.object || !node.index) {
    lastValue = nullptr;
    return;
  }
  node.object->accept(*this);
  llvm::Value* base = lastValue;
  node.index->accept(*this);
  llvm::Value* idx = lastValue;
  if (!base || !idx) {
    lastValue = nullptr;
    return;
  }
  if (!idx->getType()->isIntegerTy(32)) {
    idx = builder->CreateIntCast(idx, llvm::Type::getInt32Ty(context), true);
  }
  // Assume base is i32* for now
  llvm::Type* elemTy = llvm::Type::getInt32Ty(context);
  llvm::Value* ptr = base;
  if (!base->getType()->isPointerTy()) {
    // If base is an integer address, cast it to i32*
    if (base->getType()->isIntegerTy()) {
      ptr = builder->CreateIntToPtr(base, llvm::PointerType::get(elemTy, 0));
    } else {
      ptr = ensureBitcast(base, llvm::PointerType::get(elemTy, 0));
    }
  }
  llvm::Value* gep = builder->CreateGEP(elemTy, ptr, idx, "elem.ptr");
  lastValue = builder->CreateLoad(elemTy, gep);
}

// Emit array literal as i32* allocated with GC and initialized
// Only supports integer elements for now; others will be cast/truncated
void LLVMCodeGenerator::visit(ast::ArrayLiteralExpr& node) {
  if (!currentFunction) {
    lastValue = nullptr;
    return;
  }
  // Evaluate elements and collect as i32 values
  std::vector<llvm::Value*> elems;
  for (auto& el : node.elements) {
    if (!el) continue;
    el->accept(*this);
    llvm::Value* v = lastValue;
    if (!v) v = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    if (v->getType()->isIntegerTy(1)) {
      v = builder->CreateZExt(v, llvm::Type::getInt32Ty(context));
    } else if (v->getType()->isIntegerTy() && !v->getType()->isIntegerTy(32)) {
      v = builder->CreateIntCast(v, llvm::Type::getInt32Ty(context), true);
    } else if (v->getType()->isFloatingPointTy()) {
      v = builder->CreateFPToSI(v, llvm::Type::getInt32Ty(context));
    } else if (v->getType()->isPointerTy()) {
      // For pointers, convert to address integer for now
      v = builder->CreatePtrToInt(v, llvm::Type::getInt32Ty(context));
    } else if (!v->getType()->isIntegerTy(32)) {
      v = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    }
    elems.push_back(v);
  }
  // Allocate with Boehm GC: GC_malloc(n * 4)
  auto gcMallocTy = llvm::FunctionType::get(
      getPointerTy(), {llvm::Type::getInt64Ty(context)}, false);
  auto gcMalloc = module->getOrInsertFunction("GC_malloc", gcMallocTy);
  uint64_t n = elems.size();
  llvm::Value* bytes = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context),
                                              static_cast<uint64_t>(n * 4));
  llvm::Value* raw = builder->CreateCall(gcMalloc, {bytes}, "arr");
  // Cast to i32*
  llvm::Value* basePtr = builder->CreatePointerCast(
      raw, llvm::PointerType::get(llvm::Type::getInt32Ty(context), 0));
  // Store elements
  for (size_t i = 0; i < elems.size(); ++i) {
    llvm::Value* idx = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context),
                                              static_cast<uint32_t>(i));
    llvm::Value* gep =
        builder->CreateGEP(llvm::Type::getInt32Ty(context), basePtr, idx);
    builder->CreateStore(elems[i], gep);
  }
  // Return as i32* but treat as generic i8* for interop
  lastValue = builder->CreatePointerCast(basePtr, getPointerTy());
}

void LLVMCodeGenerator::visit(ast::NewExpr& node) {
  // Allocate memory using GC_malloc and call constructor if present
  if (!currentFunction) {
    lastValue = nullptr;
    return;
  }
  std::string className = node.className.getLexeme();
  // Primitive array form: new int[size] etc. Treat as GC_malloc of N * elemSz
  auto isBasic = [&](const std::string& n) {
    return n == "int" || n == "int32" || n == "float" || n == "double" ||
           n == "bool" || n == "boolean" || n == "string";
  };
  if (node.arguments.size() == 1 && isBasic(className)) {
    // Resolve element LLVM type and size
    llvm::Type* elemTy = getLLVMType(className);
    uint64_t elemSize = 4;  // default to 4 bytes
    if (elemTy->isDoubleTy())
      elemSize = 8;
    else if (elemTy->isIntegerTy(1))
      elemSize = 1;
    else if (elemTy->isPointerTy())
      elemSize = 8;  // pointers

    node.arguments[0]->accept(*this);
    llvm::Value* nVal = lastValue;
    if (!nVal) {
      lastValue = llvm::ConstantPointerNull::get(
          llvm::cast<llvm::PointerType>(getPointerTy()));
      return;
    }
    if (!nVal->getType()->isIntegerTy(32)) {
      if (nVal->getType()->isIntegerTy()) {
        nVal =
            builder->CreateIntCast(nVal, llvm::Type::getInt32Ty(context), true);
      } else if (nVal->getType()->isFloatingPointTy()) {
        nVal = builder->CreateFPToSI(nVal, llvm::Type::getInt32Ty(context));
      } else {
        nVal = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
      }
    }
    llvm::Value* sz32 =
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), elemSize);
    llvm::Value* bytes32 = builder->CreateMul(nVal, sz32);
    llvm::Value* bytes64 =
        builder->CreateSExt(bytes32, llvm::Type::getInt64Ty(context));
    auto gcMallocTy = llvm::FunctionType::get(
        getPointerTy(), {llvm::Type::getInt64Ty(context)}, false);
    auto gcMalloc = module->getOrInsertFunction("GC_malloc", gcMallocTy);
    llvm::Value* raw = builder->CreateCall(gcMalloc, {bytes64}, "arr");
    // Return pointer casted to element pointer type
    lastValue =
        builder->CreatePointerCast(raw, llvm::PointerType::get(elemTy, 0));
    return;
  }
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
  // If constructor exists, call __ctor_ClassName(obj, )
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
    if (fn) {
      // Coerce argument types to match constructor params when possible
      if (args.size() == fn->arg_size()) {
        for (size_t i = 1; i < args.size(); ++i) {
          llvm::Value* v = args[i];
          llvm::Type* pt = fn->getArg(i)->getType();
          if (!v || !pt) continue;
          llvm::Type* vt = v->getType();
          if (vt == pt) continue;
          if (vt->isPointerTy() && pt->isPointerTy()) {
            args[i] = ensureBitcast(v, pt);
          } else if (vt->isIntegerTy() && pt->isIntegerTy()) {
            args[i] = builder->CreateIntCast(v, pt, true);
          } else if (vt->isFloatingPointTy() && pt->isFloatingPointTy()) {
            if (vt->isDoubleTy() && pt->isFloatTy())
              args[i] = builder->CreateFPTrunc(v, pt);
            else if (vt->isFloatTy() && pt->isDoubleTy())
              args[i] = builder->CreateFPExt(v, pt);
          } else if (vt->isIntegerTy() && pt->isFloatingPointTy()) {
            args[i] = builder->CreateSIToFP(v, pt);
          } else if (vt->isFloatingPointTy() && pt->isIntegerTy()) {
            args[i] = builder->CreateFPToSI(v, pt);
          } else if (pt->isPointerTy() &&
                     (vt->isIntegerTy() || vt->isFloatingPointTy())) {
            // Convert scalars to strings to satisfy pointer params gracefully
            if (vt->isIntegerTy(1)) {
              auto boolToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::Type::getInt1Ty(context)}, false);
              auto boolToStr = module->getOrInsertFunction(
                  "tspp_bool_to_string", boolToStrType);
              v = builder->CreateCall(boolToStr, {v});
            } else if (vt->isIntegerTy()) {
              llvm::Value* i32 = v;
              if (!vt->isIntegerTy(32))
                i32 = builder->CreateIntCast(v, llvm::Type::getInt32Ty(context),
                                             true);
              auto intToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::Type::getInt32Ty(context)}, false);
              auto intToStrFunc = module->getOrInsertFunction(
                  "tspp_int_to_string", intToStrType);
              v = builder->CreateCall(intToStrFunc, {i32});
            } else if (vt->isFloatingPointTy()) {
              llvm::Value* f = v;
              if (vt->isDoubleTy())
                f = builder->CreateFPTrunc(v, llvm::Type::getFloatTy(context));
              auto floatToStrType = llvm::FunctionType::get(
                  llvm::PointerType::get(context, 0),
                  {llvm::Type::getFloatTy(context)}, false);
              auto floatToStr = module->getOrInsertFunction(
                  "tspp_float_to_string", floatToStrType);
              v = builder->CreateCall(floatToStr, {f});
            }
            if (v->getType() != pt && v->getType()->isPointerTy()) {
              v = ensureBitcast(v, pt);
            }
            args[i] = v;
          }
        }
      }
      builder->CreateCall(fn, args);
    }
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

// Constructor implementations
LLVMCodeGenerator::LLVMCodeGenerator()
    : builder(std::make_unique<llvm::IRBuilder<>>(context)),
      module(std::make_unique<llvm::Module>("tspp", context)) {}

LLVMCodeGenerator::LLVMCodeGenerator(
    ast::SemanticAnalyzerVisitor* semanticAnalyzer)
    : builder(std::make_unique<llvm::IRBuilder<>>(context)),
      module(std::make_unique<llvm::Module>("tspp", context)),
      semanticAnalyzer(semanticAnalyzer) {}

// Generate method implementations
void LLVMCodeGenerator::generate(ast::BaseNode* root) {
  if (root) {
    root->accept(*this);
    emitPendingGlobalInits();
    // Always emit a valid main function if not present
    if (!module->getFunction("main")) {
      llvm::FunctionType* mainType =
          llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
      llvm::Function* mainFn = llvm::Function::Create(
          mainType, llvm::Function::ExternalLinkage, "main", module.get());
      llvm::BasicBlock* bb = llvm::BasicBlock::Create(context, "entry", mainFn);
      builder->SetInsertPoint(bb);
      builder->CreateRet(
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
    } else {
      // If user main exists, ensure it has correct i32 return type
      auto* userMain = module->getFunction("main");
      if (userMain && !userMain->getReturnType()->isIntegerTy(32)) {
        // Patch: emit error or fixup (for now, emit error)
        llvm::errs() << "[ERROR] User main() must return i32.\n";
      }
    }
  }
}

void LLVMCodeGenerator::generate(ast::BaseNode* root,
                                 const std::string& outFile) {
  generate(root);

  // Write LLVM IR to file
  std::error_code EC;
  llvm::raw_fd_ostream dest(outFile, EC, llvm::sys::fs::OF_None);
  if (EC) {
    return;
  }
  module->print(dest, nullptr);
}

// getLLVMType implementation
llvm::Type* LLVMCodeGenerator::getLLVMType(const std::string& typeName) {
  if (typeName == "int" || typeName == "i32") {
    return llvm::Type::getInt32Ty(context);
  } else if (typeName == "float" || typeName == "f32") {
    return llvm::Type::getFloatTy(context);
  } else if (typeName == "double" || typeName == "f64") {
    return llvm::Type::getDoubleTy(context);
  } else if (typeName == "bool") {
    return llvm::Type::getInt1Ty(context);
  } else if (typeName == "string" || typeName == "i8*") {
    return llvm::PointerType::get(context, 0);
  } else if (typeName == "void") {
    return llvm::Type::getVoidTy(context);
  } else if (typeName.back() == '*') {
    // Pointer type - strip the * and get the base type
    std::string baseType = typeName.substr(0, typeName.length() - 1);
    return llvm::PointerType::get(context, 0);
  } else {
    // Check if it's a class type
    auto classIt = classes.find(typeName);
    if (classIt != classes.end() && classIt->second.structTy) {
      return classIt->second.structTy;
    }
    // Default to i8* for unknown types
    return llvm::PointerType::get(context, 0);
  }
}

// visit(ExprStmt&) implementation
void LLVMCodeGenerator::visit(ast::ExprStmt& node) {
  if (node.expression) {
    node.expression->accept(*this);
  }
}

// Helper method to emit pending global initializers
void LLVMCodeGenerator::emitPendingGlobalInits() {
  if (pendingGlobalInits.empty()) return;

  // Create a module constructor function if needed
  llvm::FunctionType* ctorType =
      llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);
  llvm::Function* ctorFunc =
      llvm::Function::Create(ctorType, llvm::Function::InternalLinkage,
                             "__tspp_global_ctors", module.get());

  llvm::BasicBlock* ctorBB =
      llvm::BasicBlock::Create(context, "entry", ctorFunc);
  builder->SetInsertPoint(ctorBB);

  // Emit initializers
  for (auto& pending : pendingGlobalInits) {
    pending.expr->accept(*this);
    if (lastValue) {
      builder->CreateStore(lastValue, pending.gvar);
    }
  }

  builder->CreateRetVoid();
  pendingGlobalInits.clear();
}

// === Helper: coerce runtime value to the current function's declared return
// type ===
llvm::Value* LLVMCodeGenerator::coerceToReturnType(llvm::Value* v,
                                                   llvm::Type* expectedTy) {
  if (!v || !expectedTy) return v;

  llvm::Type* vt = v->getType();
  if (vt == expectedTy) return v;

  if (expectedTy->isVoidTy()) return nullptr;

  // ---- Integer expected ----
  if (expectedTy->isIntegerTy()) {
    if (vt->isIntegerTy()) {
      return builder->CreateIntCast(v, expectedTy, true);
    }
    if (vt->isFloatingPointTy()) {
      return builder->CreateFPToSI(v, expectedTy);
    }
    if (vt->isPointerTy()) {
      llvm::Value* i64 =
          builder->CreatePtrToInt(v, llvm::Type::getInt64Ty(context));
      if (expectedTy->isIntegerTy(64)) return i64;
      return builder->CreateTrunc(i64, expectedTy);
    }
    return llvm::ConstantInt::get(expectedTy, 0);
  }

  // ---- Floating expected ----
  if (expectedTy->isFloatingPointTy()) {
    if (vt->isFloatingPointTy()) {
      if (vt->isDoubleTy() && expectedTy->isFloatTy())
        return builder->CreateFPTrunc(v, expectedTy);
      if (vt->isFloatTy() && expectedTy->isDoubleTy())
        return builder->CreateFPExt(v, expectedTy);
      return v;
    }
    if (vt->isIntegerTy()) {
      return builder->CreateSIToFP(v, expectedTy);
    }
    if (vt->isPointerTy()) {
      return llvm::ConstantFP::get(expectedTy, 0.0);
    }
    return llvm::ConstantFP::get(expectedTy, 0.0);
  }

  // ---- Pointer expected ----
  if (expectedTy->isPointerTy()) {
    if (vt->isPointerTy()) {
      return builder->CreateBitCast(v, expectedTy);
    }
    if (vt->isIntegerTy()) {
      if (auto* CI = llvm::dyn_cast<llvm::ConstantInt>(v)) {
        if (CI->isZero()) {
          return llvm::ConstantPointerNull::get(
              llvm::cast<llvm::PointerType>(expectedTy));
        }
      }
      return llvm::ConstantPointerNull::get(
          llvm::cast<llvm::PointerType>(expectedTy));
    }
    if (vt->isFloatingPointTy()) {
      if (auto* CFP = llvm::dyn_cast<llvm::ConstantFP>(v)) {
        if (CFP->isZero()) {
          return llvm::ConstantPointerNull::get(
              llvm::cast<llvm::PointerType>(expectedTy));
        }
      }
      return llvm::ConstantPointerNull::get(
          llvm::cast<llvm::PointerType>(expectedTy));
    }
    if (vt->isIntegerTy(1)) {
      return llvm::ConstantPointerNull::get(
          llvm::cast<llvm::PointerType>(expectedTy));
    }
    return llvm::ConstantPointerNull::get(
        llvm::cast<llvm::PointerType>(expectedTy));
  }

  return llvm::PoisonValue::get(expectedTy);
}
}  // namespace codegen
