#include "TypeBuilder.h"

#include <iostream>
#include <memory>

#include "core/common/macros.h"

namespace parser {

Shared(ast::TypeNode) TypeBuilder::build(tokens::TokenStream& stream) {
  if (stream.isAtEnd()) return nullptr;
  auto type = stream.peek().getType();

  switch (type) {
    case tokens::TokenType::INT:
    case tokens::TokenType::FLOAT:
    case tokens::TokenType::BOOLEAN:
    case tokens::TokenType::STRING:
    case tokens::TokenType::IDENTIFIER:
      // Handle basic types
      return buildBasic(stream);
    // case tokens::TokenType::POINTER:
    //   // Handle pointer types
    //   stream.advance();  // consume 'pointer'
    //   if (stream.isAtEnd()) return nullptr;
    //   std::cout << "Building pointer type at position: "
    //             << stream.getCurrentPosition() << std::endl;
    //   if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
    //     // Error: expected identifier after 'pointer'
    //     stream.advance();  // Skip faulty token
    //     return nullptr;
    //   }
    //   return buildPointer(stream, TypeBuilder::build(stream));
    // case tokens::TokenType::SHARED_PTR:
    // case tokens::TokenType::UNIQUE_PTR:
    // case tokens::TokenType::WEAK_PTR:
    //   // Handle smart pointer types
    //   stream.advance();  // consume 'shared_ptr', 'unique_ptr', or 'weak_ptr
    //   std::cout << "Building smart pointer type at position: "
    //             << stream.getCurrentPosition() << std::endl;
    //   return buildSmartPointer(stream);
    default:
      stream.advance();  // Skip faulty token
      return nullptr;
  }
}

Shared(ast::BasicTypeNode)
    TypeBuilder::buildBasic(tokens::TokenStream& stream) {
  if (stream.isAtEnd()) return nullptr;
  auto type = stream.peek().getType();
  switch (type) {
    case tokens::TokenType::INT:
    case tokens::TokenType::FLOAT:
    case tokens::TokenType::BOOLEAN:
    case tokens::TokenType::STRING:
      // Create a BasicTypeNode for basic types
      {
        auto basicTypeNode = std::make_shared<ast::BasicTypeNode>();
        basicTypeNode->name = stream.peek();
        basicTypeNode->location = stream.peek().getLocation();
        stream.advance();  // consume the basic type token
        return basicTypeNode;
      }
    default:
      // Unrecognized basic type, advance and return nullptr
      stream.advance();  // Skip faulty token
      return nullptr;
  }
}

Shared(ast::PointerTypeNode) TypeBuilder::buildPointer(
    tokens::TokenStream& stream, Shared(ast::TypeNode) baseType) {
  return nullptr;
}

Shared(ast::SmartPointerTypeNode)
    TypeBuilder::buildSmartPointer(tokens::TokenStream& stream) {
  return nullptr;
}

bool TypeBuilder::isBasicType(tokens::TokenType type) {
  return false;
}

}  // namespace parser
