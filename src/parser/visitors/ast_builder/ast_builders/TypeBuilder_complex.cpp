#include "TypeBuilder.h"

#include <iostream>

#include "tokens/token_type.h"

namespace parser {

Shared(ast::TypeNode) TypeBuilder::build(tokens::TokenStream& stream) {
  // Check for smart pointer syntax (#shared, #unique, #weak)
  if (stream.check(tokens::TokenType::HASH) &&
      stream.peekNext(1).getType() == tokens::TokenType::IDENTIFIER) {
    auto next = stream.peekNext(1).lexeme;
    if (next == "shared" || next == "unique" || next == "weak") {
      return buildSmartPointer(stream);
    }
  }

  // Build basic type first
  auto baseType = buildBasic(stream);
  if (!baseType) {
    return nullptr;
  }

  // Check for pointer modifier
  if (stream.check(tokens::TokenType::MULTIPLY)) {
    return buildPointer(stream, baseType);
  }

  return baseType;
}

Shared(ast::BasicTypeNode)
    TypeBuilder::buildBasic(tokens::TokenStream& stream) {
  auto type = stream.peek().getType();

  if (isBasicType(type) || type == tokens::TokenType::IDENTIFIER) {
    auto basicType = MakeShared(ast::BasicTypeNode)();
    basicType->name = stream.advance();
    return basicType;
  }

  std::cerr << "Expected type, got: " << stream.peek().lexeme << std::endl;
  return nullptr;
}

Shared(ast::PointerTypeNode) TypeBuilder::buildPointer(
    tokens::TokenStream& stream, Shared(ast::TypeNode) baseType) {
  auto pointerType = MakeShared(ast::PointerTypeNode)();
  pointerType->baseType = baseType;

  // Consume '*'
  if (stream.match(tokens::TokenType::MULTIPLY)) {
    // For now, no qualifier handling
    pointerType->qualifier = tokens::Token{tokens::TokenType::IDENTIFIER, "",
                                           core::SourceLocation{}};
  }

  return pointerType;
}

Shared(ast::SmartPointerTypeNode)
    TypeBuilder::buildSmartPointer(tokens::TokenStream& stream) {
  // Expect '#'
  if (!stream.match(tokens::TokenType::HASH)) {
    return nullptr;
  }

  auto smartPtrType = MakeShared(ast::SmartPointerTypeNode)();

  // Expect kind (shared/unique/weak)
  if (stream.check(tokens::TokenType::IDENTIFIER)) {
    smartPtrType->kind = stream.advance();
  } else {
    std::cerr << "Expected smart pointer kind after '#'" << std::endl;
    return nullptr;
  }

  // Expect '<'
  if (!stream.match(tokens::TokenType::LESS)) {
    std::cerr << "Expected '<' after smart pointer kind" << std::endl;
    return nullptr;
  }

  // Parse target type
  smartPtrType->target = build(stream);
  if (!smartPtrType->target) {
    std::cerr << "Expected type in smart pointer template" << std::endl;
    return nullptr;
  }

  // Expect '>'
  if (!stream.match(tokens::TokenType::GREATER)) {
    std::cerr << "Expected '>' after smart pointer template" << std::endl;
    return nullptr;
  }

  return smartPtrType;
}

bool TypeBuilder::isBasicType(tokens::TokenType type) {
  return type == tokens::TokenType::INT || type == tokens::TokenType::FLOAT ||
         type == tokens::TokenType::BOOLEAN ||
         type == tokens::TokenType::STRING || type == tokens::TokenType::VOID;
}

}  // namespace parser
