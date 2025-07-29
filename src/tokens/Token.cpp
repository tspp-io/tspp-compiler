#include "Token.h"

#include <sstream>

#include "TokenUtils.h"

namespace tokens {

bool Token::isOperator() const {
  return TokenUtils::isOperator(type);
}

bool Token::isKeyword() const {
  return TokenUtils::isKeyword(lexeme);
}

std::string Token::toString() const {
  std::ostringstream oss;
  oss << "Token(" << TokenUtils::tokenTypeToString(type) << ", '" << lexeme
      << "', line=" << line << ", col=" << column << ")";
  return oss.str();
}

}  // namespace tokens
