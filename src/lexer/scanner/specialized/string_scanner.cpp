#include "string_scanner.h"

#include "../../../tokens/TokenFactory.h"
#include "../../patterns/lexer_patterns.h"

namespace lexer {
namespace scanner {

tokens::Token StringScanner::scan(const std::string& src, size_t& pos) {
  // Example: scan string using regex
  // TODO: Implement actual logic
  return tokens::TokenFactory::makeLiteralToken(tokens::TokenType::STRING,
                                                "\"example\"", 1, 1);
}

}  // namespace scanner
}  // namespace lexer
