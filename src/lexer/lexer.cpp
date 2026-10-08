/*****************************************************************************
 * File: lexer.cpp
 * Description: Implementation of main lexer for TSPP language
 *****************************************************************************/

#include "lexer.h"

#include <cctype>

#include "core/diagnostics/error_reporter.h"

namespace lexer {

Lexer::Lexer(std::string source, std::string filename)
    : state_(
          std::make_shared<LexerState>(std::move(source), std::move(filename))),
      scanner_(state_) {}

std::vector<tokens::Token> Lexer::tokenize() {
  while (!state_->isAtEnd()) {
    tokens::Token token = scanner_.scanToken();

    if (token.isError()) {
      addError(token);
      continue;
    }

    state_->addToken(token);

    if (token.getType() == tokens::TokenType::END_OF_FILE) {
      break;
    }
  }

  return state_->getTokens();
}

void Lexer::addError(const tokens::Token &token) {
  std::string message = token.getErrorMessage().value_or("invalid token");
  // Scanner messages are capitalized sentences ("Invalid operator").
  if (!message.empty()) message[0] = static_cast<char>(std::tolower(message[0]));
  if (!token.getLexeme().empty() && message == "invalid operator") {
    message = "unexpected character '" + token.getLexeme() + "'";
  }
  errors_.push_back(message);
  core::diagnostics().error(token.getLocation(), message);
}

} // namespace lexer