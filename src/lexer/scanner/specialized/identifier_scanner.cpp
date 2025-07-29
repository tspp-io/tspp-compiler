#include "identifier_scanner.h"

#include "../../../tokens/TokenFactory.h"
#include "../../patterns/lexer_patterns.h"

namespace lexer {
namespace scanner {

tokens::Token IdentifierScanner::scan(const std::string& src, size_t& pos) {
  // Example: scan identifier using regex
  // TODO: Implement actual logic
  return tokens::TokenFactory::makeIdentifierToken("example", 1, 1);
}

}  // namespace scanner
}  // namespace lexer
