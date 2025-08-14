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
  // Note: semantic analysis is handled by the compiler driver (main.cpp).
  // Always return the raw AST here so callers can decide when/how to analyze.
  return program;
}

}  // namespace parser
