#pragma once
#include <any>
#include <string>

#include "../core/Location.h"
#include "TokenType.h"
/**
 * Token - Represents a lexical token in TSPP++.
 */
namespace tokens {

struct Token {
  TokenType type;
  std::string lexeme;
  int line;
  int column;
  std::any literalValue;
  common::Location location() const {
    return common::Location(line, column);
  }

  Token(TokenType t, std::string l, int ln, int col, std::any val = {})
      : type(t),
        lexeme(std::move(l)),
        line(ln),
        column(col),
        literalValue(std::move(val)) {}

  bool isOperator() const {
    return TokenUtils::isOperator(type);
  }
  bool isKeyword() const {
    return TokenUtils::isKeyword(lexeme);
  }
  std::string toString() const {
    return "Token(" + TokenUtils::tokenTypeToString(type) + ", '" + lexeme +
           "', line=" + std::to_string(line) +
           ", col=" + std::to_string(column) + ")";
  }
};

}  // namespace tokens
