#include "StatementBuilder.h"

#include <iostream>

#include "ExpressionBuilder.h"
#include "tokens/token_type.h"

namespace parser {

Shared(ast::Stmt) StatementBuilder::build(tokens::TokenStream& stream) {
  auto type = stream.peek().getType();

  switch (type) {
    case tokens::TokenType::LEFT_BRACE:
      return buildBlock(stream);

    case tokens::TokenType::IF:
      return buildIf(stream);

    case tokens::TokenType::WHILE:
      return buildWhile(stream);

    case tokens::TokenType::FOR:
      return buildFor(stream);

    case tokens::TokenType::RETURN:
      return buildReturn(stream);

    default:
      // Try to parse as expression statement
      return buildExpressionStatement(stream);
  }
}

Shared(ast::BlockStmt)
    StatementBuilder::buildBlock(tokens::TokenStream& stream) {
  // Expect '{'
  if (!stream.match(tokens::TokenType::LEFT_BRACE)) {
    return nullptr;
  }

  auto blockStmt = MakeShared(ast::BlockStmt)();

  // Parse statements until '}'
  while (!stream.check(tokens::TokenType::RIGHT_BRACE) && !stream.isAtEnd()) {
    auto stmt = build(stream);
    if (stmt) {
      blockStmt->statements.push_back(stmt);
    } else {
      // Skip token if we can't parse a statement
      stream.advance();
    }
  }

  // Expect '}'
  if (!stream.match(tokens::TokenType::RIGHT_BRACE)) {
    std::cerr << "Expected '}' to close block" << std::endl;
  }

  return blockStmt;
}

Shared(ast::IfStmt) StatementBuilder::buildIf(tokens::TokenStream& stream) {
  // Expect 'if'
  if (!stream.match(tokens::TokenType::IF)) {
    return nullptr;
  }

  auto ifStmt = MakeShared(ast::IfStmt)();

  // Expect '('
  if (!stream.match(tokens::TokenType::LEFT_PAREN)) {
    std::cerr << "Expected '(' after 'if'" << std::endl;
    return nullptr;
  }

  // Parse condition
  ifStmt->condition = ExpressionBuilder::build(stream);
  if (!ifStmt->condition) {
    std::cerr << "Expected condition in if statement" << std::endl;
    return nullptr;
  }

  // Expect ')'
  if (!stream.match(tokens::TokenType::RIGHT_PAREN)) {
    std::cerr << "Expected ')' after if condition" << std::endl;
    return nullptr;
  }

  // Parse then branch
  ifStmt->thenBranch = build(stream);
  if (!ifStmt->thenBranch) {
    std::cerr << "Expected statement after if condition" << std::endl;
    return nullptr;
  }

  // Optional else branch
  if (stream.match(tokens::TokenType::ELSE)) {
    ifStmt->elseBranch = build(stream);
  }

  return ifStmt;
}

Shared(ast::WhileStmt)
    StatementBuilder::buildWhile(tokens::TokenStream& stream) {
  // Expect 'while'
  if (!stream.match(tokens::TokenType::WHILE)) {
    return nullptr;
  }

  auto whileStmt = MakeShared(ast::WhileStmt)();

  // Expect '('
  if (!stream.match(tokens::TokenType::LEFT_PAREN)) {
    std::cerr << "Expected '(' after 'while'" << std::endl;
    return nullptr;
  }

  // Parse condition
  whileStmt->condition = ExpressionBuilder::build(stream);
  if (!whileStmt->condition) {
    std::cerr << "Expected condition in while statement" << std::endl;
    return nullptr;
  }

  // Expect ')'
  if (!stream.match(tokens::TokenType::RIGHT_PAREN)) {
    std::cerr << "Expected ')' after while condition" << std::endl;
    return nullptr;
  }

  // Parse body
  whileStmt->body = build(stream);
  if (!whileStmt->body) {
    std::cerr << "Expected statement after while condition" << std::endl;
    return nullptr;
  }

  return whileStmt;
}

Shared(ast::ForStmt) StatementBuilder::buildFor(tokens::TokenStream& stream) {
  // Expect 'for'
  if (!stream.match(tokens::TokenType::FOR)) {
    return nullptr;
  }

  auto forStmt = MakeShared(ast::ForStmt)();

  // Expect '('
  if (!stream.match(tokens::TokenType::LEFT_PAREN)) {
    std::cerr << "Expected '(' after 'for'" << std::endl;
    return nullptr;
  }

  // Parse init (optional)
  if (!stream.check(tokens::TokenType::SEMICOLON)) {
    forStmt->init = ExpressionBuilder::build(stream);
  }
  if (!stream.match(tokens::TokenType::SEMICOLON)) {
    std::cerr << "Expected ';' after for init" << std::endl;
    return nullptr;
  }

  // Parse condition (optional)
  if (!stream.check(tokens::TokenType::SEMICOLON)) {
    forStmt->condition = ExpressionBuilder::build(stream);
  }
  if (!stream.match(tokens::TokenType::SEMICOLON)) {
    std::cerr << "Expected ';' after for condition" << std::endl;
    return nullptr;
  }

  // Parse increment (optional)
  if (!stream.check(tokens::TokenType::RIGHT_PAREN)) {
    forStmt->increment = ExpressionBuilder::build(stream);
  }

  // Expect ')'
  if (!stream.match(tokens::TokenType::RIGHT_PAREN)) {
    std::cerr << "Expected ')' after for clauses" << std::endl;
    return nullptr;
  }

  // Parse body
  forStmt->body = build(stream);
  if (!forStmt->body) {
    std::cerr << "Expected statement after for clauses" << std::endl;
    return nullptr;
  }

  return forStmt;
}

Shared(ast::ReturnStmt)
    StatementBuilder::buildReturn(tokens::TokenStream& stream) {
  // Expect 'return'
  if (!stream.match(tokens::TokenType::RETURN)) {
    return nullptr;
  }

  auto returnStmt = MakeShared(ast::ReturnStmt)();

  // Optional return value
  if (!stream.check(tokens::TokenType::SEMICOLON)) {
    returnStmt->value = ExpressionBuilder::build(stream);
  }

  // Expect semicolon
  if (!stream.match(tokens::TokenType::SEMICOLON)) {
    std::cerr << "Expected ';' after return statement" << std::endl;
  }

  return returnStmt;
}

Shared(ast::ExprStmt)
    StatementBuilder::buildExpressionStatement(tokens::TokenStream& stream) {
  auto exprStmt = MakeShared(ast::ExprStmt)();

  exprStmt->expression = ExpressionBuilder::build(stream);
  if (!exprStmt->expression) {
    return nullptr;
  }

  // Expect semicolon
  if (!stream.match(tokens::TokenType::SEMICOLON)) {
    std::cerr << "Expected ';' after expression statement" << std::endl;
  }

  return exprStmt;
}

}  // namespace parser
