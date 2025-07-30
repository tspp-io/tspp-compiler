/*****************************************************************************
 * File: lexer.h
 * Description: Main lexer interface for TSPP language
 *****************************************************************************/

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "scanner/token_scanner.h"
#include "state/lexer_state.h"

namespace lexer {

class Lexer {
 public:
  explicit Lexer(std::string source, std::string filename = "");

  // Process source and return all tokens
  std::vector<tokens::Token> tokenize();

  // Get any lexical errors that occurred
  const std::vector<std::string> &getErrors() const {
    return errors_;
  }

  // Check if lexical analysis encountered errors
  bool hasErrors() const {
    return !errors_.empty();
  }

 private:
  std::shared_ptr<LexerState> state_;
  TokenScanner scanner_;
  std::vector<std::string> errors_;

  // Add: clear separation of token categories for maintainability
  // - Storage/memory qualifiers: see TokenType::STORAGE_BEGIN/STORAGE_END
  // - Smart pointer modifiers: see TokenType::SMARTPTR_BEGIN/SMARTPTR_END
  // - Pointer type modifiers: see TokenType::PTRMOD_BEGIN/PTRMOD_END
  // - Function/method modifiers: see TokenType::FUNC_MOD_BEGIN/FUNC_MOD_END
  // - Class modifiers: see TokenType::CLASS_MOD_BEGIN/CLASS_MOD_END

  void addError(const tokens::Token &token);
};

}  // namespace lexer