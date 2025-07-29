#pragma once
#include "parser/nodes/ast_visitor.h"

namespace parser {

/**
 * @brief Base visitor class for AST operations
 * Provides common functionality for all visitor implementations
 */
class BaseVisitor : public ast::ASTVisitor {
 public:
  virtual ~BaseVisitor() = default;

 protected:
  // Common visitor utilities can be added here
};

}  // namespace parser
