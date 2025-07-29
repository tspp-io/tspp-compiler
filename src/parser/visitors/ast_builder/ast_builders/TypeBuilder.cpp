#include "TypeBuilder.h"
#include "core/common/macros.h"
#include <memory>

namespace parser {

Shared(ast::TypeNode) TypeBuilder::build(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::BasicTypeNode) TypeBuilder::buildBasic(tokens::TokenStream& stream) {
    return nullptr;
}

Shared(ast::PointerTypeNode) TypeBuilder::buildPointer(tokens::TokenStream& stream, Shared(ast::TypeNode) baseType) {
    return nullptr;
}

Shared(ast::SmartPointerTypeNode) TypeBuilder::buildSmartPointer(tokens::TokenStream& stream) {
    return nullptr;
}

bool TypeBuilder::isBasicType(tokens::TokenType type) {
    return false;
}

} // namespace parser
