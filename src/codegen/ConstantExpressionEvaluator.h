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
  enum Type { INT, BOOL, FLOAT, STRING };
  Type type;
  std::variant<int, bool, float, std::string> value;

  ConstantValue() : type(INT), value(0) {}  // Default constructor
  ConstantValue(int val) : type(INT), value(val) {}
  ConstantValue(bool val) : type(BOOL), value(val) {}
  ConstantValue(float val) : type(FLOAT), value(val) {}
  ConstantValue(const std::string& val) : type(STRING), value(val) {}

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

  // Non-constant expressions - these fail evaluation
  void visit(CallExpr& node) override;
  void visit(AssignmentExpr& node) override;
};

}  // namespace ast
