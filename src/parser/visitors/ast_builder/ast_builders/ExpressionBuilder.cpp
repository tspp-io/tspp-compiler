#include "ExpressionBuilder.h"

#include <memory>

#include "core/common/macros.h"

namespace parser {

Shared(ast::Expr) ExpressionBuilder::build(tokens::TokenStream& stream) {
  return nullptr;
}

Shared(ast::Expr) ExpressionBuilder::buildPrimary(tokens::TokenStream& stream) {
  return nullptr;
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
