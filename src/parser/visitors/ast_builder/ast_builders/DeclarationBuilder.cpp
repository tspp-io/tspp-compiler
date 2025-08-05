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
      return buildFunction(stream, functionModifier);
    case tokens::TokenType::CLASS:
      return buildClass(stream);
    case tokens::TokenType::INTERFACE:
      return buildInterface(stream);
    case tokens::TokenType::TYPEDEF:
      // Handle type alias declaration
      return buildTypeDef(stream);
    default:
      return nullptr;
  }
}

Shared(ast::TypeAliasDecl)
    DeclarationBuilder::buildTypeDef(tokens::TokenStream& stream) {
  if (stream.peek().getType() != tokens::TokenType::TYPEDEF) {
    return nullptr;
  }
  stream.advance();  // consume 'typedef'

  if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  tokens::Token typeName = stream.peek();
  auto typeLoc = stream.peek().getLocation();
  stream.advance();  // consume identifier

  if (stream.peek().getType() != tokens::TokenType::EQUALS) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  stream.advance();  // consume '='

  Shared(ast::TypeNode) typeNode = TypeBuilder::build(stream);
  if (!typeNode) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }

  if (stream.peek().getType() != tokens::TokenType::SEMICOLON) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  stream.advance();  // consume ';'

  auto typeDefDecl = std::make_shared<ast::TypeAliasDecl>();
  typeDefDecl->name = typeName;
  typeDefDecl->aliasedType = typeNode;
  typeDefDecl->location = typeLoc;

  return typeDefDecl;
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
    // Allow both built-in types and custom type identifiers (typedefs)
    if (stream.peek().isType() ||
        stream.peek().getType() == tokens::TokenType::IDENTIFIER) {
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
  //   ## Functions
  // ```
  // FunctionDecl   ::= [ FunctionModifier ] 'function'
  // Identifier '(' [ ParameterList ] ')' [ ':' Type ] Block
  // FunctionModifier       ::= '#' ('const' | 'constexpr' |
  // 'zerocast' | 'simd' | 'prefetch' | 'atomic' | 'pinned')
  // ParameterList  ::= Parameter { ',' Parameter }
  // Parameter      ::= Type Identifier
  // ```

  if (stream.peek().getType() != tokens::TokenType::FUNCTION) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  stream.advance();  // consume 'function'

  if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  tokens::Token funcName = stream.peek();
  auto funcLoc = stream.peek().getLocation();
  auto funcDecl = std::make_shared<ast::FunctionDecl>();
  funcDecl->modifier = modifier;
  funcDecl->name = funcName;
  funcDecl->location = funcLoc;
  stream.advance();  // consume identifier

  if (stream.peek().getType() != tokens::TokenType::LEFT_PAREN) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  stream.advance();  // consume '('

  // Parse parameter list
  while (stream.peek().getType() != tokens::TokenType::RIGHT_PAREN &&
         !stream.isAtEnd()) {
    if (stream.peek().getType() == tokens::TokenType::COMMA) {
      stream.advance();  // consume ','
      continue;          // skip empty parameters
    }
    auto param = std::make_shared<ast::Parameter>();
    if (stream.peek().getType() == tokens::TokenType::IDENTIFIER) {
      tokens::Token paramName = stream.peek();
      auto paramLoc = stream.peek().getLocation();
      stream.advance();  // consume identifier
      if (stream.peek().getType() != tokens::TokenType::COLON) {
        stream.advance();  // Skip faulty token
        return nullptr;
      }
      stream.advance();  // consume ':'
      Shared(ast::TypeNode) paramType = TypeBuilder::build(stream);
      if (!paramType) {
        stream.advance();  // Skip faulty token
        return nullptr;
      }
      param->name = paramName;
      param->type = paramType;
      param->location = paramLoc;
      funcDecl->params.push_back(param);
    }
  }
  if (stream.peek().getType() != tokens::TokenType::RIGHT_PAREN) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  stream.advance();  // consume ')'

  if (stream.peek().getType() == tokens::TokenType::COLON) {
    stream.advance();  // consume ':'
    funcDecl->returnType = TypeBuilder::build(stream);
    if (!funcDecl->returnType) {
      stream.advance();  // Skip faulty token
      return nullptr;
    }
  }

  if (stream.peek().getType() != tokens::TokenType::LEFT_BRACE) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  // Do NOT advance here; StatementBuilder::buildBlock should consume the '{'
  // and '}'
  auto blockStmt = StatementBuilder::buildBlock(stream);
  funcDecl->body = blockStmt;
  if (!funcDecl->body) {
    std::cerr << "Error: Function body is not properly closed at "
              << funcLoc.toString() << std::endl;
    return nullptr;
  }
  return funcDecl;
}

Shared(ast::ClassDecl) DeclarationBuilder::buildClass(
    tokens::TokenStream& stream, ast::ClassModifier modifier) {
  // grammar rules for class declaration
  //       ## Classes and Interfaces
  // ```
  // ClassDecl        ::= ClassModifiers 'class' Identifier [ 'extends'
  // Identifier ] Block ClassModifier  ::= '#' ('abstract' | 'packed' |
  // 'pinned') InterfaceDecl  ::= 'interface' Identifier Block TypeAliasDecl ::=
  // 'type' Identifier '=' Type ';'
  // ```

  if (stream.peek().getType() != tokens::TokenType::CLASS) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  stream.advance();  // consume 'class'
  if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  tokens::Token className = stream.peek();
  auto classLoc = stream.peek().getLocation();
  auto classDecl = std::make_shared<ast::ClassDecl>();

  stream.advance();  // consume identifier
  if (stream.peek().getType() == tokens::TokenType::EXTENDS) {
    stream.advance();  // consume 'extends'
    if (stream.peek().getType() == tokens::TokenType::IDENTIFIER) {
      classDecl->baseClass = stream.peek();
      stream.advance();  // consume identifier
    } else {
      stream.advance();  // Skip faulty token
      return nullptr;
    }
  }
  classDecl->modifier = modifier;
  classDecl->name = className;
  classDecl->location = classLoc;
  classDecl->body = std::make_shared<ast::BlockStmt>();
  if (stream.peek().getType() == tokens::TokenType::LEFT_BRACE) {
    stream.advance();  // consume '{'
    // while (stream.peek().getType() != tokens::TokenType::RIGHT_BRACE &&
    //        !stream.isAtEnd()) {
    //   auto stmt = StatementBuilder::build(stream);
    if (stream.peek().getType() == tokens::TokenType::LEFT_BRACE) {
      stream.advance();  // consume '{'
      // while (stream.peek().getType() != tokens::TokenType::RIGHT_BRACE &&
      //        !stream.isAtEnd()) {
      //   auto stmt = StatementBuilder::build(stream);
      //   if (stmt) {
      //     classDecl->body->statements.push_back(stmt);
      //   } else {
      //     stream.advance();  // Skip faulty token
      //   }
      // }
      // if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE    ) {
      //   stream.advance();  // consume '}'
      // } else {
      //   std::cerr << "Error: Class body is not properly closed at "
      //             << classLoc.toString() << std::endl;
      //   return nullptr;
      // }
      // } else {
      //   std::cerr << "Error: Class body is missing at " <<
      //   classLoc.toString()
      //             << std::endl;
      //   return nullptr;
      // }
      // if (classDecl->body->statements.empty()) {
      //   std::cerr << "Error: Class body is empty at " << classLoc.toString()
      //             << std::endl;
      //   return nullptr;
      // }
    }
    return classDecl;
  }
}

Shared(ast::InterfaceDecl)
    DeclarationBuilder::buildInterface(tokens::TokenStream& stream) {
  // grammar rules for interface declaration
  //   ## Classes and Interfaces
  // ```
  // ClassDecl        ::= ClassModifiers 'class' Identifier [ 'extends'
  // Identifier ] Block ClassModifier  ::= '#' ('abstract' | 'packed' |
  // 'pinned') InterfaceDecl  ::= 'interface' Identifier Block TypeAliasDecl ::=
  // 'type' Identifier '=' Type ';'
  // ```
  if (stream.peek().getType() != tokens::TokenType::INTERFACE) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  stream.advance();  // consume 'interface'
  if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
    stream.advance();  // Skip faulty token
    return nullptr;
  }
  tokens::Token interfaceName = stream.peek();
  auto interfaceLoc = stream.peek().getLocation();
  stream.advance();  // consume identifier

  auto interfaceDecl = std::make_shared<ast::InterfaceDecl>();
  interfaceDecl->name = interfaceName;
  interfaceDecl->location = interfaceLoc;
  interfaceDecl->body = std::make_shared<ast::BlockStmt>();

  // std::cout << "[DEBUG] Checking if next token is BLOCK for interface
  // body\n"; if (stream.peek().getType() == tokens::TokenType::LEFT_BRACE) {
  //   stream.advance();  // consume '{'
  //   // while (stream.peek().getType() != tokens::TokenType::RIGHT_BRACE &&
  //   //        !stream.isAtEnd()) {
  //   //   auto stmt = StatementBuilder::build(stream);
  //   //   if (stmt) {
  //   //     interfaceDecl->body->statements.push_back(stmt);
  //   //   } else {
  //   //     stream.advance();  // Skip faulty token
  //   //   }
  //   // }
  //   // if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
  //   //   stream.advance();  // consume '}'
  //   // } else {
  //   //   std::cerr << "Error: Interface body is not properly closed at "
  //   //             << interfaceLoc.toString() << std::endl;
  //   //   return nullptr;
  //   // }
  //   return interfaceDecl;
  // }
  // std::cerr << "Error: Interface body is missing at " <<
  // interfaceLoc.toString()
  //           << std::endl;
  // return nullptr;
  // // if (interfaceDecl->body->statements.empty()) {
  // //   std::cerr << "Error: Interface body is empty at "
  // //             << interfaceLoc.toString() << std::endl;
  // //   return nullptr;
  // // }
  return interfaceDecl;
}

}  // namespace parser
