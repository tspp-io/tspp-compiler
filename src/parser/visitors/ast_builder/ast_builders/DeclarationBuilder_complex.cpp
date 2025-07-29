#include "DeclarationBuilder.h"

#include <iostream>

#include "ExpressionBuilder.h"
#include "StatementBuilder.h"
#include "TypeBuilder.h"
#include "core/common/macros.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/statement_nodes.h"
#include "tokens/token_type.h"

namespace parser {

Shared(ast::Decl) DeclarationBuilder::build(tokens::TokenStream& stream) {
  auto type = stream.peek().getType();

  switch (type) {
    case tokens::TokenType::LET:
    case tokens::TokenType::CONST:
      return buildVariable(stream);

    case tokens::TokenType::FUNCTION:
      return buildFunction(stream);

    case tokens::TokenType::CLASS:
      return buildClass(stream);

    case tokens::TokenType::INTERFACE:
      return buildInterface(stream);

    default:
      std::cerr << "Unexpected token in declaration: " << stream.peek().getLexeme()
                << std::endl;
      return nullptr;
  }
}
}

Shared(ast::ImportDecl)
    DeclarationBuilder::buildImport(tokens::TokenStream& stream) {
  // Expect 'import'
  if (!stream.match(tokens::TokenType::IMPORT)) {
    return nullptr;
  }

  auto importDecl = std::make_shared<ast::ImportDecl>();

  // Expect identifier
  if (stream.check(tokens::TokenType::IDENTIFIER)) {
    importDecl->name = stream.advance();
  } else {
    std::cerr << "Expected identifier after 'import'" << std::endl;
    return nullptr;
  }

  // Optional 'from' clause
  if (stream.match(tokens::TokenType::FROM)) {
    if (stream.check(tokens::TokenType::STRING_LITERAL)) {
      importDecl->fromPath = stream.advance();
    } else {
      std::cerr << "Expected string literal after 'from'" << std::endl;
      return nullptr;
    }
  }

  // Expect semicolon
  if (!stream.match(tokens::TokenType::SEMICOLON)) {
    std::cerr << "Expected ';' after import statement" << std::endl;
  }

  return importDecl;
}

Shared(ast::VarDecl)
    DeclarationBuilder::buildVariable(tokens::TokenStream& stream) {
  auto varDecl = std::make_shared<ast::VarDecl>();

  // Handle storage qualifier (const/let)
  if (stream.match(tokens::TokenType::CONST)) {
    varDecl->qualifier =
        ast::StorageQualifier::Static;  // Treat const as static for now
  } else if (stream.match(tokens::TokenType::LET)) {
    varDecl->qualifier = ast::StorageQualifier::None;
  } else {
    return nullptr;
  }

  // Parse type (simplified for now)
  varDecl->type = TypeBuilder::build(stream);
  if (!varDecl->type) {
    std::cerr << "Expected type in variable declaration" << std::endl;
    return nullptr;
  }

  // Parse identifier
  if (stream.check(tokens::TokenType::IDENTIFIER)) {
    varDecl->name = stream.advance();
  } else {
    std::cerr << "Expected identifier in variable declaration" << std::endl;
    return nullptr;
  }

  // Optional initializer
  if (stream.match(tokens::TokenType::EQUALS)) {
    varDecl->initializer = ExpressionBuilder::build(stream);
    if (!varDecl->initializer) {
      std::cerr << "Expected expression after '=' in variable declaration"
                << std::endl;
      return nullptr;
    }
  }

  // Expect semicolon
  if (!stream.match(tokens::TokenType::SEMICOLON)) {
    std::cerr << "Expected ';' after variable declaration" << std::endl;
  }

  return varDecl;
}

Shared(ast::FunctionDecl)
    DeclarationBuilder::buildFunction(tokens::TokenStream& stream) {
  // Expect 'function'
  if (!stream.match(tokens::TokenType::FUNCTION)) {
    return nullptr;
  }

  auto funcDecl = std::make_shared<ast::FunctionDecl>();
  funcDecl->modifier = ast::FunctionModifier::None;

  // Parse identifier
  if (stream.check(tokens::TokenType::IDENTIFIER)) {
    funcDecl->name = stream.advance();
  } else {
    std::cerr << "Expected function name" << std::endl;
    return nullptr;
  }

  // Parse parameter list
  if (!stream.match(tokens::TokenType::LEFT_PAREN)) {
    std::cerr << "Expected '(' after function name" << std::endl;
    return nullptr;
  }

  // TODO: Parse parameters

  if (!stream.match(tokens::TokenType::RIGHT_PAREN)) {
    std::cerr << "Expected ')' after parameters" << std::endl;
    return nullptr;
  }

  // Optional return type
  if (stream.match(tokens::TokenType::COLON)) {
    funcDecl->returnType = TypeBuilder::build(stream);
  }

  // Parse body
  funcDecl->body = StatementBuilder::buildBlock(stream);

  return funcDecl;
}

Shared(ast::ClassDecl)
    DeclarationBuilder::buildClass(tokens::TokenStream& stream) {
  // Expect 'class'
  if (!stream.match(tokens::TokenType::CLASS)) {
    return nullptr;
  }

  auto classDecl = std::make_shared<ast::ClassDecl>();
  classDecl->modifier = ast::FunctionModifier::None;

  // Parse identifier
  if (stream.check(tokens::TokenType::IDENTIFIER)) {
    classDecl->name = stream.advance();
  } else {
    std::cerr << "Expected class name" << std::endl;
    return nullptr;
  }

  // TODO: Parse inheritance, body
  classDecl->body = StatementBuilder::buildBlock(stream);

  return classDecl;
}

Shared(ast::InterfaceDecl)
    DeclarationBuilder::buildInterface(tokens::TokenStream& stream) {
  // Expect 'interface'
  if (!stream.match(tokens::TokenType::INTERFACE)) {
    return nullptr;
  }

  auto interfaceDecl = std::make_shared<ast::InterfaceDecl>();

  // Parse identifier
  if (stream.check(tokens::TokenType::IDENTIFIER)) {
    interfaceDecl->name = stream.advance();
  } else {
    std::cerr << "Expected interface name" << std::endl;
    return nullptr;
  }

  // TODO: Parse body
  interfaceDecl->body = StatementBuilder::buildBlock(stream);

  return interfaceDecl;
}

}  // namespace parser
