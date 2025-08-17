#include "ExpressionBuilder.h"

#include <iostream>
#include <memory>

#include "core/common/macros.h"
#include "parser/nodes/expression_nodes.h"
// For ThisExpr and GroupingExpr/NullExpr definitions
#include "parser/nodes/misc_nodes.h"

namespace parser {

// Entry point: Parse any expression (handles binary precedence)
Shared(ast::Expr) ExpressionBuilder::build(tokens::TokenStream& stream) {
  return buildBinary(stream, 0);
}

// Parse a primary expression: literals, identifiers, parenthesized, unary
Shared(ast::Expr) ExpressionBuilder::buildPrimary(tokens::TokenStream& stream) {
  auto type = stream.peek().getType();

  switch (type) {
    // Handle array literals: [ e1, e2, ... ]
    case tokens::TokenType::LEFT_BRACKET: {
      auto startTok = stream.peek();
      stream.advance();  // consume '['
      auto arr = std::make_shared<ast::ArrayLiteralExpr>();
      arr->location = startTok.getLocation();
      // Empty array
      if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACKET) {
        stream.advance();
        return arr;
      }
      while (!stream.isAtEnd() &&
             stream.peek().getType() != tokens::TokenType::RIGHT_BRACKET) {
        auto elem = build(stream);
        if (elem) arr->elements.push_back(elem);
        if (stream.peek().getType() == tokens::TokenType::COMMA) {
          stream.advance();
        } else {
          break;
        }
      }
      if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACKET) {
        stream.advance();
      }
      return arr;
    }
    // Handle object literals: { key: value, ... }
    case tokens::TokenType::LEFT_BRACE: {
      auto startTok = stream.peek();
      stream.advance();  // consume '{'
      auto obj = std::make_shared<ast::ObjectLiteralExpr>();
      obj->location = startTok.getLocation();

      // Empty object literal
      if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
        stream.advance();  // '}'
        return obj;
      }

      // Parse fields
      while (!stream.isAtEnd() &&
             stream.peek().getType() != tokens::TokenType::RIGHT_BRACE) {
        // Key can be identifier, string literal, or number
        auto keyTok = stream.peek();
        auto kt = keyTok.getType();
        if (kt != tokens::TokenType::IDENTIFIER &&
            kt != tokens::TokenType::STRING_LITERAL &&
            kt != tokens::TokenType::NUMBER) {
          // Malformed; bail out gracefully by skipping until '}'
          while (!stream.isAtEnd() &&
                 stream.peek().getType() != tokens::TokenType::RIGHT_BRACE) {
            stream.advance();
          }
          if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE)
            stream.advance();
          return obj;
        }
        stream.advance();  // consume key
        // Expect ':'
        if (stream.peek().getType() == tokens::TokenType::COLON) {
          stream.advance();
        }
        // Parse value expression
        auto val = build(stream);
        ast::ObjectLiteralExpr::Field f{keyTok, val};
        obj->fields.push_back(f);
        // Comma between fields
        if (stream.peek().getType() == tokens::TokenType::COMMA) {
          stream.advance();
          continue;
        } else {
          break;
        }
      }
      if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
        stream.advance();
      }
      return obj;
    }
    // Handle null literal
    case tokens::TokenType::NULL_VALUE: {
      auto tok = stream.peek();
      stream.advance();
      auto n = std::make_shared<ast::NullExpr>();
      n->nullToken = tok;
      n->location = tok.getLocation();
      return n;
    }
    // Handle 'new' operator: new ClassName(args)
    case tokens::TokenType::NEW: {
      auto newTok = stream.peek();
      stream.advance();
      // Expect class identifier
      if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
        // Malformed; return null to avoid crash
        return nullptr;
      }
      auto classTok = stream.peek();
      stream.advance();
      auto newExpr = std::make_shared<ast::NewExpr>();
      newExpr->keyword = newTok;
      newExpr->className = classTok;
      newExpr->location = newTok.getLocation();
      // Parse optional constructor args: '(' ... ')'
      if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN) {
        stream.advance();
        while (stream.peek().getType() != tokens::TokenType::RIGHT_PAREN &&
               !stream.isAtEnd()) {
          auto arg = build(stream);
          if (arg) newExpr->arguments.push_back(arg);
          if (stream.peek().getType() == tokens::TokenType::COMMA) {
            stream.advance();
          } else {
            break;
          }
        }
        if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN) {
          stream.advance();
        }
      }
      return newExpr;
    }
    // Handle literals (numbers, strings, booleans, chars)
    case tokens::TokenType::CHAR_LITERAL:
    case tokens::TokenType::STRING_LITERAL:
    case tokens::TokenType::NUMBER:
    case tokens::TokenType::TRUE:
    case tokens::TokenType::FALSE: {
      auto expr = std::make_shared<ast::LiteralExpr>();
      expr->value = stream.peek();
      expr->location = stream.peek().getLocation();
      stream.advance();
      return expr;
    }
    // Handle 'this' keyword as a primary expression, with member access/calls
    case tokens::TokenType::THIS: {
      auto thisToken = stream.peek();
      stream.advance();
      auto thisExpr = std::make_shared<ast::ThisExpr>();
      thisExpr->keyword = thisToken;
      thisExpr->location = thisToken.getLocation();
      // Support chained member access: this.foo.bar
      Shared(ast::Expr) object = thisExpr;
      while (stream.peek().getType() == tokens::TokenType::DOT) {
        stream.advance();  // consume '.'
        auto t = stream.peek().getType();
        if (t == tokens::TokenType::IDENTIFIER || t == tokens::TokenType::GET ||
            t == tokens::TokenType::SET) {
          auto memberToken = stream.peek();
          stream.advance();
          auto memberExpr = std::make_shared<ast::MemberAccessExpr>();
          memberExpr->object = object;
          memberExpr->member = memberToken;
          memberExpr->location = object->location;
          object = memberExpr;
        } else {
          break;  // Malformed member access; exit gracefully
        }
      }
      // If next token is '(', treat as a function call on the member chain
      if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN) {
        return buildCall(stream, object);
      }
      return object;
    }
    // Handle identifiers and member access (console.log, foo.bar.baz)
    case tokens::TokenType::IDENTIFIER: {
      auto expr = std::make_shared<ast::IdentifierExpr>();
      expr->name = stream.peek();
      expr->location = stream.peek().getLocation();
      stream.advance();
      // Parse chained member accesses and index accesses
      Shared(ast::Expr) object = expr;
      while (true) {
        if (stream.peek().getType() == tokens::TokenType::DOT) {
          stream.advance();  // consume '.'
          auto t = stream.peek().getType();
          if (t == tokens::TokenType::IDENTIFIER ||
              t == tokens::TokenType::GET || t == tokens::TokenType::SET) {
            auto memberToken = stream.peek();
            stream.advance();
            auto memberExpr = std::make_shared<ast::MemberAccessExpr>();
            memberExpr->object = object;
            memberExpr->member = memberToken;
            memberExpr->location = object->location;
            object = memberExpr;
            continue;
          } else {
            break;
          }
        } else if (stream.peek().getType() == tokens::TokenType::LEFT_BRACKET) {
          // Index access: obj[expr]
          auto lb = stream.peek();
          stream.advance();
          auto indexExpr = build(stream);
          if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACKET) {
            stream.advance();
          }
          auto idx = std::make_shared<ast::IndexAccessExpr>();
          idx->object = object;
          idx->index = indexExpr;
          idx->location = lb.getLocation();
          object = idx;
          continue;
        } else {
          break;
        }
      }
      // If next token is '(', treat as function call on the (possibly chained)
      // member
      if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN) {
        return buildCall(stream, object);
      }
      return object;
    }
    // Handle parenthesized expressions: (expr)
    case tokens::TokenType::LEFT_PAREN: {
      stream.advance();  // consume '('
      auto expr = build(stream);
      if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN) {
        stream.advance();  // consume ')'
      }
      return expr;
    }
      // Handle unary operators: -expr, !expr, @expr (address-of), *expr
      // (dereference)
    case tokens::TokenType::MINUS:
    case tokens::TokenType::EXCLAIM:
    case tokens::TokenType::TILDE:   // Bitwise NOT (~)
    case tokens::TokenType::AT:      // Address-of operator (@)
    case tokens::TokenType::STAR: {  // Dereference operator (*)
      auto op = stream.peek();
      stream.advance();
      auto right = buildPrimary(stream);
      auto expr = std::make_shared<ast::UnaryExpr>();
      expr->op = op;
      expr->operand = right;
      expr->location = op.getLocation();
      return expr;
    }
    default:
      // Unrecognized: skip token and return nullptr
      stream.advance();
      return nullptr;
  }
}

// Parse a binary expression using precedence climbing
Shared(ast::Expr) ExpressionBuilder::buildBinary(tokens::TokenStream& stream,
                                                 int minPrec) {
  auto left = buildPrimary(stream);
  while (true) {
    auto type = stream.peek().getType();
    if (!isBinaryOperator(type)) break;
    int prec = getOperatorPrecedence(type);
    if (prec < minPrec) break;
    auto op = stream.peek();
    stream.advance();

    // Special handling for assignment operators
    if (type == tokens::TokenType::EQUALS) {
      // Assignment is right-associative: parse RHS with the same precedence
      // so that chains like x = y = 5 associate as x = (y = 5)
      auto right = buildBinary(stream, prec);
      auto assignExpr = std::make_shared<ast::AssignmentExpr>();
      assignExpr->target = left;
      assignExpr->value = right;
      assignExpr->location = op.getLocation();
      left = assignExpr;
    } else {
      // Regular binary expression
      auto right = buildBinary(stream, prec + 1);
      auto expr = std::make_shared<ast::BinaryExpr>();
      expr->op = op;
      expr->left = left;
      expr->right = right;
      expr->location = op.getLocation();
      left = expr;
    }
  }
  // Postfix type assertion: '<expr> as <Type>' -> consume and return left
  // We treat all 'as' casts as no-ops in codegen for now, but we must consume
  // the following type tokens to keep the parser in sync.
  if (!stream.isAtEnd() &&
      stream.peek().getType() == tokens::TokenType::IDENTIFIER &&
      stream.peek().getLexeme() == std::string("as")) {
    // consume 'as'
    stream.advance();
    // Best-effort consume a simple type: either a basic type token or identifier
    auto tt = stream.peek().getType();
    if (tt == tokens::TokenType::IDENTIFIER ||
        tt == tokens::TokenType::INT || tt == tokens::TokenType::FLOAT ||
        tt == tokens::TokenType::BOOLEAN || tt == tokens::TokenType::STRING ||
        tt == tokens::TokenType::VOID) {
      stream.advance();
      // Optionally consume array suffixes: [] ...
      while (stream.peek().getType() == tokens::TokenType::LEFT_BRACKET &&
             stream.peekNext().getType() == tokens::TokenType::RIGHT_BRACKET) {
        stream.advance(); // '['
        stream.advance(); // ']'
      }
      // Optionally consume pointer suffixes: '*'+
      while (stream.peek().getType() == tokens::TokenType::STAR) {
        stream.advance();
      }
    }
    return left;
  }
  return left;
}

// Parse a function call: callee(args...)
Shared(ast::CallExpr) ExpressionBuilder::buildCall(tokens::TokenStream& stream,
                                                   Shared(ast::Expr) callee) {
  if (stream.peek().getType() != tokens::TokenType::LEFT_PAREN) return nullptr;
  auto call = std::make_shared<ast::CallExpr>();
  call->callee = callee;
  call->location = callee->location;
  stream.advance();  // consume '('
  // Parse arguments (comma-separated expressions)
  while (stream.peek().getType() != tokens::TokenType::RIGHT_PAREN &&
         !stream.isAtEnd()) {
    auto arg = build(stream);
    if (arg) call->arguments.push_back(arg);
    if (stream.peek().getType() == tokens::TokenType::COMMA) {
      stream.advance();
    } else {
      break;
    }
  }
  if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN) {
    stream.advance();
  }
  return call;
}

// Operator precedence: higher number = higher precedence
int ExpressionBuilder::getOperatorPrecedence(tokens::TokenType type) {
  switch (type) {
    case tokens::TokenType::STAR:
    case tokens::TokenType::SLASH:
    case tokens::TokenType::PERCENT:
      return 6;  // Multiplicative
    case tokens::TokenType::PLUS:
    case tokens::TokenType::MINUS:
      return 5;  // Additive
    case tokens::TokenType::EQUALS_EQUALS:
    case tokens::TokenType::EXCLAIM_EQUALS:
    case tokens::TokenType::LESS:
    case tokens::TokenType::LESS_EQUALS:
    case tokens::TokenType::GREATER:
    case tokens::TokenType::GREATER_EQUALS:
      return 4;  // Comparison
    case tokens::TokenType::AMPERSAND_AMPERSAND:
      return 3;  // Logical AND
    case tokens::TokenType::PIPE_PIPE:
      return 2;  // Logical OR
    case tokens::TokenType::PIPE:
      return 4;  // Bitwise OR (below comparison, above logical)
    case tokens::TokenType::CARET:
      return 4;  // Bitwise XOR
    case tokens::TokenType::AMPERSAND:
      return 4;  // Bitwise AND
    case tokens::TokenType::LEFT_SHIFT:
    case tokens::TokenType::RIGHT_SHIFT:
      return 5;  // Shift (between additive and comparison per docs)
    case tokens::TokenType::EQUALS:
      return 1;  // Assignment
    default:
      return 0;
  }
}

// Returns true if the token is a binary operator
bool ExpressionBuilder::isBinaryOperator(tokens::TokenType type) {
  switch (type) {
    case tokens::TokenType::PLUS:
    case tokens::TokenType::MINUS:
    case tokens::TokenType::STAR:
    case tokens::TokenType::SLASH:
    case tokens::TokenType::PERCENT:
    case tokens::TokenType::EQUALS_EQUALS:
    case tokens::TokenType::EXCLAIM_EQUALS:
    case tokens::TokenType::LESS:
    case tokens::TokenType::LESS_EQUALS:
    case tokens::TokenType::GREATER:
    case tokens::TokenType::GREATER_EQUALS:
    case tokens::TokenType::AMPERSAND_AMPERSAND:
    case tokens::TokenType::PIPE_PIPE:
    case tokens::TokenType::PIPE:
    case tokens::TokenType::CARET:
    case tokens::TokenType::AMPERSAND:
    case tokens::TokenType::LEFT_SHIFT:
    case tokens::TokenType::RIGHT_SHIFT:
    case tokens::TokenType::EQUALS:
      return true;
    default:
      return false;
  }
}

}  // namespace parser
