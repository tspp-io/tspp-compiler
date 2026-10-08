#include "StatementBuilder.h"

#include <memory>

#include "core/common/macros.h"
#include "parser/nodes/statement_nodes.h"
#include "parser/visitors/ast_builder/ast_builders/DeclarationBuilder.h"
#include "parser/visitors/ast_builder/ast_builders/ExpressionBuilder.h"
#include "parser/visitors/ast_builder/ast_builders/TypeBuilder.h"

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
    case tokens::TokenType::DO:
      return buildDoWhile(stream);
    case tokens::TokenType::SWITCH:
      return buildSwitch(stream);
    case tokens::TokenType::BREAK:
    case tokens::TokenType::CONTINUE:
      return buildJump(stream);
    case tokens::TokenType::THROW:
      return buildThrow(stream);
    case tokens::TokenType::TRY:
      return buildTry(stream);
    case tokens::TokenType::RETURN:
      return buildReturn(stream);
    case tokens::TokenType::ASM:
      return buildAsm(stream);
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
    // Empty statement: skip it here so error recovery below does not also
    // skip the following token
    if (stream.peek().getType() == tokens::TokenType::SEMICOLON) {
      stream.advance();
      continue;
    }
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
// or a for-of statement: for (let|const name[: Type] of expr) stmt
Shared(ast::Stmt) StatementBuilder::buildFor(tokens::TokenStream& stream) {
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
      // for-of: the declaration stops at 'of' because it has no initializer
      if (varDecl && stream.peek().getType() == tokens::TokenType::OF) {
        stream.advance();  // consume 'of'
        auto forOf = std::make_shared<ast::ForOfStmt>();
        forOf->location = forStmt->location;
        forOf->variable = varDecl;
        forOf->iterable = ExpressionBuilder::build(stream);
        if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN)
          stream.advance();
        forOf->body = build(stream);
        return forOf;
      }
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

// Parse a do-while statement: do stmt while (expr) ;
Shared(ast::DoWhileStmt)
    StatementBuilder::buildDoWhile(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::DO) return nullptr;
  auto doWhile = std::make_shared<ast::DoWhileStmt>();
  doWhile->location = stream.peek().getLocation();
  stream.advance();  // consume 'do'
  while (stream.peek().getType() == tokens::TokenType::SEMICOLON)
    stream.advance();  // ASI after 'do' at end of line
  doWhile->body = build(stream);
  if (stream.peek().getType() != tokens::TokenType::WHILE) return nullptr;
  stream.advance();  // consume 'while'
  if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN)
    stream.advance();
  doWhile->condition = ExpressionBuilder::build(stream);
  if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN)
    stream.advance();
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) stream.advance();
  return doWhile;
}

// Parse a switch statement:
//   switch (expr) { (case expr: | default:) stmt* ... }
Shared(ast::SwitchStmt)
    StatementBuilder::buildSwitch(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::SWITCH) return nullptr;
  auto switchStmt = std::make_shared<ast::SwitchStmt>();
  switchStmt->location = stream.peek().getLocation();
  stream.advance();  // consume 'switch'
  if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN)
    stream.advance();
  switchStmt->discriminant = ExpressionBuilder::build(stream);
  if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN)
    stream.advance();
  if (stream.peek().getType() != tokens::TokenType::LEFT_BRACE) return nullptr;
  stream.advance();  // consume '{'
  while (!stream.isAtEnd() &&
         stream.peek().getType() != tokens::TokenType::RIGHT_BRACE) {
    auto t = stream.peek().getType();
    if (t != tokens::TokenType::CASE && t != tokens::TokenType::DEFAULT) {
      stream.advance();  // error recovery: statement outside of a case
      continue;
    }
    ast::SwitchStmt::Case c;
    c.location = stream.peek().getLocation();
    stream.advance();  // consume 'case' / 'default'
    if (t == tokens::TokenType::CASE) c.test = ExpressionBuilder::build(stream);
    if (stream.peek().getType() == tokens::TokenType::COLON) stream.advance();
    while (!stream.isAtEnd()) {
      auto st = stream.peek().getType();
      if (st == tokens::TokenType::CASE || st == tokens::TokenType::DEFAULT ||
          st == tokens::TokenType::RIGHT_BRACE)
        break;
      // Empty statement (also produced by ASI after 'case x:' at end of line)
      if (st == tokens::TokenType::SEMICOLON) {
        stream.advance();
        continue;
      }
      auto stmt = build(stream);
      if (stmt)
        c.body.push_back(stmt);
      else
        stream.advance();  // error recovery
    }
    switchStmt->cases.push_back(std::move(c));
  }
  if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE)
    stream.advance();  // consume '}'
  return switchStmt;
}

// Parse 'break ;' or 'continue ;'
Shared(ast::Stmt) StatementBuilder::buildJump(tokens::TokenStream& stream) {
  Shared(ast::Stmt) stmt;
  switch (stream.peek().getType()) {
    case tokens::TokenType::BREAK:
      stmt = std::make_shared<ast::BreakStmt>();
      break;
    case tokens::TokenType::CONTINUE:
      stmt = std::make_shared<ast::ContinueStmt>();
      break;
    default:
      return nullptr;
  }
  stmt->location = stream.peek().getLocation();
  stream.advance();  // consume keyword
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) stream.advance();
  return stmt;
}

// Parse a throw statement: throw expr ;
Shared(ast::ThrowStmt)
    StatementBuilder::buildThrow(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::THROW) return nullptr;
  auto throwStmt = std::make_shared<ast::ThrowStmt>();
  throwStmt->location = stream.peek().getLocation();
  stream.advance();  // consume 'throw'
  throwStmt->value = ExpressionBuilder::build(stream);
  if (!throwStmt->value) return nullptr;
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) stream.advance();
  return throwStmt;
}

// Parse a try statement:
//   try block [catch [(name[: Type])] block] [finally block]
// At least one of catch/finally is required.
Shared(ast::TryStmt) StatementBuilder::buildTry(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::TRY) return nullptr;
  auto tryStmt = std::make_shared<ast::TryStmt>();
  tryStmt->location = stream.peek().getLocation();
  stream.advance();  // consume 'try'
  tryStmt->tryBlock = buildBlock(stream);
  if (!tryStmt->tryBlock) return nullptr;
  if (stream.peek().getType() == tokens::TokenType::CATCH) {
    stream.advance();  // consume 'catch'
    if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN) {
      stream.advance();  // consume '('
      if (stream.peek().getType() != tokens::TokenType::IDENTIFIER)
        return nullptr;
      tryStmt->hasCatchParam = true;
      tryStmt->catchParam = stream.peek();
      stream.advance();  // consume identifier
      if (stream.peek().getType() == tokens::TokenType::COLON) {
        stream.advance();  // consume ':'
        tryStmt->catchType = TypeBuilder::build(stream);
      }
      if (stream.peek().getType() != tokens::TokenType::RIGHT_PAREN)
        return nullptr;
      stream.advance();  // consume ')'
    }
    tryStmt->catchBlock = buildBlock(stream);
    if (!tryStmt->catchBlock) return nullptr;
  }
  if (stream.peek().getType() == tokens::TokenType::FINALLY) {
    stream.advance();  // consume 'finally'
    tryStmt->finallyBlock = buildBlock(stream);
    if (!tryStmt->finallyBlock) return nullptr;
  }
  if (!tryStmt->catchBlock && !tryStmt->finallyBlock) return nullptr;
  return tryStmt;
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

Shared(ast::AsmStmt) StatementBuilder::buildAsm(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::ASM) return nullptr;
  auto stmt = std::make_shared<ast::AsmStmt>();
  stmt->location = stream.peek().getLocation();
  stream.advance();  // consume #asm

  if (stream.peek().getType() == tokens::TokenType::VOLATILE) {
    stmt->isVolatile = true;
    stream.advance();
  }

  if (stream.peek().getType() != tokens::TokenType::LEFT_PAREN) {
    // Error: expected '('
    return nullptr;
  }
  stream.advance();  // consume '('

  // Parse assembly string
  if (stream.peek().getType() != tokens::TokenType::STRING_LITERAL) {
    // Error: expected string literal
    return nullptr;
  }
  stmt->assembly = stream.peek().getLexeme();
  // Remove quotes
  if (stmt->assembly.size() >= 2) {
    stmt->assembly = stmt->assembly.substr(1, stmt->assembly.size() - 2);
  }
  stream.advance();

  // Parse outputs
  if (stream.peek().getType() == tokens::TokenType::COLON) {
    stream.advance();  // consume ':'
    while (stream.peek().getType() == tokens::TokenType::STRING_LITERAL) {
      ast::AsmStmt::Constraint c;
      c.constraint = stream.peek().getLexeme();
      if (c.constraint.size() >= 2)
        c.constraint = c.constraint.substr(1, c.constraint.size() - 2);
      stream.advance();

      if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN) {
        stream.advance();
        c.expression = ExpressionBuilder::build(stream);
        if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN) {
          stream.advance();
        }
      }
      stmt->outputs.push_back(c);

      if (stream.peek().getType() == tokens::TokenType::COMMA) {
        stream.advance();
      } else {
        break;
      }
    }
  }

  // Parse inputs
  if (stream.peek().getType() == tokens::TokenType::COLON) {
    stream.advance();  // consume ':'
    while (stream.peek().getType() == tokens::TokenType::STRING_LITERAL) {
      ast::AsmStmt::Constraint c;
      c.constraint = stream.peek().getLexeme();
      if (c.constraint.size() >= 2)
        c.constraint = c.constraint.substr(1, c.constraint.size() - 2);
      stream.advance();

      if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN) {
        stream.advance();
        c.expression = ExpressionBuilder::build(stream);
        if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN) {
          stream.advance();
        }
      }
      stmt->inputs.push_back(c);

      if (stream.peek().getType() == tokens::TokenType::COMMA) {
        stream.advance();
      } else {
        break;
      }
    }
  }

  // Parse clobbers
  if (stream.peek().getType() == tokens::TokenType::COLON) {
    stream.advance();  // consume ':'
    while (stream.peek().getType() == tokens::TokenType::STRING_LITERAL) {
      std::string clobber = stream.peek().getLexeme();
      if (clobber.size() >= 2) clobber = clobber.substr(1, clobber.size() - 2);
      stmt->clobbers.push_back(clobber);
      stream.advance();

      if (stream.peek().getType() == tokens::TokenType::COMMA) {
        stream.advance();
      } else {
        break;
      }
    }
  }

  if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN) {
    stream.advance();
  }

  // Optional semicolon
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) {
    stream.advance();
  }

  return stmt;
}

}  // namespace parser
