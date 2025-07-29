#pragma once
#include <vector>

#include "core/common/macros.h"
#include "parser/nodes/meta_nodes.h"
#include "tokens/tokens.h"

namespace parser {

/**
 * @brief Build AST from token stream
 * @param tokens Vector of tokens to parse
 * @return Shared pointer to root ProgramNode of the AST
 */
Shared(ast::ProgramNode) buildAST(const std::vector<tokens::Token>& tokens);

}  // namespace parser
