#include "ast_builder_visitor.h"

#include <iostream>

#include "ast_builders/DeclarationBuilder.h"
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
    // Skip any potential EOF or error tokens
    if (stream_.peek().getType() == tokens::TokenType::END_OF_FILE) {
      break;
    }

    // Handle optional semicolons (skip them, as in JavaScript)
    if (stream_.peek().getType() == tokens::TokenType::SEMICOLON) {
      stream_.advance();
      continue;
    }

    // Handle imports as a top-level construct
    if (stream_.peek().getType() == tokens::TokenType::IMPORT) {
      auto importDecl = DeclarationBuilder::buildImport(stream_);
      if (importDecl) {
        program.imports.push_back(importDecl);
      }
      // Optionally consume a semicolon after an import
      if (stream_.peek().getType() == tokens::TokenType::SEMICOLON) {
        stream_.advance();
      }
      continue;
    }

    // Handle declarations (functions, classes, etc.)
    if (isDeclarationStart()) {
      auto node = DeclarationBuilder::build(stream_);
      if (node) {
        node->location = stream_.peek().getLocation();
        if (auto asDecl = std::dynamic_pointer_cast<ast::Decl>(node)) {
          program.declarations.push_back(asDecl);
        } else if (auto asStmt = std::dynamic_pointer_cast<ast::Stmt>(node)) {
          program.statements.push_back(asStmt);
        }
      } else {
        stream_.advance();
      }
      if (stream_.peek().getType() == tokens::TokenType::SEMICOLON) {
        stream_.advance();
      }
      continue;
    }

    // Handle variable declarations as statements
    if (stream_.peek().getType() == tokens::TokenType::LET ||
        stream_.peek().getType() == tokens::TokenType::CONST) {
      auto stmt = StatementBuilder::build(stream_);
      if (stmt) {
        program.statements.push_back(stmt);
      } else {
        stream_.advance();
      }
      if (stream_.peek().getType() == tokens::TokenType::SEMICOLON) {
        stream_.advance();
      }
      continue;
    }

    // Handle statements
    if (isStatementStart()) {
      auto stmt = StatementBuilder::build(stream_);
      if (stmt) {
        program.statements.push_back(stmt);
      } else {
        stream_.advance();
      }
      // Optionally consume a semicolon after a statement
      if (stream_.peek().getType() == tokens::TokenType::SEMICOLON) {
        stream_.advance();
      }
      continue;
    }
    stream_.advance();
  }
}

bool ASTBuilderVisitor::isDeclarationStart() const {
  switch (stream_.peek().getType()) {
    case tokens::TokenType::CONST_FUNCTION:
    case tokens::TokenType::FUNCTION:
    case tokens::TokenType::CLASS:
    case tokens::TokenType::INTERFACE:
    case tokens::TokenType::TYPEDEF:
    case tokens::TokenType::ENUM:
    case tokens::TokenType::NAMESPACE:
    case tokens::TokenType::STACK:
    case tokens::TokenType::HEAP:
    case tokens::TokenType::STATIC:
      return true;
    default:
      return false;
  }
}

bool ASTBuilderVisitor::isStatementStart() const {
  auto type = stream_.peek().getType();
  return type == tokens::TokenType::IF || type == tokens::TokenType::WHILE ||
         type == tokens::TokenType::FOR || type == tokens::TokenType::RETURN ||
         type == tokens::TokenType::LEFT_BRACE ||
         type == tokens::TokenType::LET || type == tokens::TokenType::CONST;
}

}  // namespace parser
