#include "StatementBuilder.h"
#include "core/common/macros.h"
#include <memory>

namespace parser {

Shared(ast::Stmt) StatementBuilder::build(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::BlockStmt) StatementBuilder::buildBlock(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::IfStmt) StatementBuilder::buildIf(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::WhileStmt) StatementBuilder::buildWhile(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::ForStmt) StatementBuilder::buildFor(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::ReturnStmt) StatementBuilder::buildReturn(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::ExprStmt) StatementBuilder::buildExpressionStatement(tokens::TokenStream& stream) {
    return nullptr;
}

} // namespace parser
