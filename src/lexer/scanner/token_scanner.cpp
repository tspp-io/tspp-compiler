#include "token_scanner.h"

namespace lexer {
namespace scanner {

TokenScanner::TokenScanner() {}

std::vector<tokens::Token> TokenScanner::scanAll(const std::string& src) {
  // TODO: Implement logic to use specialized scanners
  std::vector<tokens::Token> tokens;
  // Example: just call identifier scanner for now
  size_t pos = 0;
  tokens.push_back(idScanner_.scan(src, pos));
  return tokens;
}

}  // namespace scanner
}  // namespace lexer
