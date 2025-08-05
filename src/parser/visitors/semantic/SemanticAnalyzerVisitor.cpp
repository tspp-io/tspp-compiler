#include "SemanticAnalyzerVisitor.h"

#include <iostream>
#include <sstream>

#include "parser/nodes/meta_nodes.h"
#include "parser/nodes/misc_nodes.h"
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

  // Handle UnionTypeNode (for now, just return first type)
  if (auto* union_type = dynamic_cast<UnionTypeNode*>(type)) {
    if (!union_type->types.empty()) {
      return resolveType(union_type->types[0].get());
    }
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
  if (node.expression) node.expression->accept(*this);
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
  if (node.init) node.init->accept(*this);
  if (node.condition) node.condition->accept(*this);
  if (node.increment) node.increment->accept(*this);
  if (node.body) node.body->accept(*this);
}

void SemanticAnalyzerVisitor::visit(Parameter& node) {
  // Parameters are handled in FunctionDecl, but could check type validity here
}

void SemanticAnalyzerVisitor::visit(ClassDecl& node) {
  std::string className = node.name.getLexeme();
  if (!currentScope->insert(className, Symbol{className, /*typeName*/ className,
                                              false, false, true, nullptr})) {
    reportError("Class '" + className + "' already declared in this scope");
  }
  enterScope();
  if (node.body) node.body->accept(*this);
  exitScope();
}

void SemanticAnalyzerVisitor::visit(InterfaceDecl& node) {
  std::string ifaceName = node.name.getLexeme();
  if (!currentScope->insert(ifaceName, Symbol{ifaceName, /*typeName*/ ifaceName,
                                              false, false, false, nullptr})) {
    reportError("Interface '" + ifaceName + "' already declared in this scope");
  }
  enterScope();
  if (node.body) node.body->accept(*this);
  exitScope();
}

void SemanticAnalyzerVisitor::visit(TypeAliasDecl& node) {
  std::string aliasName = node.name.getLexeme();
  // Store the alias mapping in the typeAliases table
  typeAliases[aliasName] = node.aliasedType;
  if (!currentScope->insert(aliasName, Symbol{aliasName, /*typeName*/ aliasName,
                                              false, false, false, nullptr})) {
    reportError("Type alias '" + aliasName +
                "' already declared in this scope");
  }
}

void SemanticAnalyzerVisitor::visit(ImportDecl&) {
  // Imports are not semantically checked here
}

void SemanticAnalyzerVisitor::visit(BasicTypeNode&) {}
void SemanticAnalyzerVisitor::visit(PointerTypeNode&) {}
void SemanticAnalyzerVisitor::visit(SmartPointerTypeNode&) {}
void SemanticAnalyzerVisitor::visit(UnionTypeNode&) {}
void SemanticAnalyzerVisitor::visit(TypeConstraintNode&) {}

void SemanticAnalyzerVisitor::visit(ModifierNode&) {}
void SemanticAnalyzerVisitor::visit(StorageQualifierNode&) {}
void SemanticAnalyzerVisitor::visit(ParameterListNode&) {}

void SemanticAnalyzerVisitor::visit(GroupingExpr& node) {
  if (node.inner) node.inner->accept(*this);
}

void SemanticAnalyzerVisitor::visit(ThisExpr& node) {
  // Check if 'this' is valid in the current context (must be inside a class)
  std::shared_ptr<SemanticScope> scope = currentScope;
  bool foundClass = false;
  while (scope) {
    if (scope->containsClassSymbol()) {
      foundClass = true;
      break;
    }
    scope = scope->getParent();
  }
  if (!foundClass) {
    reportError("'this' used outside of a class context");
  }
}

void SemanticAnalyzerVisitor::visit(NullExpr&) {
  // Null is always valid
}

// --- Main semantic logic for key nodes ---

void SemanticAnalyzerVisitor::visit(ProgramNode& node) {
  enterScope();
  for (auto& decl : node.declarations) {
    if (decl) decl->accept(*this);
  }
  for (auto& stmt : node.statements) {
    if (stmt) stmt->accept(*this);
  }
  exitScope();
}

void SemanticAnalyzerVisitor::visit(BlockStmt& node) {
  enterScope();
  for (auto& stmt : node.statements) {
    if (stmt) stmt->accept(*this);
  }
  exitScope();
}

void SemanticAnalyzerVisitor::visit(VarDecl& node) {
  std::string varName = node.name.getLexeme();
  // Resolve the type (handle type aliases)
  std::string typeName = resolveType(node.type.get());

  void* llvmType = nullptr;
  if (typeName == "int") {
    llvmType = (void*)nullptr;  // will be set in codegen
  } else if (typeName == "string") {
    llvmType = (void*)nullptr;  // will be set in codegen
  }

  Symbol sym{
      varName, /*typeName*/ typeName, node.isConst, false, false, nullptr,
      llvmType};
  if (!currentScope->insert(varName, sym)) {
    reportError("Variable '" + varName + "' already declared in this scope");
  }
  if (node.initializer) {
    node.initializer->accept(*this);
    // TODO: Type compatibility check
  }
}

void SemanticAnalyzerVisitor::visit(FunctionDecl& node) {
  std::string funcName = node.name.getLexeme();
  if (!currentScope->insert(funcName, Symbol{funcName, /*typeName*/ "", false,
                                             true, false, nullptr})) {
    reportError("Function '" + funcName + "' already declared in this scope");
  }
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
  if (!currentScope->lookup(idName)) {
    reportError("Use of undeclared identifier '" + idName + "'");
  }
}

void SemanticAnalyzerVisitor::visit(AssignmentExpr& node) {
  // For now, only support identifier assignments
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
  } else {
    // TODO: Support member access assignments
    reportError("Unsupported assignment target");
  }

  // Visit the target and value for completeness
  if (node.target) node.target->accept(*this);
  if (node.value) node.value->accept(*this);
}

void SemanticAnalyzerVisitor::visit(CallExpr& node) {
  std::string funcName;
  // Support identifier and member access callees
  if (auto id = dynamic_cast<IdentifierExpr*>(node.callee.get())) {
    funcName = id->name.getLexeme();
  } else if (auto member = dynamic_cast<MemberAccessExpr*>(node.callee.get())) {
    // Compose full name: e.g., console.log
    if (auto objIdent = dynamic_cast<IdentifierExpr*>(member->object.get())) {
      funcName = objIdent->name.getLexeme() + "." + member->member.getLexeme();
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
    // TODO: Check argument count and types
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

}  // namespace ast
