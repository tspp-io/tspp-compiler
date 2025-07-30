#include <sstream>

#include "token_type.h"
#include "tokens/TokenUtils.h"
#include "tokens/tokens.h"

namespace tokens {

std::string Token::toString() const {
  std::stringstream ss;
  ss << "Token(" << TokenUtils::tokenTypeToString(type_) << ", '" << lexeme_
     << "', " << location_.toString();
  if (errorMessage_) {
    ss << ", error: " << *errorMessage_;
  }
  ss << ")";
  return ss.str();
}

}  // namespace tokens
