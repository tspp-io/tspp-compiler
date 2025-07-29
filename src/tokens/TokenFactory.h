#pragma once
#include "Token.h"
/**
 * TokenFactory - Helpers for constructing tokens.
 */
namespace tokens {
namespace TokenFactory {
inline Token makeIdentifierToken(const std::string &name, int line, int col) {
  return Token(TokenType::IDENTIFIER, name, line, col);
}
inline Token makeLiteralToken(TokenType type, const std::string &val, int line,
                              int col) {
  return Token(type, val, line, col, val);
}
inline Token makeOperatorToken(const std::string &symbol, int line, int col) {
  TokenType type = TokenUtils::keywordToToken(symbol);
  if (!TokenUtils::isOperator(type)) type = TokenType::ERROR_TOKEN;
  return Token(type, symbol, line, col);
}
inline Token makeKeywordToken(const std::string &word, int line, int col) {
  TokenType type = TokenUtils::keywordToToken(word);
  if (!TokenUtils::isKeyword(word)) type = TokenType::IDENTIFIER;
  return Token(type, word, line, col);
}
}  // namespace TokenFactory
}  // namespace tokens
