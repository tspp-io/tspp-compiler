#include "ast_builder_visitor.h"

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

    // Handle imports
    if (stream_.check(tokens::TokenType::IMPORT)) {
      auto importDecl = DeclarationBuilder::buildImport(stream_);
      if (importDecl) {
        program.imports.push_back(
            std::static_pointer_cast<ast::ImportDecl>(importDecl));
      }
    }
    // Handle declarations
    else if (isDeclarationStart()) {
      auto decl = DeclarationBuilder::build(stream_);
      if (decl) {
        program.declarations.push_back(decl);
      }
    }
    // Handle statements
    else if (isStatementStart()) {
      auto stmt = StatementBuilder::build(stream_);
      if (stmt) {
        program.statements.push_back(stmt);
      }
    }
    // Handle unexpected tokens
    else {
      // Skip unrecognized token and try to continue
      stream_.advance();
    }
  }
}

bool ASTBuilderVisitor::isDeclarationStart() const {
  auto type = stream_.peek().getType();
  return type == tokens::TokenType::LET || type == tokens::TokenType::CONST ||
         type == tokens::TokenType::FUNCTION ||
         type == tokens::TokenType::CLASS ||
         type == tokens::TokenType::INTERFACE;
}

bool ASTBuilderVisitor::isStatementStart() const {
  auto type = stream_.peek().getType();
  return type == tokens::TokenType::IF || type == tokens::TokenType::WHILE ||
         type == tokens::TokenType::FOR || type == tokens::TokenType::RETURN ||
         type == tokens::TokenType::LEFT_BRACE;  // Block statement
}

}  // namespace parser
