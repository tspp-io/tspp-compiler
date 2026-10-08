#include "ast_builder_visitor.h"

#include <iostream>

#include "ast_builders/DeclarationBuilder.h"
#include "ast_builders/ParseDiagnostics.h"
#include "ast_builders/StatementBuilder.h"
#include "tokens/token_type.h"

namespace parser {

ASTBuilderVisitor::ASTBuilderVisitor(tokens::TokenStream& stream)
    : stream_(stream) {}

void ASTBuilderVisitor::visit(ast::ProgramNode& node) {
  parseTopLevel(node);
}

void ASTBuilderVisitor::parseTopLevel(ast::ProgramNode& program) {
  while (!stream_.isAtEnd()) {
    auto type = stream_.peek().getType();
    if (type == tokens::TokenType::END_OF_FILE) break;

    // Empty statements (explicit or inserted at line ends)
    if (type == tokens::TokenType::SEMICOLON) {
      stream_.advance();
      continue;
    }

    if (type == tokens::TokenType::ENUM ||
        type == tokens::TokenType::NAMESPACE) {
      skipUnsupportedDeclaration(stream_);
      continue;
    }

    if (type == tokens::TokenType::RIGHT_BRACE) {
      parseError(stream_.peek(), "unexpected '}'");
      stream_.advance();
      continue;
    }

    const tokens::Token startTok = stream_.peek();
    const size_t startPos = stream_.getCurrentPosition();
    const int errorsBefore = core::diagnostics().errorCount();
    Shared(ast::BaseNode) node;

    if (type == tokens::TokenType::IMPORT) {
      auto importDecl = DeclarationBuilder::buildImport(stream_);
      if (importDecl) program.imports.push_back(importDecl);
      node = importDecl;
    } else if (isDeclarationStart()) {
      node = DeclarationBuilder::build(stream_);
      if (node) {
        if (!node->location.isValid()) node->location = startTok.getLocation();
        if (auto asImport = std::dynamic_pointer_cast<ast::ImportDecl>(node)) {
          program.imports.push_back(asImport);
        } else if (auto asDecl = std::dynamic_pointer_cast<ast::Decl>(node)) {
          program.declarations.push_back(asDecl);
          program.topLevel.push_back(asDecl);
        } else if (auto asStmt = std::dynamic_pointer_cast<ast::Stmt>(node)) {
          program.statements.push_back(asStmt);
          program.topLevel.push_back(asStmt);
        }
      }
    } else {
      // Statements, variable declarations and expression statements run in
      // source order at startup, inside __tspp_global_ctors (C2c).
      auto stmt = StatementBuilder::build(stream_);
      if (stmt) {
        if (!stmt->location.isValid()) stmt->location = startTok.getLocation();
        program.statements.push_back(stmt);
        program.topLevel.push_back(stmt);
      }
      node = stmt;
    }
    finishListItem(stream_, node.get(), startPos, errorsBefore,
                   "a declaration or statement");
  }
}

bool ASTBuilderVisitor::isDeclarationStart() const {
  // Special-case: support 'type' keyword spelled as identifier
  if (stream_.peek().getType() == tokens::TokenType::IDENTIFIER &&
      stream_.peek().getLexeme() == std::string("type")) {
    return true;
  }
  // If a class modifier appears first (e.g., #final, #abstract, #packed),
  // treat it as a declaration start so DeclarationBuilder can consume the
  // modifiers and the following 'class' token.
  if (tokens::isClassModifier(stream_.peek().getType())) {
    return true;
  }
  // Function modifiers (#constexpr, #simd, #pinned, ...) before 'function',
  // and #pinned before 'class'
  if (tokens::isFunctionModifier(stream_.peek().getType()) &&
      (stream_.peekNext().getType() == tokens::TokenType::FUNCTION ||
       stream_.peekNext().getType() == tokens::TokenType::CLASS)) {
    return true;
  }
  switch (stream_.peek().getType()) {
    case tokens::TokenType::CONST_FUNCTION:
    case tokens::TokenType::FUNCTION:
    case tokens::TokenType::CLASS:
    case tokens::TokenType::INTERFACE:
    case tokens::TokenType::TYPEDEF:
    case tokens::TokenType::IMPORT:
    case tokens::TokenType::ENUM:
    case tokens::TokenType::NAMESPACE:
    case tokens::TokenType::STACK:
    case tokens::TokenType::HEAP:
    case tokens::TokenType::STATIC:
    case tokens::TokenType::ATTRIBUTE:  // e.g., #final class X { ... }
      return true;
    default:
      return false;
  }
}

}  // namespace parser
