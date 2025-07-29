#pragma once
#include <string>
#include <vector>

#include "../../tokens/Token.h"
#include "../state/lexer_state.h"
#include "specialized/identifier_scanner.h"
#include "specialized/number_scanner.h"
#include "specialized/operator_scanner.h"
#include "specialized/string_scanner.h"
namespace lexer {
namespace scanner {
class TokenScanner {
 public:
  TokenScanner();
  std::vector<tokens::Token> scanAll(const std::string& src);

 private:
  IdentifierScanner idScanner_;
  NumberScanner numScanner_;
  OperatorScanner opScanner_;
  StringScanner strScanner_;
  // ... add more as needed
};
}  // namespace scanner
}  // namespace lexer
