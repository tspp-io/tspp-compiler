#include "parser.h"

#include <iostream>
#include <memory>

#include "core/common/common_types.h"
#include "core/common/macros.h"
#include "parser/nodes/meta_nodes.h"
#include "parser/visitors/ast_builder/ast_builder_visitor.h"
#include "parser/visitors/semantic/SemanticAnalyzerVisitor.h"
#include "tokens/stream/token_stream.h"

using namespace ast;
using namespace tokens;

namespace parser {

Shared(ProgramNode) buildAST(const std::vector<Token>& tokens) {
  TokenStream stream(tokens);
  auto program = std::make_shared<ProgramNode>();
  ASTBuilderVisitor visitor(stream);
  visitor.visit(*program);
  // const auto& errors = visitor.getErrors();
  // if (!errors.empty()) {
  //   for (const auto& error : errors) {
  //     std::cerr << "AST build error: " << error << std::endl;
  //   }
  //   return nullptr;  // Return null if there are build errors
  // }
  SemanticAnalyzerVisitor semanticVisitor;
  program->accept(semanticVisitor);
  const auto& errors = semanticVisitor.getErrors();
  if (!errors.empty()) {
    for (const auto& error : errors) {
      std::cerr << "Semantic error: " << error << std::endl;
    }
    return nullptr;  // Return null if there are semantic errors
  }
  // If no semantic errors, return the built AST
  return program;
}

}  // namespace parser
