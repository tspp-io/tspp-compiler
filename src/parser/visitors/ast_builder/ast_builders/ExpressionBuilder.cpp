#include "ExpressionBuilder.h"

#include <iostream>
#include <memory>

#include "core/common/macros.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/visitors/ast_builder/ast_builders/ParseDiagnostics.h"
// For ThisExpr and GroupingExpr/NullExpr definitions
#include "parser/nodes/misc_nodes.h"

namespace parser {

namespace {
// At '<' after a callee: if the tokens up to the matching '>' can only be
// type arguments and are followed by '(', '.' or '[', consume them (they are
// not used by codegen) and return true. Otherwise leave the stream alone, so
// `a < b` stays a comparison.
bool skipTypeArguments(tokens::TokenStream& stream) {
  size_t save = stream.getCurrentPosition();
  int depth = 0;
  while (!stream.isAtEnd()) {
    auto t = stream.peek().getType();
    if (t == tokens::TokenType::LESS) {
      ++depth;
    } else if (t == tokens::TokenType::GREATER) {
      --depth;
    } else if (t == tokens::TokenType::RIGHT_SHIFT) {
      depth -= 2;
    } else if (!(t == tokens::TokenType::IDENTIFIER ||
                 t == tokens::TokenType::INT || t == tokens::TokenType::FLOAT ||
                 t == tokens::TokenType::BOOLEAN ||
                 t == tokens::TokenType::STRING ||
                 t == tokens::TokenType::VOID ||
                 t == tokens::TokenType::COMMA || t == tokens::TokenType::DOT ||
                 t == tokens::TokenType::STAR || t == tokens::TokenType::AT ||
                 t == tokens::TokenType::PIPE ||
                 t == tokens::TokenType::LEFT_BRACKET ||
                 t == tokens::TokenType::RIGHT_BRACKET)) {
      break;
    }
    stream.advance();
    if (depth <= 0) break;
  }
  auto next = stream.peek().getType();
  if (depth == 0 && (next == tokens::TokenType::LEFT_PAREN ||
                     next == tokens::TokenType::DOT ||
                     next == tokens::TokenType::LEFT_BRACKET)) {
    return true;
  }
  stream.setPosition(save);
  return false;
}
}  // namespace

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
        if (!elem) return nullptr;
        arr->elements.push_back(elem);
        if (stream.peek().getType() == tokens::TokenType::COMMA) {
          stream.advance();
        } else {
          break;
        }
      }
      if (!expect(stream, tokens::TokenType::RIGHT_BRACKET,
                  "',' or ']' in array literal"))
        return nullptr;
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
          expectedError(stream, "a property name in object literal");
          return nullptr;
        }
        stream.advance();  // consume key
        if (!expect(stream, tokens::TokenType::COLON,
                    "':' after property name"))
          return nullptr;
        // Parse value expression
        auto val = build(stream);
        if (!val) return nullptr;
        ast::ObjectLiteralExpr::Field f{keyTok, val};
        obj->fields.push_back(f);
        // Comma between fields (a line break may follow the last one)
        if (stream.peek().getType() == tokens::TokenType::COMMA) {
          stream.advance();
        } else {
          break;
        }
        while (isImplicitSemicolon(stream.peek())) stream.advance();
      }
      while (isImplicitSemicolon(stream.peek())) stream.advance();
      if (!expect(stream, tokens::TokenType::RIGHT_BRACE,
                  "',' or '}' in object literal"))
        return nullptr;
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
    // Handle 'new' operator: new ClassName(args) or new BasicType[expr]
    case tokens::TokenType::NEW: {
      auto newTok = stream.peek();
      stream.advance();
      // Expect class identifier or basic type (int, float, boolean, string)
      auto t = stream.peek().getType();
      if (!(t == tokens::TokenType::IDENTIFIER || t == tokens::TokenType::INT ||
            t == tokens::TokenType::FLOAT || t == tokens::TokenType::BOOLEAN ||
            t == tokens::TokenType::STRING)) {
        expectedError(stream, "a class or type name after 'new'");
        return nullptr;
      }
      auto classTok = stream.peek();
      stream.advance();
      auto newExpr = std::make_shared<ast::NewExpr>();
      newExpr->keyword = newTok;
      newExpr->className = classTok;
      newExpr->location = newTok.getLocation();
      // Optionally consume generic arguments after class name: Foo<Bar, Baz>
      if (stream.peek().getType() == tokens::TokenType::LESS) {
        int depth = 0;
        // Consume tokens until the matching '>' is found
        do {
          auto ct = stream.peek().getType();
          if (ct == tokens::TokenType::LESS) depth++;
          if (ct == tokens::TokenType::GREATER) depth--;
          stream.advance();
          if (stream.isAtEnd()) break;
        } while (depth > 0);
      }
      // Two forms:
      // 1) Constructor args: '(' ... ')'
      // 2) Array dimension: '[' expr ']'
      if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN) {
        stream.advance();
        while (stream.peek().getType() != tokens::TokenType::RIGHT_PAREN &&
               !stream.isAtEnd()) {
          auto arg = build(stream);
          if (!arg) return nullptr;
          newExpr->arguments.push_back(arg);
          if (stream.peek().getType() == tokens::TokenType::COMMA) {
            stream.advance();
          } else {
            break;
          }
        }
        if (!expect(stream, tokens::TokenType::RIGHT_PAREN,
                    "',' or ')' in argument list"))
          return nullptr;
      } else if (stream.peek().getType() == tokens::TokenType::LEFT_BRACKET) {
        // Parse single dimension for now: new T[expr]
        stream.advance();  // '['
        auto sizeExpr = build(stream);
        if (!sizeExpr) return nullptr;
        newExpr->arguments.push_back(sizeExpr);
        if (!expect(stream, tokens::TokenType::RIGHT_BRACKET, "']'"))
          return nullptr;
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
      // Support chained member and index access: this.foo.bar[0]
      Shared(ast::Expr) object = thisExpr;
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
            expectedError(stream, "a member name after '.'");
            return nullptr;
          }
        } else if (stream.peek().getType() == tokens::TokenType::LESS &&
                   skipTypeArguments(stream)) {
          continue;  // explicit type arguments: f<int>(x)
        } else if (stream.peek().getType() == tokens::TokenType::LEFT_BRACKET) {
          auto lb = stream.peek();
          stream.advance();
          auto idxExpr = build(stream);
          if (!idxExpr) return nullptr;
          if (!expect(stream, tokens::TokenType::RIGHT_BRACKET, "']'"))
            return nullptr;
          auto idx = std::make_shared<ast::IndexAccessExpr>();
          idx->object = object;
          idx->index = idxExpr;
          idx->location = lb.getLocation();
          object = idx;
          continue;
        } else {
          break;
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
            expectedError(stream, "a member name after '.'");
            return nullptr;
          }
        } else if (stream.peek().getType() == tokens::TokenType::LESS &&
                   skipTypeArguments(stream)) {
          continue;  // explicit type arguments: f<int>(x)
        } else if (stream.peek().getType() == tokens::TokenType::LEFT_BRACKET) {
          // Index access: obj[expr]
          auto lb = stream.peek();
          stream.advance();
          auto indexExpr = build(stream);
          if (!indexExpr) return nullptr;
          if (!expect(stream, tokens::TokenType::RIGHT_BRACKET, "']'"))
            return nullptr;
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
      if (!expr) return nullptr;
      if (!expect(stream, tokens::TokenType::RIGHT_PAREN, "')'"))
        return nullptr;
      return expr;
    }
      // Handle unary operators: -expr, !expr, @expr (address-of), *expr
      // (dereference)
    case tokens::TokenType::MINUS:
    case tokens::TokenType::PLUS:    // Unary plus (numeric identity)
    case tokens::TokenType::EXCLAIM:
    case tokens::TokenType::TILDE:   // Bitwise NOT (~)
    case tokens::TokenType::AT:      // Address-of operator (@)
    case tokens::TokenType::STAR: {  // Dereference operator (*)
      auto op = stream.peek();
      stream.advance();
      auto right = buildPostfix(stream);
      if (!right) return nullptr;
      auto expr = std::make_shared<ast::UnaryExpr>();
      expr->op = op;
      expr->operand = right;
      expr->location = op.getLocation();
      return expr;
    }
    // Prefix increment/decrement: ++expr, --expr
    case tokens::TokenType::PLUS_PLUS:
    case tokens::TokenType::MINUS_MINUS: {
      auto op = stream.peek();
      stream.advance();
      auto expr = std::make_shared<ast::UpdateExpr>();
      expr->op = op;
      expr->operand = buildPostfix(stream);
      if (!expr->operand) return nullptr;
      expr->isPrefix = true;
      expr->location = op.getLocation();
      return expr;
    }
    default:
      // Not the start of an expression. The token is left in place; the
      // enclosing statement list resynchronizes.
      expectedError(stream, "an expression");
      return nullptr;
  }
}

// Parse a primary expression followed by any postfix '++' / '--'
Shared(ast::Expr) ExpressionBuilder::buildPostfix(tokens::TokenStream& stream) {
  auto expr = buildPrimary(stream);
  while (expr && (stream.peek().getType() == tokens::TokenType::PLUS_PLUS ||
                  stream.peek().getType() == tokens::TokenType::MINUS_MINUS)) {
    auto update = std::make_shared<ast::UpdateExpr>();
    update->op = stream.peek();
    update->operand = expr;
    update->isPrefix = false;
    update->location = expr->location;
    stream.advance();
    expr = update;
  }
  return expr;
}

// Parse a binary expression using precedence climbing
Shared(ast::Expr) ExpressionBuilder::buildBinary(tokens::TokenStream& stream,
                                                 int minPrec) {
  auto left = buildPostfix(stream);
  if (!left) return nullptr;
  while (true) {
    auto type = stream.peek().getType();
    // Ternary conditional: cond ? thenExpr : elseExpr. Binds looser than
    // '||' and is right-associative, like assignment.
    if (type == tokens::TokenType::QUESTION) {
      int prec = getOperatorPrecedence(type);
      if (prec < minPrec) break;
      auto q = stream.peek();
      stream.advance();  // consume '?'
      auto cond = std::make_shared<ast::ConditionalExpr>();
      cond->condition = left;
      cond->thenExpr = build(stream);
      if (!cond->thenExpr) return nullptr;
      if (!expect(stream, tokens::TokenType::COLON,
                  "':' in conditional expression"))
        return nullptr;
      // Like TS, each branch is an assignment expression.
      cond->elseExpr = buildBinary(stream, kPrecAssignment);
      if (!cond->elseExpr) return nullptr;
      cond->location = q.getLocation();
      left = cond;
      continue;
    }
    if (!isBinaryOperator(type)) break;
    int prec = getOperatorPrecedence(type);
    if (prec < minPrec) break;
    auto op = stream.peek();
    stream.advance();
    // Compound assignment is right-associative like '='
    if (isCompoundAssignmentOperator(type)) {
      auto assignExpr = std::make_shared<ast::CompoundAssignmentExpr>();
      assignExpr->target = left;
      assignExpr->op = op;
      assignExpr->value = buildBinary(stream, prec);
      if (!assignExpr->value) return nullptr;
      assignExpr->location = op.getLocation();
      left = assignExpr;
      continue;
    }
    // Optionally consume generic arguments: Identifier '<' ... '>'
    if (stream.peek().getType() == tokens::TokenType::LESS) {
      int depth = 0;
      do {
        if (stream.peek().getType() == tokens::TokenType::LESS) depth++;
        if (stream.peek().getType() == tokens::TokenType::GREATER) depth--;
        stream.advance();
      } while (!stream.isAtEnd() && depth > 0);
    }

    // Special handling for assignment operators
    if (type == tokens::TokenType::EQUALS) {
      // Assignment is right-associative: parse RHS with the same precedence
      // so that chains like x = y = 5 associate as x = (y = 5)
      auto right = buildBinary(stream, prec);
      if (!right) return nullptr;
      auto assignExpr = std::make_shared<ast::AssignmentExpr>();
      assignExpr->target = left;
      assignExpr->value = right;
      assignExpr->location = op.getLocation();
      left = assignExpr;
    } else {
      // Regular binary expression
      auto right = buildBinary(stream, prec + 1);
      if (!right) return nullptr;
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
    // Best-effort consume a simple type: either a basic type token or
    // identifier
    auto tt = stream.peek().getType();
    if (tt == tokens::TokenType::IDENTIFIER || tt == tokens::TokenType::INT ||
        tt == tokens::TokenType::FLOAT || tt == tokens::TokenType::BOOLEAN ||
        tt == tokens::TokenType::STRING || tt == tokens::TokenType::VOID) {
      stream.advance();
      // Optionally consume array suffixes: [] ...
      while (stream.peek().getType() == tokens::TokenType::LEFT_BRACKET &&
             stream.peekNext().getType() == tokens::TokenType::RIGHT_BRACKET) {
        stream.advance();  // '['
        stream.advance();  // ']'
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
    if (!arg) return nullptr;
    call->arguments.push_back(arg);
    if (stream.peek().getType() == tokens::TokenType::COMMA) {
      stream.advance();
    } else {
      break;
    }
  }
  if (!expect(stream, tokens::TokenType::RIGHT_PAREN,
              "',' or ')' in argument list"))
    return nullptr;
  return call;
}

// Binary operator precedence, higher number = binds tighter. Matches
// TypeScript/C (docs/grammar.md, "Operator precedence"):
//   * / %  >  + -  >  << >>  >  < <= > >=  >  == !=  >  &  >  ^  >  |
//   >  &&  >  ||  >  ?:  >  = op=
// Unary operators (- + ! ~ @ * ++ --) bind tighter than all of these: they
// are parsed in buildPrimary. Assignment and '?:' are right-associative.
int ExpressionBuilder::getOperatorPrecedence(tokens::TokenType type) {
  switch (type) {
    case tokens::TokenType::STAR:
    case tokens::TokenType::SLASH:
    case tokens::TokenType::PERCENT:
      return kPrecMultiplicative;
    case tokens::TokenType::PLUS:
    case tokens::TokenType::MINUS:
      return kPrecAdditive;
    case tokens::TokenType::LEFT_SHIFT:
    case tokens::TokenType::RIGHT_SHIFT:
      return kPrecShift;
    case tokens::TokenType::LESS:
    case tokens::TokenType::LESS_EQUALS:
    case tokens::TokenType::GREATER:
    case tokens::TokenType::GREATER_EQUALS:
      return kPrecRelational;
    case tokens::TokenType::EQUALS_EQUALS:
    case tokens::TokenType::EXCLAIM_EQUALS:
      return kPrecEquality;
    case tokens::TokenType::AMPERSAND:
      return kPrecBitAnd;
    case tokens::TokenType::CARET:
      return kPrecBitXor;
    case tokens::TokenType::PIPE:
      return kPrecBitOr;
    case tokens::TokenType::AMPERSAND_AMPERSAND:
      return kPrecLogicalAnd;
    case tokens::TokenType::PIPE_PIPE:
      return kPrecLogicalOr;
    case tokens::TokenType::QUESTION:
      return kPrecConditional;
    case tokens::TokenType::EQUALS:
    case tokens::TokenType::PLUS_EQUALS:
    case tokens::TokenType::MINUS_EQUALS:
    case tokens::TokenType::STAR_EQUALS:
    case tokens::TokenType::SLASH_EQUALS:
    case tokens::TokenType::PERCENT_EQUALS:
    case tokens::TokenType::AMPERSAND_EQUALS:
    case tokens::TokenType::PIPE_EQUALS:
    case tokens::TokenType::CARET_EQUALS:
      return kPrecAssignment;
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
      return isCompoundAssignmentOperator(type);
  }
}

// Returns true if the token is a compound assignment operator
bool ExpressionBuilder::isCompoundAssignmentOperator(tokens::TokenType type) {
  switch (type) {
    case tokens::TokenType::PLUS_EQUALS:
    case tokens::TokenType::MINUS_EQUALS:
    case tokens::TokenType::STAR_EQUALS:
    case tokens::TokenType::SLASH_EQUALS:
    case tokens::TokenType::PERCENT_EQUALS:
    case tokens::TokenType::AMPERSAND_EQUALS:
    case tokens::TokenType::PIPE_EQUALS:
    case tokens::TokenType::CARET_EQUALS:
      return true;
    default:
      return false;
  }
}

}  // namespace parser
