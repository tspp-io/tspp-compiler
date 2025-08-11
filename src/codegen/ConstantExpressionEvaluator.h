#pragma once

#include <memory>
#include <unordered_map>
#include <variant>

#include "parser/nodes/ast_visitor.h"
#include "parser/nodes/base_node.h"
#include "parser/nodes/expression_nodes.h"

namespace ast {

// Value type for constant expression evaluation
struct ConstantValue {
  enum Type { INT, BOOL, FLOAT, STRING, NULL_PTR };
  Type type;
  std::variant<int, bool, float, std::string, void*> value;

  ConstantValue() : type(INT), value(0) {}  // Default constructor
  ConstantValue(int val) : type(INT), value(val) {}
  ConstantValue(bool val) : type(BOOL), value(val) {}
  ConstantValue(float val) : type(FLOAT), value(val) {}
  ConstantValue(const std::string& val) : type(STRING), value(val) {}
  ConstantValue(void* ptr) : type(NULL_PTR), value(ptr) {}

  // Constructor for null pointer
  static ConstantValue nullPtr() {
    ConstantValue val;
    val.type = NULL_PTR;
    val.value = static_cast<void*>(nullptr);
    return val;
  }

  int asInt() const {
    return std::get<int>(value);
  }
  bool asBool() const {
    return std::get<bool>(value);
  }
  float asFloat() const {
    return std::get<float>(value);
  }
  std::string asString() const {
    return std::get<std::string>(value);
  }
  void* asPointer() const {
    return std::get<void*>(value);
  }
  bool isNull() const {
    return type == NULL_PTR;
  }
};

class ConstantExpressionEvaluator : public ASTVisitor {
 private:
  std::unordered_map<std::string, ConstantValue> constantMap;
  std::unique_ptr<ConstantValue> result;
  bool evaluationSucceeded;

 public:
  ConstantExpressionEvaluator();

  // Add a constant to the evaluator's context
  void addConstant(const std::string& name, const ConstantValue& value);

  // Try to evaluate an expression as a constant
  // Returns nullptr if the expression is not constant
  std::unique_ptr<ConstantValue> evaluate(Expr* expr);

  // Check if evaluation succeeded
  bool succeeded() const {
    return evaluationSucceeded;
  }

  // Visitor methods
  void visit(LiteralExpr& node) override;
  void visit(IdentifierExpr& node) override;
  void visit(BinaryExpr& node) override;
  void visit(UnaryExpr& node) override;
  void visit(MemberAccessExpr& node) override;
  void visit(GroupingExpr& node) override;
  void visit(ThisExpr& node) override;
  void visit(NullExpr& node) override;

  // Non-constant expressions - these fail evaluation
  void visit(CallExpr& node) override;
  void visit(AssignmentExpr& node) override;

  // Statement visitors (no-op for constant evaluation)
  void visit(BlockStmt& node) override {}
  void visit(ExprStmt& node) override {}
  void visit(IfStmt& node) override {}
  void visit(WhileStmt& node) override {}
  void visit(ForStmt& node) override {}
  void visit(ReturnStmt& node) override {}

  // Declaration visitors (no-op for constant evaluation)
  void visit(VarDecl& node) override {}
  void visit(Parameter& node) override {}
  void visit(FunctionDecl& node) override {}
  void visit(ClassDecl& node) override {}
  void visit(InterfaceDecl& node) override {}
  void visit(TypeAliasDecl& node) override {}
  void visit(ImportDecl& node) override {}

  // Program node visitor (no-op for constant evaluation)
  void visit(ProgramNode& node) override {}

  // Type node visitors (no-op for constant evaluation)
  void visit(ast::BasicTypeNode&) override {}
  void visit(ast::PointerTypeNode&) override {}
  void visit(ast::SmartPointerTypeNode&) override {}
  void visit(ast::UnionTypeNode&) override {}
  void visit(ast::TypeConstraintNode&) override {}
};

}  // namespace ast
