/*****************************************************************************
 * File: lexer_state.cpp
 * Description: Implementation of LexerState class providing source code
 *navigation and token collection functionality.
 *****************************************************************************/

#include "lexer_state.h"

#include "core/common/macros.h"

namespace lexer {

namespace {
using tokens::TokenType;

// A line ending in one of these continues on the next line, so no implicit
// ';' is inserted after it (e.g. `f(a,` or `x = a +`).
bool continuesAfter(TokenType t) {
  switch (t) {
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::STAR:
    case TokenType::SLASH:
    case TokenType::PERCENT:
    case TokenType::AMPERSAND:
    case TokenType::PIPE:
    case TokenType::CARET:
    case TokenType::TILDE:
    case TokenType::LEFT_SHIFT:
    case TokenType::EXCLAIM:
    case TokenType::AMPERSAND_AMPERSAND:
    case TokenType::PIPE_PIPE:
    case TokenType::EQUALS:
    case TokenType::EQUALS_EQUALS:
    case TokenType::EXCLAIM_EQUALS:
    case TokenType::LESS_EQUALS:
    case TokenType::GREATER_EQUALS:
    case TokenType::PLUS_EQUALS:
    case TokenType::MINUS_EQUALS:
    case TokenType::STAR_EQUALS:
    case TokenType::SLASH_EQUALS:
    case TokenType::PERCENT_EQUALS:
    case TokenType::AMPERSAND_EQUALS:
    case TokenType::PIPE_EQUALS:
    case TokenType::CARET_EQUALS:
    case TokenType::QUESTION:
    case TokenType::ARROW:
    case TokenType::DOT:
    case TokenType::AT:
    case TokenType::LEFT_PAREN:
    case TokenType::LEFT_BRACKET:
    case TokenType::COMMA:
      return true;
    default:
      return false;
  }
}

// A line starting with one of these continues the previous line, so the
// implicit ';' before it is dropped (e.g. a closing `)` or `.method()`).
bool continuesBefore(TokenType t) {
  switch (t) {
    case TokenType::RIGHT_PAREN:
    case TokenType::RIGHT_BRACKET:
    case TokenType::COMMA:
    case TokenType::DOT:
    case TokenType::QUESTION:
    case TokenType::COLON:
    case TokenType::PLUS:
    case TokenType::MINUS:
    case TokenType::SLASH:
    case TokenType::PERCENT:
    case TokenType::AMPERSAND:
    case TokenType::PIPE:
    case TokenType::CARET:
    case TokenType::AMPERSAND_AMPERSAND:
    case TokenType::PIPE_PIPE:
    case TokenType::EQUALS:
    case TokenType::EQUALS_EQUALS:
    case TokenType::EXCLAIM_EQUALS:
    case TokenType::LESS_EQUALS:
    case TokenType::GREATER_EQUALS:
    case TokenType::PLUS_EQUALS:
    case TokenType::MINUS_EQUALS:
    case TokenType::STAR_EQUALS:
    case TokenType::SLASH_EQUALS:
    case TokenType::PERCENT_EQUALS:
    case TokenType::AMPERSAND_EQUALS:
    case TokenType::PIPE_EQUALS:
    case TokenType::CARET_EQUALS:
    case TokenType::ARROW:
      return true;
    default:
      return false;
  }
}

bool isImplicitSemicolon(const tokens::Token &tok) {
  return tok.getType() == TokenType::SEMICOLON && tok.getLexeme() == "\n";
}
}  // namespace

/*****************************************************************************
 * Source Inspection Methods
 *****************************************************************************/

std::string_view LexerState::getCurrentLexeme() const {
  if (position_ >= source_.length()) {
    return std::string_view();
  }
  return std::string_view(source_).substr(position_);
}

char LexerState::getCurrentChar() const {
  // position_ is unsigned, so position_ < 0 is always false
  if (position_ >= source_.length()) {
    return '\0';
  }
  return source_[position_];
}

char LexerState::peekNext(int n) const {
  // Handle negative lookhead
  if (n < 0) {
    return getCurrentChar();
  }

  // Calculate peek position and bounds check
  size_t peek_pos = position_ + n;
  if (peek_pos >= source_.length()) {
    return '\0';
  }
  return source_[peek_pos];
}

bool LexerState::match(char expected) {
  if (isAtEnd() || getCurrentChar() != expected) {
    return false;
  }
  advance();
  return true;
}

std::string_view LexerState::getLineContent() const {
  // Find start of current line
  size_t lineStart = source_.rfind('\n', position_);
  if (lineStart == std::string::npos) {
    lineStart = 0;
  } else {
    lineStart++;  // Skip newline character
  }

  // Find end of current line
  size_t lineEnd = source_.find('\n', position_);
  if (lineEnd == std::string::npos) {
    lineEnd = source_.length();
  }

  return std::string_view(source_).substr(lineStart, lineEnd - lineStart);
}

/*****************************************************************************
 * State Management Methods
 *****************************************************************************/

void LexerState::advance(size_t count) {
  if (count == 0) return;

  // Advance position and track columns, handling tabs
  for (size_t i = 0; i < count && !isAtEnd(); ++i) {
    if (getCurrentChar() == '\t') {
      column_ += 4;  // Use 4 spaces per tab
    } else {
      column_++;
    }
    position_++;
  }
}

void LexerState::newLine() {
  if (position_ >= source_.length()) {
    return;
  }

  // The implicit semicolon sits at the end of the line it terminates.
  core::Location eol(line_, column_, fileName_);

  // Update line tracking
  line_++;
  column_ = 1;
  position_++;

  // Add implicit semicolon if needed (automatic semicolon insertion)
  if (!tokens_.empty()) {
    const auto &lastToken = tokens_.back();
    if (lastToken.getType() != tokens::TokenType::SEMICOLON &&
        lastToken.getType() != tokens::TokenType::LEFT_BRACE &&
        lastToken.getType() != tokens::TokenType::RIGHT_BRACE &&
        !continuesAfter(lastToken.getType())) {
      // Lexeme "\n" marks it as implicit (parser::isImplicitSemicolon)
      addToken(tokens::Token(tokens::TokenType::SEMICOLON, "\n", eol));
    }
  }
}

void LexerState::addToken(tokens::Token token) {
  if (continuesBefore(token.getType()) && !tokens_.empty() &&
      isImplicitSemicolon(tokens_.back())) {
    tokens_.pop_back();
  }
  ASSERT(token.getLocation().getLine() <= line_);
  ASSERT(token.getLocation().getLine() < line_ ||
         token.getLocation().getColumn() <= column_);

  tokens_.push_back(std::move(token));
}

void LexerState::reset() {
  position_ = 0;
  line_ = 1;
  column_ = 1;
  tokens_.clear();
}

/*****************************************************************************
 * State Query Methods
 *****************************************************************************/

const tokens::Token &LexerState::getLastToken() const {
  ASSERT_MSG(!tokens_.empty(), "Cannot get last token from empty collection");
  return tokens_.back();
}

}  // namespace lexer