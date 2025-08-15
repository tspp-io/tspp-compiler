#include "StatementBuilder.h"

#include <memory>

#include "core/common/macros.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/visitors/ast_builder/ast_builders/DeclarationBuilder.h"
#include "parser/visitors/ast_builder/ast_builders/ExpressionBuilder.h"

namespace parser {

// Entry point: Parse any statement
Shared(ast::Stmt) StatementBuilder::build(tokens::TokenStream& stream) {
  switch (stream.peek().getType()) {
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
    // Support storage-qualified local declarations: e.g., `#stack let x: int =
    // 1;`
    case tokens::TokenType::STACK: {
      // consume qualifier and delegate
      stream.advance();
      auto varDecl = ::parser::DeclarationBuilder::buildVariable(
          stream, ast::StorageQualifier::Stack);
      if (varDecl)
        return std::static_pointer_cast<ast::Stmt>(varDecl);
      else
        return nullptr;
    }
    case tokens::TokenType::HEAP: {
      stream.advance();
      auto varDecl = ::parser::DeclarationBuilder::buildVariable(
          stream, ast::StorageQualifier::Heap);
      if (varDecl)
        return std::static_pointer_cast<ast::Stmt>(varDecl);
      else
        return nullptr;
    }
    case tokens::TokenType::STATIC: {
      stream.advance();
      auto varDecl = ::parser::DeclarationBuilder::buildVariable(
          stream, ast::StorageQualifier::Static);
      if (varDecl)
        return std::static_pointer_cast<ast::Stmt>(varDecl);
      else
        return nullptr;
    }
    case tokens::TokenType::LET:
    case tokens::TokenType::CONST: {
      auto varDecl = ::parser::DeclarationBuilder::buildVariable(
          stream, ast::StorageQualifier::None);
      if (varDecl)
        return std::static_pointer_cast<ast::Stmt>(varDecl);
      else
        return nullptr;
    }
    default:
      return buildExpressionStatement(stream);
  }
}

// Parse a block: { statement* }
Shared(ast::BlockStmt)
    StatementBuilder::buildBlock(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::LEFT_BRACE) return nullptr;
  auto block = std::make_shared<ast::BlockStmt>();
  block->location = stream.peek().getLocation();
  stream.advance();  // consume '{'
  while (!stream.isAtEnd() &&
         stream.peek().getType() != tokens::TokenType::RIGHT_BRACE) {
    // Patch: handle local variable declarations as VarDecl nodes
    if (stream.peek().getType() == tokens::TokenType::LET ||
        stream.peek().getType() == tokens::TokenType::CONST) {
      auto varDecl = ::parser::DeclarationBuilder::buildVariable(
          stream, ast::StorageQualifier::None);
      if (varDecl)
        block->statements.push_back(
            std::static_pointer_cast<ast::Stmt>(varDecl));
      else
        stream.advance();
      continue;
    }
    auto stmt = build(stream);
    if (stmt)
      block->statements.push_back(stmt);
    else
      stream.advance();  // error recovery
  }
  if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
    stream.advance();  // consume '}'
  }
  return block;
}

// Parse an if statement: if (expr) stmt [else stmt]
Shared(ast::IfStmt) StatementBuilder::buildIf(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::IF) return nullptr;
  auto ifStmt = std::make_shared<ast::IfStmt>();
  ifStmt->location = stream.peek().getLocation();
  stream.advance();  // consume 'if'
  if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN)
    stream.advance();
  ifStmt->condition = ExpressionBuilder::build(stream);
  if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN)
    stream.advance();
  ifStmt->thenBranch = build(stream);
  if (stream.peek().getType() == tokens::TokenType::ELSE) {
    stream.advance();
    ifStmt->elseBranch = build(stream);
  }
  return ifStmt;
}

// Parse a while statement: while (expr) stmt
Shared(ast::WhileStmt)
    StatementBuilder::buildWhile(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::WHILE) return nullptr;
  auto whileStmt = std::make_shared<ast::WhileStmt>();
  whileStmt->location = stream.peek().getLocation();
  stream.advance();  // consume 'while'
  if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN)
    stream.advance();
  whileStmt->condition = ExpressionBuilder::build(stream);
  if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN)
    stream.advance();
  whileStmt->body = build(stream);
  return whileStmt;
}

// Parse a for statement: for ([init]; [cond]; [inc]) stmt
Shared(ast::ForStmt) StatementBuilder::buildFor(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::FOR) return nullptr;
  auto forStmt = std::make_shared<ast::ForStmt>();
  forStmt->location = stream.peek().getLocation();
  stream.advance();  // consume 'for'
  if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN)
    stream.advance();
  // Parse initializer
  if (stream.peek().getType() != tokens::TokenType::SEMICOLON) {
    // Support declarations in init position
    auto t = stream.peek().getType();
    if (t == tokens::TokenType::LET || t == tokens::TokenType::CONST) {
      auto varDecl = ::parser::DeclarationBuilder::buildVariable(
          stream, ast::StorageQualifier::None);
      if (varDecl) forStmt->init = std::static_pointer_cast<ast::Stmt>(varDecl);
    } else if (t == tokens::TokenType::STACK) {
      stream.advance();
      auto varDecl = ::parser::DeclarationBuilder::buildVariable(
          stream, ast::StorageQualifier::Stack);
      if (varDecl) forStmt->init = std::static_pointer_cast<ast::Stmt>(varDecl);
    } else if (t == tokens::TokenType::HEAP) {
      stream.advance();
      auto varDecl = ::parser::DeclarationBuilder::buildVariable(
          stream, ast::StorageQualifier::Heap);
      if (varDecl) forStmt->init = std::static_pointer_cast<ast::Stmt>(varDecl);
    } else if (t == tokens::TokenType::STATIC) {
      stream.advance();
      auto varDecl = ::parser::DeclarationBuilder::buildVariable(
          stream, ast::StorageQualifier::Static);
      if (varDecl) forStmt->init = std::static_pointer_cast<ast::Stmt>(varDecl);
    } else {
      // Fallback: parse as expression statement
      auto expr = ExpressionBuilder::build(stream);
      if (expr) {
        auto exprStmt = std::make_shared<ast::ExprStmt>();
        exprStmt->expression = expr;
        exprStmt->location = expr->location;
        forStmt->init = exprStmt;
      }
    }
  }
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) stream.advance();
  // Parse condition
  if (stream.peek().getType() != tokens::TokenType::SEMICOLON)
    forStmt->condition = ExpressionBuilder::build(stream);
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) stream.advance();
  // Parse increment
  if (stream.peek().getType() != tokens::TokenType::RIGHT_PAREN)
    forStmt->increment = ExpressionBuilder::build(stream);
  if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN)
    stream.advance();
  forStmt->body = build(stream);
  return forStmt;
}

// Parse a return statement: return [expr] ;
Shared(ast::ReturnStmt)
    StatementBuilder::buildReturn(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::RETURN) return nullptr;
  auto ret = std::make_shared<ast::ReturnStmt>();
  ret->location = stream.peek().getLocation();
  stream.advance();  // consume 'return'
  if (stream.peek().getType() != tokens::TokenType::SEMICOLON)
    ret->value = ExpressionBuilder::build(stream);
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) stream.advance();
  return ret;
}

// Parse an expression statement: expr ;
Shared(ast::ExprStmt)
    StatementBuilder::buildExpressionStatement(tokens::TokenStream& stream) {
  auto expr = ExpressionBuilder::build(stream);
  if (!expr) return nullptr;
  auto stmt = std::make_shared<ast::ExprStmt>();
  stmt->expression = expr;
  stmt->location = expr->location;
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) stream.advance();
  return stmt;
}

}  // namespace parser
