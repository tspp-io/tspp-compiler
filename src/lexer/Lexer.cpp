#include "Lexer.h"

#include "../common/DebugLog.h"

namespace lexer {
Lexer::Lexer(const std::string& source)
    : source_(source), pos_(0), line_(1), col_(1) {}

std::vector<tokens::Token> Lexer::tokenize() {
  tokens_.clear();
  errors_.clear();
  // TODO: Implement tokenization logic here
  common::DebugLog::info("Lexer::tokenize() called");
  return tokens_;
}
// Helper methods for lexing will be implemented here
}  // namespace lexer
