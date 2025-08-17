#include "SemanticAnalyzerVisitor.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include "core/common/macros.h"
#include "parser/nodes/meta_nodes.h"
#include "parser/nodes/misc_nodes.h"
#include "parser/nodes/statement_sequence_node.h"  // Ensure StatementSequenceNode is a complete type
#include "parser/nodes/type_nodes.h"
#include "parser/visitors/semantic/Symbol.h"

namespace ast {

SemanticAnalyzerVisitor::SemanticAnalyzerVisitor() {
  currentScope = std::make_shared<SemanticScope>();
  // Register built-in global 'console'
  Symbol consoleSym;
  consoleSym.name = "console";
  consoleSym.typeName = "builtin_object";
  currentScope->insert("console", consoleSym);
}

void SemanticAnalyzerVisitor::enterScope() {
  currentScope = std::make_shared<SemanticScope>(currentScope);
}

void SemanticAnalyzerVisitor::exitScope() {
  if (currentScope->getParent()) {
    currentScope = currentScope->getParent();
  }
}

void SemanticAnalyzerVisitor::reportError(const std::string& message) {
  errors.push_back(message);
}

const std::vector<std::string>& SemanticAnalyzerVisitor::getErrors() const {
  return errors;
}

std::string SemanticAnalyzerVisitor::resolveType(TypeNode* type) {
  if (!type) return "";

  // Handle BasicTypeNode (int, string, or custom type alias)
  if (auto* basic = dynamic_cast<BasicTypeNode*>(type)) {
    std::string typeName = basic->name.getLexeme();

    // Check if it's a type alias
    auto it = typeAliases.find(typeName);
    if (it != typeAliases.end()) {
      // Recursively resolve the aliased type
      return resolveType(it->second.get());
    }

    // Built-in type or unresolved custom type
    return typeName;
  }

  // Handle PointerTypeNode
  if (auto* ptr = dynamic_cast<PointerTypeNode*>(type)) {
    std::string baseType = resolveType(ptr->baseType.get());
    return baseType + "*";
  }

  // Handle SmartPointerTypeNode
  if (auto* smart = dynamic_cast<SmartPointerTypeNode*>(type)) {
    std::string baseType = resolveType(smart->target.get());
    return smart->kind.getLexeme() + "<" + baseType + ">";
  }

  // Handle GenericTypeNode: Name<Arg1, Arg2, ...>
  if (auto* gen = dynamic_cast<GenericTypeNode*>(type)) {
    std::string name = gen->name.getLexeme();
    std::string out = name;
    out += "<";
    for (size_t i = 0; i < gen->typeArgs.size(); ++i) {
      if (i > 0) out += ", ";
      out += resolveType(gen->typeArgs[i].get());
    }
    out += ">";
    return out;
  }

  // Handle UnionTypeNode (for now, just return first type)
  if (auto* union_type = dynamic_cast<UnionTypeNode*>(type)) {
    if (!union_type->types.empty()) {
      // Prefer first non-empty type; if includes string, treat as string
      for (auto& ty : union_type->types) {
        auto n = resolveType(ty.get());
        if (n == "string") return n;
        if (!n.empty()) return n;
      }
    }
  }

  // Handle IntersectionTypeNode: if any constituent resolves to 'string'
  // (e.g., object type literals), prefer 'string'. Otherwise, return the
  // first non-empty resolved type.
  if (auto* inter = dynamic_cast<IntersectionTypeNode*>(type)) {
    if (!inter->types.empty()) {
      std::string firstNonEmpty;
      for (auto& ty : inter->types) {
        auto n = resolveType(ty.get());
        if (n == "string") return n;
        if (firstNonEmpty.empty() && !n.empty()) firstNonEmpty = n;
      }
      return firstNonEmpty;
    }
  }

  // Handle ObjectTypeNode: treat as string for logging and generic pointer at
  // runtime
  if (auto* obj = dynamic_cast<ObjectTypeNode*>(type)) {
    (void)obj;
    return "string";  // printable representation
  }

  // Fallback for other types
  return "";
}

// --- Visitor Implementations ---

// === Default and basic logic for all nodes ===
void SemanticAnalyzerVisitor::visit(BaseNode&) {}
void SemanticAnalyzerVisitor::visit(Expr&) {}
void SemanticAnalyzerVisitor::visit(Stmt&) {}
void SemanticAnalyzerVisitor::visit(Decl&) {}
void SemanticAnalyzerVisitor::visit(TypeNode&) {}

void SemanticAnalyzerVisitor::visit(BinaryExpr& node) {
  if (node.left) node.left->accept(*this);
  if (node.right) node.right->accept(*this);
  // TODO: Type checking for binary operations
}

void SemanticAnalyzerVisitor::visit(UnaryExpr& node) {
  if (node.operand) node.operand->accept(*this);
  // TODO: Type checking for unary operations
}

void SemanticAnalyzerVisitor::visit(LiteralExpr&) {
  // Literals are always valid
}

void SemanticAnalyzerVisitor::visit(ObjectLiteralExpr& node) {
  // Record keys to avoid misinterpreting them as undeclared identifiers
  std::vector<std::string> keysAdded;
  keysAdded.reserve(node.fields.size());
  for (auto& f : node.fields) {
    auto k = f.key.getLexeme();
    if (objectLiteralKeys_.insert(k).second) keysAdded.push_back(k);
  }
  // Validate values but don't treat keys as identifiers
  for (auto& f : node.fields) {
    if (f.value) f.value->accept(*this);
  }
  // Remove keys added for this literal to restore previous state
  for (auto& k : keysAdded) objectLiteralKeys_.erase(k);
}

void SemanticAnalyzerVisitor::visit(MemberAccessExpr& node) {
  if (node.object) node.object->accept(*this);
  // If object is 'console', allow 'log', 'error', 'warn' as built-in members
  if (auto objIdent = dynamic_cast<IdentifierExpr*>(node.object.get())) {
    if (objIdent->name.getLexeme() == "console") {
      std::string member = node.member.getLexeme();
      if (member == "log" || member == "error" || member == "warn") {
        // OK: treat as built-in
        return;
      }
    }
  }
  // TODO: Check member existence/type for other objects
}

void SemanticAnalyzerVisitor::visit(ExprStmt& node) {
  // Patch: If the expression is a variable declaration, visit as VarDecl
  if (node.expression) {
    if (auto varDecl = dynamic_cast<VarDecl*>(node.expression.get())) {
      varDecl->accept(*this);
    } else {
      node.expression->accept(*this);
    }
  }
}

void SemanticAnalyzerVisitor::visit(IfStmt& node) {
  if (node.condition) node.condition->accept(*this);
  if (node.thenBranch) node.thenBranch->accept(*this);
  if (node.elseBranch) node.elseBranch->accept(*this);
}

void SemanticAnalyzerVisitor::visit(WhileStmt& node) {
  if (node.condition) node.condition->accept(*this);
  if (node.body) node.body->accept(*this);
}

void SemanticAnalyzerVisitor::visit(ForStmt& node) {
  // for-loop introduces its own scope for initializer declarations
  enterScope();
  if (node.init) {
    // Init can be a VarDecl or an ExprStmt
    if (auto varDecl = dynamic_cast<VarDecl*>(node.init.get())) {
      // Insert symbol before analyzing condition/increment
      std::string name = varDecl->name.getLexeme();
      currentScope->insert(
          name, Symbol{name, resolveType(varDecl->type.get()), varDecl->isConst,
                       false, false, nullptr});
      varDecl->accept(*this);
    } else if (auto exprStmt = dynamic_cast<ExprStmt*>(node.init.get())) {
      exprStmt->accept(*this);
    } else {
      node.init->accept(*this);
    }
  }
  if (node.condition) node.condition->accept(*this);
  if (node.increment) node.increment->accept(*this);
  if (node.body) node.body->accept(*this);
  exitScope();
}

void SemanticAnalyzerVisitor::visit(Parameter& node) {
  // Parameters are handled in FunctionDecl, but could check type validity here
}

void SemanticAnalyzerVisitor::visit(ClassDecl& node) {
  std::string className = node.name.getLexeme();
  // If not already hoisted, insert; otherwise don't re-insert to avoid
  // duplicates
  if (!currentScope->lookup(className)) {
    currentScope->insert(className, Symbol{className, /*typeName*/ className,
                                           false, false, true, nullptr});
  }
  // Hoist methods as free functions with ClassName.method signature
  for (auto& method : node.methods) {
    if (!method) continue;
    std::string mname = method->name.getLexeme();
    std::string fq = className + "." + mname;
    if (!currentScope->lookup(fq)) {
      currentScope->insert(
          fq, Symbol{fq, /*typeName*/ "", false, true, false, nullptr});
    }
  }
  // Hoist constructor as __ctor_ClassName
  if (node.constructor) {
    std::string ctorName = std::string("__ctor_") + className;
    if (!currentScope->lookup(ctorName)) {
      currentScope->insert(ctorName, Symbol{ctorName, /*typeName*/ "", false,
                                            true, false, nullptr});
    }
  }
  classStack.push_back(className);
  enterScope();
  // Visit new structured members if present
  for (auto& field : node.fields) {
    if (field) field->accept(*this);
  }
  for (auto& method : node.methods) {
    if (method) method->accept(*this);
  }
  if (node.constructor) node.constructor->accept(*this);
  // Preserve legacy behavior to allow any stray body statements
  if (node.body) node.body->accept(*this);
  exitScope();
  classStack.pop_back();
}

void SemanticAnalyzerVisitor::visit(InterfaceDecl& node) {
  std::string ifaceName = node.name.getLexeme();
  if (!currentScope->lookup(ifaceName)) {
    currentScope->insert(ifaceName, Symbol{ifaceName, /*typeName*/ ifaceName,
                                           false, false, false, nullptr});
  }
  enterScope();
  if (node.body) node.body->accept(*this);
  exitScope();
}

void SemanticAnalyzerVisitor::visit(TypeAliasDecl& node) {
  std::string aliasName = node.name.getLexeme();
  // Store the alias mapping in the typeAliases table
  typeAliases[aliasName] = node.aliasedType;
  if (!currentScope->lookup(aliasName)) {
    currentScope->insert(aliasName, Symbol{aliasName, /*typeName*/ aliasName,
                                           false, false, false, nullptr});
  }
}

void SemanticAnalyzerVisitor::visit(ImportDecl&) {
  // Imports are not semantically checked here
}

void SemanticAnalyzerVisitor::visit(BasicTypeNode&) {}
void SemanticAnalyzerVisitor::visit(PointerTypeNode&) {}
void SemanticAnalyzerVisitor::visit(SmartPointerTypeNode&) {}
void SemanticAnalyzerVisitor::visit(UnionTypeNode&) {}
void SemanticAnalyzerVisitor::visit(IntersectionTypeNode&) {}
void SemanticAnalyzerVisitor::visit(ObjectTypeNode&) {}
void SemanticAnalyzerVisitor::visit(TypeConstraintNode&) {}

void SemanticAnalyzerVisitor::visit(ModifierNode&) {}
void SemanticAnalyzerVisitor::visit(StorageQualifierNode&) {}
void SemanticAnalyzerVisitor::visit(ParameterListNode&) {}

void SemanticAnalyzerVisitor::visit(GroupingExpr& node) {
  if (node.inner) node.inner->accept(*this);
}

void SemanticAnalyzerVisitor::visit(ThisExpr& node) {
  // 'this' is valid if we're currently visiting a class
  if (!classStack.empty()) return;
  // Fallback: scan scope chain for any class symbol
  std::shared_ptr<SemanticScope> scope = currentScope;
  while (scope) {
    if (scope->containsClassSymbol()) return;
    scope = scope->getParent();
  }
  reportError("'this' used outside of a class context");
}

void SemanticAnalyzerVisitor::visit(NullExpr&) {
  // Null is always valid
}

// --- Main semantic logic for key nodes ---

void SemanticAnalyzerVisitor::visit(ProgramNode& node) {
  enterScope();
  // Pass 1: hoist top-level declarations (functions, classes, vars, typedefs)
  for (auto& decl : node.declarations) {
    if (!decl) continue;
    if (auto fn = dynamic_cast<FunctionDecl*>(decl.get())) {
      std::string name = fn->name.getLexeme();
      currentScope->insert(
          name, Symbol{name, /*typeName*/ "", false, true, false, decl});
    } else if (auto var = dynamic_cast<VarDecl*>(decl.get())) {
      std::string name = var->name.getLexeme();
      currentScope->insert(name, Symbol{name, resolveType(var->type.get()),
                                        var->isConst, false, false, nullptr});
    } else if (auto cls = dynamic_cast<ClassDecl*>(decl.get())) {
      std::string name = cls->name.getLexeme();
      currentScope->insert(name,
                           Symbol{name, name, false, false, true, nullptr});
    } else if (auto alias = dynamic_cast<TypeAliasDecl*>(decl.get())) {
      std::string name = alias->name.getLexeme();
      typeAliases[name] = alias->aliasedType;
      currentScope->insert(name,
                           Symbol{name, name, false, false, false, nullptr});
    }
  }
  // Pass 1b: hoist top-level variable declarations that appear as statements
  for (auto& stmt : node.statements) {
    if (!stmt) continue;
    if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt.get())) {
      if (exprStmt->expression) {
        if (auto varDecl = dynamic_cast<VarDecl*>(exprStmt->expression.get())) {
          std::string name = varDecl->name.getLexeme();
          currentScope->insert(name,
                               Symbol{name, resolveType(varDecl->type.get()),
                                      varDecl->isConst, false, false, nullptr});
        }
      }
    } else if (auto varDecl = dynamic_cast<VarDecl*>(stmt.get())) {
      std::string name = varDecl->name.getLexeme();
      currentScope->insert(
          name, Symbol{name, resolveType(varDecl->type.get()), varDecl->isConst,
                       false, false, nullptr});
    }
  }
  // Pass 2: analyze declarations fully
  for (auto& decl : node.declarations) {
    if (decl) decl->accept(*this);
  }
  // Pass 3: analyze top-level statements
  for (auto& stmt : node.statements) {
    if (stmt) stmt->accept(*this);
  }
  exitScope();
}

void SemanticAnalyzerVisitor::visit(BlockStmt& node) {
  for (size_t i = 0; i < node.statements.size(); ++i) {
    auto& stmt = node.statements[i];
    if (!stmt) continue;
    // Patch: If statement is ExprStmt and contains VarDecl, visit as VarDecl
    if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt.get())) {
      if (exprStmt->expression) {
        if (auto varDecl = dynamic_cast<VarDecl*>(exprStmt->expression.get())) {
          varDecl->accept(*this);
          continue;
        }
      }
    }
    stmt->accept(*this);
  }
}

void SemanticAnalyzerVisitor::visit(VarDecl& node) {
  std::string varName = node.name.getLexeme();
  std::string typeName = resolveType(node.type.get());
  void* llvmType = nullptr;
  if (typeName == "int") {
    llvmType = (void*)nullptr;
  } else if (typeName == "string") {
    llvmType = (void*)nullptr;
  }
  Symbol sym{
      varName, /*typeName*/ typeName, node.isConst, false, false, nullptr,
      llvmType};
  // If already hoisted in the current scope, don't re-insert or error
  if (!currentScope->lookup(varName)) {
    if (!currentScope->insert(varName, sym)) {
      reportError("Variable '" + varName + "' already declared in this scope");
    }
  }
  if (node.initializer) {
    node.initializer->accept(*this);
    // TODO: Type compatibility check
  }
}

void SemanticAnalyzerVisitor::visit(FunctionDecl& node) {
  std::string funcName = node.name.getLexeme();
  // If already hoisted in the current scope, don't re-insert or error
  if (!currentScope->lookup(funcName)) {
    if (!currentScope->insert(funcName, Symbol{funcName, /*typeName*/ "", false,
                                               true, false, nullptr})) {
      reportError("Function '" + funcName + "' already declared in this scope");
    }
  }
  enterScope();
  for (auto& param : node.params) {
    if (param) {
      std::string paramName = param->name.getLexeme();
      currentScope->insert(paramName, Symbol{paramName, /*typeName*/ "", false,
                                             false, false, nullptr});
    }
  }
  if (node.body) {
    // Try BlockStmt first
    if (auto block = dynamic_cast<BlockStmt*>(node.body.get())) {
      block->accept(*this);
    }
    // Try StatementSequenceNode next
    else if (auto seq = dynamic_cast<StatementSequenceNode*>(node.body.get())) {
      seq->accept(*this);
    }
    // Fallback: treat as generic Stmt
    else {
      node.body->accept(*this);
    }
  }
  exitScope();
}

void SemanticAnalyzerVisitor::visit(ConstructorDecl& node) {
  // Constructor introduces a new scope for its parameters and body
  enterScope();
  for (auto& param : node.params) {
    if (param) {
      std::string paramName = param->name.getLexeme();
      currentScope->insert(paramName, Symbol{paramName, /*typeName*/ "", false,
                                             false, false, nullptr});
    }
  }
  if (node.body) node.body->accept(*this);
  exitScope();
}

void SemanticAnalyzerVisitor::visit(IdentifierExpr& node) {
  std::string idName = node.name.getLexeme();
  // If this identifier matches an object-literal property key we've seen,
  // suppress false-positive undeclared errors. Keys are not variables.
  if (objectLiteralKeys_.find(idName) != objectLiteralKeys_.end()) {
    return;
  }
  auto sym = currentScope->lookup(idName);
  if (!sym) {
    reportError("Use of undeclared identifier '" + idName + "'");
  }
}

void SemanticAnalyzerVisitor::visit(AssignmentExpr& node) {
  // Support identifier assignments and member assignments (e.g., this.x = y)
  if (auto targetIdent = dynamic_cast<IdentifierExpr*>(node.target.get())) {
    std::string varName = targetIdent->name.getLexeme();
    auto sym = currentScope->lookup(varName);
    if (sym) {
      if (sym->isConst) {
        reportError("Cannot assign to const variable '" + varName + "'");
      }
      // TODO: Type compatibility check between target and value
    } else {
      reportError("Assignment to undeclared variable '" + varName + "'");
    }
  } else if (auto memberTarget =
                 dynamic_cast<MemberAccessExpr*>(node.target.get())) {
    // For now, allow member assignments without detailed checks.
    // If it's 'this.something', 'this' validity will be checked when visiting
    // the target.
    (void)memberTarget;  // suppress unused warning in some builds
  } else if (dynamic_cast<IndexAccessExpr*>(node.target.get())) {
    // Support indexed assignment like a[i] = v
    // Detailed bounds/type checks omitted for now
  } else {
    reportError("Unsupported assignment target");
  }

  // Visit the target and value for completeness (also validates 'this' usage)
  if (node.target) node.target->accept(*this);
  if (node.value) node.value->accept(*this);
}

void SemanticAnalyzerVisitor::visit(CallExpr& node) {
  std::string funcName;
  // Support identifier and member access callees
  if (auto id = dynamic_cast<IdentifierExpr*>(node.callee.get())) {
    funcName = id->name.getLexeme();
  } else if (auto member = dynamic_cast<MemberAccessExpr*>(node.callee.get())) {
    // Compose name for console.* or ClassName.method
    if (auto objIdent = dynamic_cast<IdentifierExpr*>(member->object.get())) {
      std::string objName = objIdent->name.getLexeme();
      if (objName == "console") {
        funcName = objName + "." + member->member.getLexeme();
      } else {
        // Look up variable type; if it's a class type, use that
        auto sym = currentScope->lookup(objName);
        if (sym && !sym->typeName.empty()) {
          std::string typeName = sym->typeName;
          // strip pointer star if present
          if (!typeName.empty() && typeName.back() == '*') typeName.pop_back();
          funcName = typeName + "." + member->member.getLexeme();
        }
      }
    } else if (dynamic_cast<ThisExpr*>(member->object.get())) {
      // this.method() inside class -> ClassName.method
      if (!classStack.empty()) {
        funcName = classStack.back() + "." + member->member.getLexeme();
      }
    }
  }
  // Allow built-in console methods
  if (funcName == "console.log" || funcName == "console.error" ||
      funcName == "console.warn") {
    // OK: treat as built-in
  } else {
    auto sym = currentScope->lookup(funcName);
    if (!sym || !sym->isFunction) {
      reportError("Call to undeclared function '" + funcName + "'");
      return;
    }
    // Attempt to fetch the FunctionDecl node to inspect parameters
    auto fnDecl = std::dynamic_pointer_cast<FunctionDecl>(sym->declNode);
    if (fnDecl) {
      size_t arg_before = node.arguments.size();
      size_t param_count = fnDecl->params.size();
      LOG_DEBUG("CallExpr callee=" << funcName
                                   << ", arg_count_before=" << arg_before
                                   << ", param_count=" << param_count);
      // Fill default parameters exactly once
      if (!node.defaultsFilledOnce_ && !inDefaultParamEval_) {
        // Prevent re-entry filling
        node.defaultsFilledOnce_ = true;
        // Only append missing arguments up to param_count
        for (size_t i = arg_before; i < param_count; ++i) {
          auto& p = fnDecl->params[i];
          if (p && p->defaultExpr) {
            // Resolve default expression in callee scope: we simulate by
            // visiting once and marking cached
            if (!p->defaultResolved_) {
              LOG_TRACE("Resolving default for param index " << i);
              // Evaluate in a nested scope to emulate callee lexical env
              enterScope();
              DefaultParamEvalGuard guard(*this);
              // Insert prior parameters into scope (names only)
              for (size_t j = 0; j < fnDecl->params.size(); ++j) {
                auto& pj = fnDecl->params[j];
                if (pj) {
                  std::string pname = pj->name.getLexeme();
                  currentScope->insert(
                      pname, Symbol{pname, "", false, false, false, nullptr});
                }
              }
              p->defaultExpr->accept(*this);
              exitScope();
              p->defaultResolved_ = true;
            }
            node.arguments.push_back(p->defaultExpr);  // synthesize
          } else {
            reportError(
                "Missing argument for parameter without default in call to '" +
                funcName + "'");
            // Insert null literal placeholder to avoid vector underflow later
            node.arguments.push_back(nullptr);
          }
        }
      }
      size_t arg_after = node.arguments.size();
      LOG_DEBUG("CallExpr callee="
                << funcName << ", arg_count_after_default_fill=" << arg_after);
      ASSERT(arg_after == std::max(arg_before, param_count));
      // Basic type-check placeholder: ensure not fewer than params
      if (arg_after < param_count) {
        reportError("Not enough arguments supplied to function '" + funcName +
                    "'");
        return;
      }
    }
  }
  for (auto& arg : node.arguments) {
    if (arg) arg->accept(*this);
  }
}

void SemanticAnalyzerVisitor::visit(ReturnStmt& node) {
  if (node.value) {
    node.value->accept(*this);
    // TODO: Check return type matches function signature
  }
}

void SemanticAnalyzerVisitor::visit(StatementSequenceNode& node) {
  for (size_t i = 0; i < node.statements.size(); ++i) {
    if (node.statements[i]) {
      node.statements[i]->accept(*this);
    }
  }
}

}  // namespace ast
