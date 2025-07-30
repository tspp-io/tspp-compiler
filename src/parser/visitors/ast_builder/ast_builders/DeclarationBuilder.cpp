#include "DeclarationBuilder.h"

#include <iostream>
#include <memory>

#include "ExpressionBuilder.h"
#include "StatementBuilder.h"
#include "TypeBuilder.h"
#include "core/common/macros.h"
#include "tokens/token_type.h"

namespace parser {

Shared(ast::Decl) DeclarationBuilder::build(tokens::TokenStream& stream) {
  if (stream.isAtEnd()) return nullptr;

  ast::StorageQualifier storageQualifier = ast::StorageQualifier::None;
  switch (stream.peek().getType()) {
    case tokens::TokenType::STACK:
      storageQualifier = ast::StorageQualifier::Stack;
      stream.advance();
      break;
    case tokens::TokenType::HEAP:
      storageQualifier = ast::StorageQualifier::Heap;
      stream.advance();
      break;
    case tokens::TokenType::STATIC:
      storageQualifier = ast::StorageQualifier::Static;
      stream.advance();
      break;
    default:
      break;
  }

  ast::FunctionModifier functionModifier = ast::FunctionModifier::None;

  switch (stream.peek().getType()) {
    case tokens::TokenType::CONST:
      functionModifier = ast::FunctionModifier::Const;
      stream.advance();
      break;
    case tokens::TokenType::CONST_EXPR:
      functionModifier = ast::FunctionModifier::Constexpr;
      stream.advance();
      break;
    case tokens::TokenType::ZEROCAST:
      functionModifier = ast::FunctionModifier::Zerocast;
      stream.advance();
      break;
    case tokens::TokenType::SIMD:
      functionModifier = ast::FunctionModifier::Simd;
      stream.advance();
      break;
    case tokens::TokenType::PREFETCH:
      functionModifier = ast::FunctionModifier::Prefetch;
      stream.advance();
      break;
    case tokens::TokenType::ATOMIC:
      functionModifier = ast::FunctionModifier::Atomic;
      stream.advance();
      break;
    case tokens::TokenType::PINNED:
      if (stream.peekNext().getType() == tokens::TokenType::FUNCTION) {
        // 'pinned' can be used as a function modifier
        functionModifier = ast::FunctionModifier::Pinned;
        stream.advance();
      }
      break;
    default:
      break;
  }

  ast::ClassModifier classModifier = ast::ClassModifier::None;
  switch (stream.peek().getType()) {
    case tokens::TokenType::ABSTRACT:
      classModifier = ast::ClassModifier::Abstract;
      stream.advance();
      break;
    case tokens::TokenType::PACKED:
      classModifier = ast::ClassModifier::Packed;
      stream.advance();
      break;
    case tokens::TokenType::PINNED:
      if (stream.peekNext().getType() == tokens::TokenType::CLASS) {
        classModifier = ast::ClassModifier::Pinned;
        stream.advance();
      }
      break;
  }

  switch (stream.peek().getType()) {
    case tokens::TokenType::LET:
    case tokens::TokenType::CONST:
      return buildVariable(stream, storageQualifier);
    case tokens::TokenType::FUNCTION:
      std::cout << "[DEBUG] Found function start: " << stream.peek().toString()
                << std::endl;
      return buildFunction(stream, functionModifier);
    case tokens::TokenType::CLASS:
      return buildClass(stream);
    case tokens::TokenType::INTERFACE:
      return buildInterface(stream);
    default:
      return nullptr;
  }
}

Shared(ast::ImportDecl)
    DeclarationBuilder::buildImport(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::IMPORT) {
    return nullptr;
  }
  stream.advance();  // consume 'import'

  if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  tokens::Token importName = stream.peek();
  auto importLoc = stream.peek().getLocation();
  stream.advance();  // consume identifier

  std::string fromModule;
  if (stream.peek().getType() == tokens::TokenType::FROM) {
    stream.advance();  // consume 'from'
    if (stream.peek().getType() == tokens::TokenType::STRING_LITERAL) {
      fromModule = stream.peek().getLexeme();
      stream.advance();  // consume string literal
    } else {
      stream.advance();  // Skip faulty token
    }
  }

  auto importDecl = std::make_shared<ast::ImportDecl>();
  importDecl->name = importName;
  importDecl->fromPath = fromModule;
  importDecl->location = importLoc;
  return importDecl;
}

Shared(ast::VarDecl) DeclarationBuilder::buildVariable(
    tokens::TokenStream& stream, ast::StorageQualifier storageQualifier) {
  if (stream.peek().getType() != tokens::TokenType::LET &&
      stream.peek().getType() != tokens::TokenType::CONST) {
    return nullptr;
  }

  bool isConst = (stream.peek().getType() == tokens::TokenType::CONST);

  stream.advance();  // consume 'let' or 'const'
  if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }

  tokens::Token varName = stream.peek();
  auto varLoc = stream.peek().getLocation();

  stream.advance();  // consume identifier
  Shared(ast::TypeNode) varType = nullptr;
  if (stream.peek().getType() == tokens::TokenType::COLON) {
    stream.advance();  // consume ':'
    if (stream.peek().isType()) {
      varType = TypeBuilder::build(stream);
      if (!varType) {
        stream.advance();  // Skip faulty token
        return nullptr;
      }
    }
  }

  Shared(ast::Expr) initializer = nullptr;
  if (stream.peek().getType() == tokens::TokenType::EQUALS) {
    stream.advance();  // consume '='
    initializer = ExpressionBuilder::build(stream);
    if (!initializer) {
      stream.advance();  // Skip faulty token
      return nullptr;
    }
  }
  if (stream.peek().getType() == tokens::TokenType::SEMICOLON) {
    stream.advance();  // consume ';'
  }
  auto varDecl = std::make_shared<ast::VarDecl>();
  varDecl->name = varName;
  varDecl->type = varType;
  varDecl->initializer = initializer;
  varDecl->qualifier = storageQualifier;
  varDecl->location = varLoc;
  return varDecl;
}

Shared(ast::FunctionDecl) DeclarationBuilder::buildFunction(
    tokens::TokenStream& stream, ast::FunctionModifier modifier) {
  // grammar rules for function declaration
  //  FunctionDecl   ::= [ Modifier ] 'function' Identifier '(' [ ParameterList
  //  ] ')' [ ':' Type ] Block Modifier       ::= '#' ('const' | 'constexpr' |
  //  'zerocast' | 'simd' | 'prefetch' | 'atomic' | 'pinned') ParameterList  ::=
  //  Parameter { ',' Parameter } Parameter      ::= Type Identifier
  std::cout << "[DEBUG] Checking if next token is FUNCTION\n";
  std::cout << "[DEBUG] Current token: " << stream.peek().toString()
            << std::endl;
  if (stream.peek().getType() != tokens::TokenType::FUNCTION) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  stream.advance();  // consume 'function'
  std::cout << "[DEBUG] Checking if next token is IDENTIFIER\n";
  if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  tokens::Token funcName = stream.peek();
  auto funcLoc = stream.peek().getLocation();
  stream.advance();  // consume identifier
  Shared(ast::TypeNode) returnType = nullptr;
  std::cout << "[DEBUG] Checking if next token is COLON for return type\n";
  if (stream.peek().getType() == tokens::TokenType::COLON) {
    stream.advance();  // consume ':'
    std::cout << "[DEBUG] Checking if next token is a type for return type\n";
    if (stream.peek().isType()) {
      returnType = TypeBuilder::build(stream);
      std::cout << "[DEBUG] Checking if returnType is valid\n";
      if (!returnType) {
        stream.advance();  // Skip faulty token
        return nullptr;
      }
    }
  }
  std::vector<Shared(ast::Parameter)> params;
  std::cout << "[DEBUG] Checking if next token is LEFT_PAREN for parameters\n";
  if (stream.peek().getType() == tokens::TokenType::LEFT_PAREN) {
    stream.advance();  // consume '('
    while (stream.peek().getType() != tokens::TokenType::RIGHT_PAREN) {
      std::cout
          << "[DEBUG] Checking if next token is IDENTIFIER for parameter\n";
      if (stream.peek().getType() == tokens::TokenType::IDENTIFIER) {
        tokens::Token paramName = stream.peek();
        auto paramLoc = stream.peek().getLocation();
        stream.advance();  // consume identifier
        Shared(ast::TypeNode) paramType = nullptr;
        std::cout
            << "[DEBUG] Checking if next token is COLON for parameter type\n";
        if (stream.peek().getType() == tokens::TokenType::COLON) {
          stream.advance();  // consume ':'
          std::cout
              << "[DEBUG] Checking if next token is a type for parameter\n";
          if (stream.peek().isType()) {
            paramType = TypeBuilder::build(stream);
            std::cout << "[DEBUG] Checking if paramType is valid\n";
            if (!paramType) {
              stream.advance();  // Skip faulty token
              return nullptr;
            }
          }
        }
        auto param = std::make_shared<ast::Parameter>();
        param->name = paramName;
        param->type = paramType;
        params.push_back(param);
      }
      std::cout
          << "[DEBUG] Checking if next token is COMMA between parameters\n";
      if (stream.peek().getType() == tokens::TokenType::COMMA) {
        stream.advance();  // consume ','
      } else {
        break;
      }
    }
    stream.advance();  // consume ')'
  }
  auto funcDecl = std::make_shared<ast::FunctionDecl>();
  funcDecl->modifier = modifier;
  funcDecl->name = funcName;
  funcDecl->returnType = returnType;
  funcDecl->params = params;
  funcDecl->location = funcLoc;
  funcDecl->body = std::make_shared<ast::BlockStmt>();
  std::cout << "[DEBUG] Checking if next token is BLOCK for function body\n";
  if (stream.peek().getType() == tokens::TokenType::LEFT_BRACE) {
    stream.advance();  // consume '{'
    stream.advance();  // consume '}'
    // while (stream.peek().getType() != tokens::TokenType::RIGHT_BRACE &&
    //        !stream.isAtEnd()) {
    //   auto stmt = StatementBuilder::build(stream);
    //   if (stmt) {
    //     funcDecl->body->statements.push_back(stmt);
    //   } else {
    //     stream.advance();  // Skip faulty token
    //   }
    // }
    // if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
    //   stream.advance();  // consume '}'
    // }

    // if (!funcDecl->body) {
    //   std::cerr << "Error: Function body is missing or invalid at "
    //             << funcLoc.toString() << std::endl;
    //   return nullptr;
    // }
    // if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
    //   stream.advance();  // consume '}'
    // } else {
    //   std::cerr << "Error: Function body is not properly closed at "
    //             << funcLoc.toString() << std::endl;
    //   return nullptr;
    // }
  }
  return funcDecl;
}

Shared(ast::ClassDecl) DeclarationBuilder::buildClass(
    tokens::TokenStream& stream, ast::ClassModifier modifier) {
  if (!stream.isAtEnd()) {
    stream.advance();
  }
  return nullptr;
}

Shared(ast::InterfaceDecl)
    DeclarationBuilder::buildInterface(tokens::TokenStream& stream) {
  if (!stream.isAtEnd()) {
    stream.advance();
  }
  return nullptr;
}

}  // namespace parser
