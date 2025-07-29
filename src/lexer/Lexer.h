#pragma once
#include <string>
#include <vector>

#include "../common/Error.h"
#include "../common/Location.h"
#include "../tokens/Token.h"
#include "../tokens/TokenFactory.h"
#include "../tokens/TokenType.h"
#include "../tokens/TokenUtils.h"

namespace lexer {
/**
 * Lexer - Tokenizes TSPP++ source code into tokens.
 */
class Lexer {
 public:
  Lexer(const std::string& source);
  std::vector<tokens::Token> tokenize();
  const std::vector<common::Error>& errors() const {
    return errors_;
  }

 private:
  std::string source_;
  size_t pos_ = 0;
  int line_ = 1;
  int col_ = 1;
  std::vector<tokens::Token> tokens_;
  std::vector<common::Error> errors_;
  // Helper methods for lexing will be added here
};
}  // namespace lexer
