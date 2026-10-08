// Parser error reporting and recovery helpers shared by the AST builders.
//
// Every parse error goes to core::diagnostics() with the location of the
// offending token. After an error, the statement-list loops (top level,
// blocks, class bodies, switch cases) call synchronize() to skip to the next
// statement boundary, so several errors can be reported in one run.
#pragma once
#include <string>

#include "core/diagnostics/error_reporter.h"
#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "tokens/stream/token_stream.h"
#include "tokens/token_type.h"

namespace parser {

// The lexer inserts a ';' token with lexeme "\n" at line ends
// (LexerState::newLine).
inline bool isImplicitSemicolon(const tokens::Token& tok) {
  return tok.getType() == tokens::TokenType::SEMICOLON &&
         tok.getLexeme() == "\n";
}

// How a token is shown in a message: 'x', "end of line", "end of input".
inline std::string describeToken(const tokens::Token& tok) {
  if (tok.getType() == tokens::TokenType::END_OF_FILE) return "end of input";
  if (isImplicitSemicolon(tok)) return "end of line";
  return "'" + tok.getLexeme() + "'";
}

inline void parseError(const tokens::Token& at, const std::string& message) {
  core::diagnostics().error(at.getLocation(), message);
}

// "expected <what>, found <current token>" at the current token.
inline void expectedError(tokens::TokenStream& stream,
                          const std::string& what) {
  parseError(stream.peek(),
             "expected " + what + ", found " + describeToken(stream.peek()));
}

// Consumes a token of the given type, or reports "expected <what>" and
// leaves the stream where it is.
inline bool expect(tokens::TokenStream& stream, tokens::TokenType type,
                   const std::string& what) {
  if (stream.peek().getType() == type) {
    stream.advance();
    return true;
  }
  expectedError(stream, what);
  return false;
}

// True at a token that may follow a complete simple statement.
inline bool atStatementEnd(const tokens::TokenStream& stream) {
  auto t = stream.peek().getType();
  return t == tokens::TokenType::SEMICOLON ||
         t == tokens::TokenType::RIGHT_BRACE ||
         t == tokens::TokenType::END_OF_FILE || stream.isAtEnd();
}

// Skips to the next statement boundary: just past a ';' at this nesting
// level, or before a '}' that closes the enclosing block, or before a
// keyword that starts a declaration. Nested (), [] and {} are skipped whole.
inline void synchronize(tokens::TokenStream& stream) {
  int depth = 0;
  bool first = true;
  while (!stream.isAtEnd() &&
         stream.peek().getType() != tokens::TokenType::END_OF_FILE) {
    auto t = stream.peek().getType();
    if (depth == 0) {
      if (t == tokens::TokenType::SEMICOLON) {
        stream.advance();
        return;
      }
      if (t == tokens::TokenType::RIGHT_BRACE) return;
      if (!first) {
        switch (t) {
          case tokens::TokenType::FUNCTION:
          case tokens::TokenType::CLASS:
          case tokens::TokenType::INTERFACE:
          case tokens::TokenType::LET:
          case tokens::TokenType::CONST:
          case tokens::TokenType::IF:
          case tokens::TokenType::WHILE:
          case tokens::TokenType::FOR:
          case tokens::TokenType::RETURN:
          case tokens::TokenType::IMPORT:
            return;
          default:
            break;
        }
      }
    }
    if (t == tokens::TokenType::LEFT_BRACE ||
        t == tokens::TokenType::LEFT_PAREN ||
        t == tokens::TokenType::LEFT_BRACKET) {
      ++depth;
    } else if (t == tokens::TokenType::RIGHT_BRACE ||
               t == tokens::TokenType::RIGHT_PAREN ||
               t == tokens::TokenType::RIGHT_BRACKET) {
      if (depth > 0) --depth;
      // A '}' that closed a skipped block ends the bad statement too.
      if (depth == 0 && t == tokens::TokenType::RIGHT_BRACE) {
        stream.advance();
        return;
      }
    }
    stream.advance();
    first = false;
  }
}

// enum / namespace: report "not supported yet" and skip the declaration
// (keyword, name, and a balanced { ... } body).
inline void skipUnsupportedDeclaration(tokens::TokenStream& stream) {
  auto kw = stream.peek();
  parseError(kw, "'" + kw.getLexeme() +
                     "' declarations are not supported yet");
  stream.advance();
  while (!stream.isAtEnd() &&
         stream.peek().getType() != tokens::TokenType::LEFT_BRACE &&
         stream.peek().getType() != tokens::TokenType::SEMICOLON &&
         stream.peek().getType() != tokens::TokenType::END_OF_FILE) {
    stream.advance();
  }
  if (stream.peek().getType() == tokens::TokenType::LEFT_BRACE) {
    int depth = 0;
    do {
      auto t = stream.peek().getType();
      if (t == tokens::TokenType::LEFT_BRACE) ++depth;
      if (t == tokens::TokenType::RIGHT_BRACE) --depth;
      stream.advance();
    } while (depth > 0 && !stream.isAtEnd() &&
             stream.peek().getType() != tokens::TokenType::END_OF_FILE);
  }
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) stream.advance();
}

// A statement that must be followed by ';', a line end, '}' or end of input.
inline bool needsTerminator(const ast::BaseNode* node) {
  return dynamic_cast<const ast::ExprStmt*>(node) ||
         dynamic_cast<const ast::VarDecl*>(node) ||
         dynamic_cast<const ast::ReturnStmt*>(node) ||
         dynamic_cast<const ast::BreakStmt*>(node) ||
         dynamic_cast<const ast::ContinueStmt*>(node) ||
         dynamic_cast<const ast::ThrowStmt*>(node);
}

// Call after a statement-list loop built one item, starting at token index
// `startPos` with `errorsBefore` errors reported so far. Reports what the
// builder did not, and resynchronizes after any error. Guarantees progress.
inline void finishListItem(tokens::TokenStream& stream,
                           const ast::BaseNode* node, size_t startPos,
                           int errorsBefore, const char* what) {
  auto& diag = core::diagnostics();
  if (!node) {
    if (diag.errorCount() == errorsBefore) {
      if (stream.getCurrentPosition() == startPos)
        expectedError(stream, what);
      else
        parseError(stream.peek(),
                   "unexpected " + describeToken(stream.peek()));
    }
    synchronize(stream);
  } else if (diag.errorCount() != errorsBefore) {
    // Compound statements and declarations recover inside their bodies.
    if (needsTerminator(node) && !atStatementEnd(stream)) synchronize(stream);
  } else if (needsTerminator(node) && !atStatementEnd(stream)) {
    auto prev = stream.previous().getType();
    // The builders consume an optional ';'. A statement ending in '}'
    // (e.g. an object literal) gets no implicit ';' from the lexer.
    if (prev != tokens::TokenType::SEMICOLON &&
        prev != tokens::TokenType::RIGHT_BRACE) {
      expectedError(stream, "';' after statement");
      synchronize(stream);
    }
  }
  if (stream.getCurrentPosition() == startPos && !stream.isAtEnd() &&
      stream.peek().getType() != tokens::TokenType::RIGHT_BRACE) {
    stream.advance();
  }
}

}  // namespace parser
