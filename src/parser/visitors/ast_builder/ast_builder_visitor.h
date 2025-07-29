#pragma once
#include "../base_visitor.h"
#include "parser/nodes/meta_nodes.h"
#include "tokens/stream/token_stream.h"

namespace parser {

/**
 * @brief Visitor that builds AST nodes from token stream
 * Drives the parsing process by delegating to specialized builders
 */
class ASTBuilderVisitor : public BaseVisitor {
 public:
  explicit ASTBuilderVisitor(tokens::TokenStream& stream);

  // Override visitor methods for building AST
  void visit(ast::ProgramNode& node) override;

 private:
  tokens::TokenStream& stream_;

  // Helper methods for parsing different constructs
  bool isDeclarationStart() const;
  bool isStatementStart() const;

  void parseTopLevel(ast::ProgramNode& program);
};

}  // namespace parser
