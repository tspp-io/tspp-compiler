#pragma once
#include "core/common/macros.h"
#include "parser/nodes/expression_nodes.h"
#include "tokens/stream/token_stream.h"

namespace parser {

/**
 * @brief Builder for expression AST nodes
 * Handles parsing of binary, unary, literal, identifier expressions
 */
class ExpressionBuilder {
 public:
  /**
   * @brief Build any expression from token stream
   * @param stream Token stream to parse from
   * @return Shared pointer to expression node, or nullptr on error
   */
  static Shared(ast::Expr) build(tokens::TokenStream& stream);

  /**
   * @brief Build primary expression (literals, identifiers, grouping)
   * @param stream Token stream to parse from
   * @return Shared pointer to expression node, or nullptr on error
   */
  static Shared(ast::Expr) buildPrimary(tokens::TokenStream& stream);

  /**
   * @brief Build primary expression followed by postfix '++' / '--'
   * @param stream Token stream to parse from
   * @return Shared pointer to expression node, or nullptr on error
   */
  static Shared(ast::Expr) buildPostfix(tokens::TokenStream& stream);

  /**
   * @brief Build binary expression with precedence
   * @param stream Token stream to parse from
   * @param minPrec Minimum precedence level
   * @return Shared pointer to expression node, or nullptr on error
   */
  static Shared(ast::Expr)
      buildBinary(tokens::TokenStream& stream, int minPrec = 0);

  /**
   * @brief Build call expression
   * @param stream Token stream to parse from
   * @param callee Expression being called
   * @return Shared pointer to call expression, or nullptr on error
   */
  static Shared(ast::CallExpr)
      buildCall(tokens::TokenStream& stream, Shared(ast::Expr) callee);

  /**
   * @brief Get operator precedence
   * @param type Token type of operator
   * @return Precedence level (higher = tighter binding)
   */
  static int getOperatorPrecedence(tokens::TokenType type);

  // Binary precedence levels (higher = tighter), TypeScript/C order.
  static constexpr int kPrecAssignment = 1;  // = += -= ... (right-assoc)
  static constexpr int kPrecConditional = 2;  // ?: (right-assoc)
  static constexpr int kPrecLogicalOr = 3;    // ||
  static constexpr int kPrecLogicalAnd = 4;   // &&
  static constexpr int kPrecBitOr = 5;        // |
  static constexpr int kPrecBitXor = 6;       // ^
  static constexpr int kPrecBitAnd = 7;       // &
  static constexpr int kPrecEquality = 8;     // == !=
  static constexpr int kPrecRelational = 9;   // < <= > >=
  static constexpr int kPrecShift = 10;       // << >>
  static constexpr int kPrecAdditive = 11;    // + -
  static constexpr int kPrecMultiplicative = 12;  // * / %

  /**
   * @brief Check if token is a binary operator
   * @param type Token type to check
   * @return true if binary operator, false otherwise
   */
  static bool isBinaryOperator(tokens::TokenType type);

  /**
   * @brief Check if token is a compound assignment operator (+=, -=, ...)
   * @param type Token type to check
   * @return true if compound assignment operator, false otherwise
   */
  static bool isCompoundAssignmentOperator(tokens::TokenType type);
};

}  // namespace parser
