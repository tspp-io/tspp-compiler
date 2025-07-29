#pragma once
#include "core/common/macros.h"
#include "parser/nodes/declaration_nodes.h"
#include "tokens/stream/token_stream.h"

namespace parser {

/**
 * @brief Builder for declaration AST nodes
 * Handles parsing of variable, function, class, interface declarations
 */
class DeclarationBuilder {
 public:
  /**
   * @brief Build any declaration from token stream
   * @param stream Token stream to parse from
   * @return Shared pointer to declaration node, or nullptr on error
   */
  static Shared(ast::Decl) build(tokens::TokenStream& stream);

  /**
   * @brief Build import declaration specifically
   * @param stream Token stream to parse from
   * @return Shared pointer to import declaration, or nullptr on error
   */
  static Shared(ast::ImportDecl) buildImport(tokens::TokenStream& stream);

  /**
   * @brief Build variable declaration
   * @param stream Token stream to parse from
   * @return Shared pointer to variable declaration, or nullptr on error
   */
  static Shared(ast::VarDecl) buildVariable(tokens::TokenStream& stream);

  /**
   * @brief Build function declaration
   * @param stream Token stream to parse from
   * @return Shared pointer to function declaration, or nullptr on error
   */
  static Shared(ast::FunctionDecl) buildFunction(tokens::TokenStream& stream);

  /**
   * @brief Build class declaration
   * @param stream Token stream to parse from
   * @return Shared pointer to class declaration, or nullptr on error
   */
  static Shared(ast::ClassDecl) buildClass(tokens::TokenStream& stream);

  /**
   * @brief Build interface declaration
   * @param stream Token stream to parse from
   * @return Shared pointer to interface declaration, or nullptr on error
   */
  static Shared(ast::InterfaceDecl) buildInterface(tokens::TokenStream& stream);
};

}  // namespace parser
