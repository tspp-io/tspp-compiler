#include "ConstantExpressionEvaluator.h"

#include <iostream>

#include "tokens/token_type.h"

namespace ast {

ConstantExpressionEvaluator::ConstantExpressionEvaluator()
    : evaluationSucceeded(false) {}

void ConstantExpressionEvaluator::addConstant(const std::string& name,
                                              const ConstantValue& value) {
  constantMap[name] = value;
}

std::unique_ptr<ConstantValue> ConstantExpressionEvaluator::evaluate(
    Expr* expr) {
  if (!expr) return nullptr;

  result = nullptr;
  evaluationSucceeded = false;

  expr->accept(*this);

  if (evaluationSucceeded && result) {
    return std::move(result);
  }

  return nullptr;
}

void ConstantExpressionEvaluator::visit(LiteralExpr& node) {
  using tokens::TokenType;
  TokenType type = node.value.getType();
  std::string lexeme = node.value.getLexeme();

  try {
    if (type == TokenType::TRUE) {
      result = std::make_unique<ConstantValue>(true);
      evaluationSucceeded = true;
    } else if (type == TokenType::FALSE) {
      result = std::make_unique<ConstantValue>(false);
      evaluationSucceeded = true;
    } else if (type == TokenType::STRING_LITERAL) {
      result = std::make_unique<ConstantValue>(lexeme);
      evaluationSucceeded = true;
    } else if (type == TokenType::NUMBER) {
      // For now, assume all numbers are integers
      // TODO: Better handling of float vs int
      int intVal = std::stoi(lexeme);
      result = std::make_unique<ConstantValue>(intVal);
      evaluationSucceeded = true;
    }
  } catch (const std::exception& e) {
    evaluationSucceeded = false;
    result = nullptr;
  }
}

void ConstantExpressionEvaluator::visit(IdentifierExpr& node) {
  std::string name = node.name.getLexeme();
  auto it = constantMap.find(name);
  if (it != constantMap.end()) {
    result = std::make_unique<ConstantValue>(it->second);
    evaluationSucceeded = true;
  } else {
    evaluationSucceeded = false;
  }
}

void ConstantExpressionEvaluator::visit(BinaryExpr& node) {
  using tokens::TokenType;
  TokenType op = node.op.getType();

  // Handle logical operators with short-circuit evaluation
  if (op == TokenType::AMPERSAND_AMPERSAND || op == TokenType::PIPE_PIPE) {
    auto leftResult = evaluate(node.left.get());
    if (!leftResult || leftResult->type != ConstantValue::BOOL) {
      evaluationSucceeded = false;
      return;
    }

    bool leftBool = leftResult->asBool();

    // Short-circuit evaluation
    if (op == TokenType::AMPERSAND_AMPERSAND && !leftBool) {
      result = std::make_unique<ConstantValue>(false);
      evaluationSucceeded = true;
      return;
    } else if (op == TokenType::PIPE_PIPE && leftBool) {
      result = std::make_unique<ConstantValue>(true);
      evaluationSucceeded = true;
      return;
    }

    // Evaluate right operand
    auto rightResult = evaluate(node.right.get());
    if (!rightResult || rightResult->type != ConstantValue::BOOL) {
      evaluationSucceeded = false;
      return;
    }

    bool rightBool = rightResult->asBool();

    if (op == TokenType::AMPERSAND_AMPERSAND) {
      result = std::make_unique<ConstantValue>(leftBool && rightBool);
      evaluationSucceeded = true;
    } else if (op == TokenType::PIPE_PIPE) {
      result = std::make_unique<ConstantValue>(leftBool || rightBool);
      evaluationSucceeded = true;
    } else {
      evaluationSucceeded = false;
    }
    return;
  }

  // For other binary operators, evaluate both sides
  auto leftResult = evaluate(node.left.get());
  if (!leftResult) {
    evaluationSucceeded = false;
    return;
  }

  auto rightResult = evaluate(node.right.get());
  if (!rightResult) {
    evaluationSucceeded = false;
    return;
  }

  using tokens::TokenType;
  // Both operands must be the same type for most operations
  if (leftResult->type != rightResult->type) {
    evaluationSucceeded = false;
    return;
  }

  try {
    if (leftResult->type == ConstantValue::INT) {
      int left = leftResult->asInt();
      int right = rightResult->asInt();

      switch (op) {
        case TokenType::PLUS:
          result = std::make_unique<ConstantValue>(left + right);
          break;
        case TokenType::MINUS:
          result = std::make_unique<ConstantValue>(left - right);
          break;
        case TokenType::STAR:
          result = std::make_unique<ConstantValue>(left * right);
          break;
        case TokenType::SLASH:
          if (right == 0) {
            evaluationSucceeded = false;
            return;
          }
          result = std::make_unique<ConstantValue>(left / right);
          break;
        case TokenType::PERCENT:
          if (right == 0) {
            evaluationSucceeded = false;
            return;
          }
          result = std::make_unique<ConstantValue>(left % right);
          break;
        case TokenType::LESS:
          result = std::make_unique<ConstantValue>(left < right);
          break;
        case TokenType::GREATER:
          result = std::make_unique<ConstantValue>(left > right);
          break;
        case TokenType::LESS_EQUALS:
          result = std::make_unique<ConstantValue>(left <= right);
          break;
        case TokenType::GREATER_EQUALS:
          result = std::make_unique<ConstantValue>(left >= right);
          break;
        case TokenType::EQUALS_EQUALS:
          result = std::make_unique<ConstantValue>(left == right);
          break;
        case TokenType::EXCLAIM_EQUALS:
          result = std::make_unique<ConstantValue>(left != right);
          break;
        default:
          evaluationSucceeded = false;
          return;
      }
      evaluationSucceeded = true;
    } else if (leftResult->type == ConstantValue::BOOL) {
      bool left = leftResult->asBool();
      bool right = rightResult->asBool();

      switch (op) {
        case TokenType::EQUALS_EQUALS:
          result = std::make_unique<ConstantValue>(left == right);
          break;
        case TokenType::EXCLAIM_EQUALS:
          result = std::make_unique<ConstantValue>(left != right);
          break;
        case TokenType::AMPERSAND_AMPERSAND:
          result = std::make_unique<ConstantValue>(left && right);
          break;
        case TokenType::PIPE_PIPE:
          result = std::make_unique<ConstantValue>(left || right);
          break;
        default:
          evaluationSucceeded = false;
          return;
      }
      evaluationSucceeded = true;
    } else {
      evaluationSucceeded = false;
    }
  } catch (const std::exception& e) {
    evaluationSucceeded = false;
  }
}

void ConstantExpressionEvaluator::visit(UnaryExpr& node) {
  auto operandResult = evaluate(node.operand.get());
  if (!operandResult) {
    evaluationSucceeded = false;
    return;
  }

  using tokens::TokenType;
  TokenType op = node.op.getType();

  try {
    if (operandResult->type == ConstantValue::INT && op == TokenType::MINUS) {
      result = std::make_unique<ConstantValue>(-operandResult->asInt());
      evaluationSucceeded = true;
    } else if (operandResult->type == ConstantValue::BOOL &&
               op == TokenType::EXCLAIM) {
      result = std::make_unique<ConstantValue>(!operandResult->asBool());
      evaluationSucceeded = true;
    } else {
      evaluationSucceeded = false;
    }
  } catch (const std::exception& e) {
    evaluationSucceeded = false;
  }
}

void ConstantExpressionEvaluator::visit(CallExpr& node) {
  evaluationSucceeded = false;
}

void ConstantExpressionEvaluator::visit(AssignmentExpr& node) {
  evaluationSucceeded = false;
}

}  // namespace ast
