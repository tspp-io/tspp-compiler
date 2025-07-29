#include "number_scanner.h"

#include "../../../tokens/TokenFactory.h"
#include "../../patterns/lexer_patterns.h"

namespace lexer {
namespace scanner {

tokens::Token NumberScanner::scan(const std::string& src, size_t& pos) {
  // Example: scan number using regex
  // TODO: Implement actual logic
  return tokens::TokenFactory::makeLiteralToken(tokens::TokenType::INT, "0", 1,
                                                1);
}

}  // namespace scanner
}  // namespace lexer
