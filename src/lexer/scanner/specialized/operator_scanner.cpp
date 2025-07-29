#include "operator_scanner.h"

#include "../../../tokens/TokenFactory.h"
#include "../../patterns/token_maps.h"

namespace lexer {
namespace scanner {

tokens::Token OperatorScanner::scan(const std::string& src, size_t& pos) {
  // Example: scan operator using map
  // TODO: Implement actual logic
  return tokens::TokenFactory::makeOperatorToken("+", 1, 1);
}

}  // namespace scanner
}  // namespace lexer
