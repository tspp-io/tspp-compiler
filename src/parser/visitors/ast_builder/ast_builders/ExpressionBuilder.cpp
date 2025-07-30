#include "ExpressionBuilder.h"

#include <memory>

#include "core/common/macros.h"
#include "parser/nodes/expression_nodes.h"

namespace parser {

Shared(ast::Expr) ExpressionBuilder::build(tokens::TokenStream& stream) {
  return buildPrimary(stream);
}

Shared(ast::Expr) ExpressionBuilder::buildPrimary(tokens::TokenStream& stream) {
  auto type = stream.peek().getType();

  switch (type) {
    case tokens::TokenType::CHAR_LITERAL:
    case tokens::TokenType::STRING_LITERAL:
    case tokens::TokenType::NUMBER:
    case tokens::TokenType::TRUE:
    case tokens::TokenType::FALSE:
    case tokens::TokenType::IDENTIFIER: {
      auto expr = std::make_shared<ast::LiteralExpr>();
      expr->value = stream.peek();
      expr->location = stream.peek().getLocation();
      stream.advance();  // consume the token
      return expr;
    }
    default:
      // For unrecognized tokens, advance and return nullptr
      stream.advance();
      return nullptr;
  }
}

Shared(ast::Expr) ExpressionBuilder::buildBinary(tokens::TokenStream& stream,
                                                 int minPrec) {
  return nullptr;
}

Shared(ast::CallExpr) ExpressionBuilder::buildCall(tokens::TokenStream& stream,
                                                   Shared(ast::Expr) callee) {
  return nullptr;
}

int ExpressionBuilder::getOperatorPrecedence(tokens::TokenType type) {
  return 0;
}

bool ExpressionBuilder::isBinaryOperator(tokens::TokenType type) {
  return false;
}

}  // namespace parser
