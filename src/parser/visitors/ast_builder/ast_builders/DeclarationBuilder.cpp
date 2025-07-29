#include "DeclarationBuilder.h"
#include "core/common/macros.h"
#include <memory>

namespace parser {

Shared(ast::Decl) DeclarationBuilder::build(tokens::TokenStream& stream) {
    // Simple stub implementation 
    return nullptr;
}

Shared(ast::ImportDecl) DeclarationBuilder::buildImport(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::VarDecl) DeclarationBuilder::buildVariable(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::FunctionDecl) DeclarationBuilder::buildFunction(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::ClassDecl) DeclarationBuilder::buildClass(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::InterfaceDecl) DeclarationBuilder::buildInterface(tokens::TokenStream& stream) {
    return nullptr;
}

} // namespace parser
