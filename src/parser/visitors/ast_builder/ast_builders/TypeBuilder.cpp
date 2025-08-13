#include "TypeBuilder.h"

#include <iostream>
#include <memory>

#include "core/common/macros.h"

namespace parser {

Shared(ast::TypeNode) TypeBuilder::build(tokens::TokenStream& stream) {
  if (stream.isAtEnd()) return nullptr;
  auto type = stream.peek().getType();

  // Handle pointer types: [ '@' ('unsafe' | 'aligned') ] Type '*'
  if (type == tokens::TokenType::AT) {
    stream.advance();  // consume '@'
    auto modType = stream.peek().getType();
    tokens::Token modifier;
    if (modType == tokens::TokenType::UNSAFE ||
        modType == tokens::TokenType::ALIGNED) {
      modifier = stream.peek();
      stream.advance();
    }
    auto baseType = buildBasic(stream);
    if (!baseType) {
      // base type build failed
    }
    if (stream.peek().getType() == tokens::TokenType::STAR) {
      stream.advance();  // consume '*'
      auto ptrNode = std::make_shared<ast::PointerTypeNode>();
      ptrNode->qualifier = modifier;
      ptrNode->baseType = baseType;
      ptrNode->location =
          baseType ? baseType->location : tokens::Token().getLocation();
      return ptrNode;
    }
    // expected '*'
    return nullptr;
  }

  // Handle smart pointer types: '#' ('shared' | 'unique' | 'weak') '<' Type '>'
  if (type == tokens::TokenType::ATTRIBUTE) {
    stream.advance();  // consume '#'
    auto smartType = stream.peek().getType();
    if (smartType == tokens::TokenType::SHARED ||
        smartType == tokens::TokenType::UNIQUE ||
        smartType == tokens::TokenType::WEAK) {
      auto smartToken = stream.peek();
      stream.advance();
      if (stream.peek().getType() == tokens::TokenType::LESS) {
        stream.advance();  // consume '<'
        auto innerType = build(stream);
        if (stream.peek().getType() == tokens::TokenType::GREATER) {
          stream.advance();  // consume '>'
          auto smartPtrNode = std::make_shared<ast::SmartPointerTypeNode>();
          smartPtrNode->kind = smartToken;
          smartPtrNode->target = innerType;
          smartPtrNode->location = smartToken.getLocation();
          return smartPtrNode;
        }
      }
    }
    // Error: not a valid smart pointer type
    return nullptr;
  }

  // Handle basic types and identifiers
  if (isBasicType(type) || type == tokens::TokenType::IDENTIFIER) {
    // Peek identifier or primitive
    auto startTok = stream.peek();
    auto baseBasic = buildBasic(stream);
    if (!baseBasic) return nullptr;

    // Convert to TypeNode for further processing
    Shared(ast::TypeNode) resultType =
        std::static_pointer_cast<ast::TypeNode>(baseBasic);

    // Handle generic type application: Identifier '<' Type {',' Type} '>'
    if (startTok.getType() == tokens::TokenType::IDENTIFIER &&
        stream.peek().getType() == tokens::TokenType::LESS) {
      stream.advance();  // consume '<'
      auto genNode = std::make_shared<ast::GenericTypeNode>();
      genNode->name = startTok;
      genNode->location = startTok.getLocation();
      // Parse one or more type arguments until '>'
      while (!stream.isAtEnd() && stream.peek().getType() != tokens::TokenType::GREATER) {
        auto argTy = build(stream);
        if (argTy) genNode->typeArgs.push_back(argTy);
        if (stream.peek().getType() == tokens::TokenType::COMMA) {
          stream.advance();  // consume ',' and continue
        } else {
          break;
        }
      }
      if (stream.peek().getType() == tokens::TokenType::GREATER)
        stream.advance();  // consume '>' if present
      resultType = genNode;
    }

    // Check for pointer suffix chain: Type '*'* (without @ prefix)
    while (stream.peek().getType() == tokens::TokenType::STAR) {
      stream.advance();  // consume '*'
      auto ptrNode = std::make_shared<ast::PointerTypeNode>();
      ptrNode->qualifier = tokens::Token();  // no qualifier for suffix syntax
      ptrNode->baseType = resultType;
      ptrNode->location = resultType->location;
      resultType = ptrNode;
    }

  // Check for union types: Type '|' Type { '|' Type }
  if (stream.peek().getType() == tokens::TokenType::PIPE) {
      auto unionNode = std::make_shared<ast::UnionTypeNode>();
      unionNode->types.push_back(resultType);
      unionNode->location = resultType->location;

      while (stream.peek().getType() == tokens::TokenType::PIPE) {
        stream.advance();  // consume '|'
        auto nextType = build(stream);
        if (!nextType) break;
        unionNode->types.push_back(nextType);
      }

      resultType = unionNode;
    }

    // Check for type constraints: Type [ 'extends' Type ]
    if (stream.peek().getType() == tokens::TokenType::EXTENDS) {
      stream.advance();  // consume 'extends'
      auto constraintType = build(stream);
      if (constraintType) {
        auto constraintNode = std::make_shared<ast::TypeConstraintNode>();
        constraintNode->base = resultType;
        constraintNode->constraint = constraintType;
        constraintNode->location = resultType->location;
        return constraintNode;
      }
    }

    return resultType;
  }

  // Unrecognized type
  stream.advance();
  return nullptr;
}

// Parse a basic type (int, float, boolean, string, or identifier)
Shared(ast::BasicTypeNode)
    TypeBuilder::buildBasic(tokens::TokenStream& stream) {
  if (stream.isAtEnd()) return nullptr;
  auto type = stream.peek().getType();
  if (isBasicType(type) || type == tokens::TokenType::IDENTIFIER) {
    auto basicTypeNode = std::make_shared<ast::BasicTypeNode>();
    basicTypeNode->name = stream.peek();
    basicTypeNode->location = stream.peek().getLocation();
    stream.advance();
    return basicTypeNode;
  }
  // Unrecognized basic type
  stream.advance();
  return nullptr;
}

// Parse a pointer type (already handled in build)
Shared(ast::PointerTypeNode) TypeBuilder::buildPointer(
    tokens::TokenStream& stream, Shared(ast::TypeNode) baseType) {
  // Not used directly; handled in build()
  return nullptr;
}

// Parse a smart pointer type (already handled in build)
Shared(ast::SmartPointerTypeNode)
    TypeBuilder::buildSmartPointer(tokens::TokenStream& stream) {
  // Not used directly; handled in build()
  return nullptr;
}

// Utility: is this a basic type?
bool TypeBuilder::isBasicType(tokens::TokenType type) {
  switch (type) {
    case tokens::TokenType::INT:
    case tokens::TokenType::FLOAT:
    case tokens::TokenType::BOOLEAN:
    case tokens::TokenType::STRING:
    case tokens::TokenType::VOID:
      return true;
    default:
      return false;
  }
}

}  // namespace parser
