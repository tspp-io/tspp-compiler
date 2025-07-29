#pragma once
#include "core/common/macros.h"
#include "parser/nodes/type_nodes.h"
#include "tokens/stream/token_stream.h"

namespace parser {

/**
 * @brief Builder for type AST nodes
 * Handles parsing of basic types, pointer types, smart pointer types
 */
class TypeBuilder {
 public:
  /**
   * @brief Build any type from token stream
   * @param stream Token stream to parse from
   * @return Shared pointer to type node, or nullptr on error
   */
  static Shared(ast::TypeNode) build(tokens::TokenStream& stream);

  /**
   * @brief Build basic type (int, float, bool, string, identifier)
   * @param stream Token stream to parse from
   * @return Shared pointer to basic type node, or nullptr on error
   */
  static Shared(ast::BasicTypeNode) buildBasic(tokens::TokenStream& stream);

  /**
   * @brief Build pointer type
   * @param stream Token stream to parse from
   * @param baseType Base type being pointed to
   * @return Shared pointer to pointer type node, or nullptr on error
   */
  static Shared(ast::PointerTypeNode)
      buildPointer(tokens::TokenStream& stream, Shared(ast::TypeNode) baseType);

  /**
   * @brief Build smart pointer type
   * @param stream Token stream to parse from
   * @return Shared pointer to smart pointer type node, or nullptr on error
   */
  static Shared(ast::SmartPointerTypeNode)
      buildSmartPointer(tokens::TokenStream& stream);

  /**
   * @brief Check if token represents a basic type
   * @param type Token type to check
   * @return true if basic type, false otherwise
   */
  static bool isBasicType(tokens::TokenType type);
};

}  // namespace parser
