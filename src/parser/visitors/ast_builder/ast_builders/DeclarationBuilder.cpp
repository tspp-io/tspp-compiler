#include "DeclarationBuilder.h"

#include <iostream>
#include <memory>

#include "ExpressionBuilder.h"
#include "StatementBuilder.h"
#include "TypeBuilder.h"
#include "core/common/macros.h"
#include "tokens/token_type.h"

namespace parser {

Shared(ast::BaseNode) DeclarationBuilder::build(tokens::TokenStream& stream) {
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

  // Optional function modifier (applies only when followed by 'function')
  ast::FunctionModifier functionModifier = ast::FunctionModifier::None;
  if (tokens::isFunctionModifier(stream.peek().getType()) &&
      stream.peekNext().getType() == tokens::TokenType::FUNCTION) {
    switch (stream.peek().getType()) {
      case tokens::TokenType::CONST_FUNCTION:
        functionModifier = ast::FunctionModifier::Const;
        stream.advance();
        break;
      case tokens::TokenType::CONSTEXPR:
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
        functionModifier = ast::FunctionModifier::Pinned;
        stream.advance();
        break;
      default:
        break;
    }
  }

  ast::ClassModifier classModifier = ast::ClassModifier::None;
  std::vector<ast::ClassModifier> classModifiers;
  // Collect multiple leading class modifiers if present
  bool seenAnyClassMod = false;
  while (true) {
    auto tt = stream.peek().getType();
    ast::ClassModifier cm = ast::ClassModifier::None;
    bool isClassMod = false;
    if (tt == tokens::TokenType::ABSTRACT) {
      cm = ast::ClassModifier::Abstract;
      isClassMod = true;
    } else if (tt == tokens::TokenType::PACKED) {
      cm = ast::ClassModifier::Packed;
      isClassMod = true;
    } else if (tt == tokens::TokenType::PINNED) {
      if (stream.peekNext().getType() == tokens::TokenType::CLASS) {
        cm = ast::ClassModifier::Pinned;
        isClassMod = true;
      }
    } else if (tt == tokens::TokenType::FINAL) {
      cm = ast::ClassModifier::Final;
      isClassMod = true;
    }
    if (!isClassMod) break;
    stream.advance();
    seenAnyClassMod = true;
    classModifiers.push_back(cm);
  }
  if (seenAnyClassMod && !classModifiers.empty()) {
    classModifier = classModifiers.back();  // keep last as legacy single
  }

  switch (stream.peek().getType()) {
    case tokens::TokenType::FUNCTION: {
      return buildFunction(stream, functionModifier);
    }
    case tokens::TokenType::LET:
    case tokens::TokenType::CONST:
      return buildVariable(stream, storageQualifier);
    case tokens::TokenType::CLASS: {
      // Support class declarations with optional prior class modifiers
      return buildClass(stream, classModifier, classModifiers);
    }
    case tokens::TokenType::INTERFACE:
      return buildInterface(stream);
    case tokens::TokenType::TYPEDEF:
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

Shared(ast::Stmt) DeclarationBuilder::buildVariable(
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
  varDecl->isConst = isConst;
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
  // Accept identifier-like names including GET/SET keywords
  auto nameTokType = stream.peek().getType();
  if (!(nameTokType == tokens::TokenType::IDENTIFIER ||
        nameTokType == tokens::TokenType::GET ||
        nameTokType == tokens::TokenType::SET)) {
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
    tokens::TokenStream& stream, ast::ClassModifier modifier,
    const std::vector<ast::ClassModifier>& modifiers) {
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

  // Optional generic type parameters: < ... > (skip content for now)
  if (stream.peek().getType() == tokens::TokenType::LESS) {
    int depth = 0;
    do {
      if (stream.peek().getType() == tokens::TokenType::LESS) depth++;
      if (stream.peek().getType() == tokens::TokenType::GREATER) depth--;
      stream.advance();
    } while (!stream.isAtEnd() && depth > 0);
  }
  if (stream.peek().getType() == tokens::TokenType::EXTENDS) {
    stream.advance();  // consume 'extends'
    if (stream.peek().getType() == tokens::TokenType::IDENTIFIER) {
      classDecl->baseClass = stream.peek();
      stream.advance();  // consume identifier
      // Optional type arguments: < ... > (skip)
      if (stream.peek().getType() == tokens::TokenType::LESS) {
        int d = 0;
        do {
          if (stream.peek().getType() == tokens::TokenType::LESS) d++;
          if (stream.peek().getType() == tokens::TokenType::GREATER) d--;
          stream.advance();
        } while (!stream.isAtEnd() && d > 0);
      }
    } else {
      stream.advance();  // Skip faulty token
      return nullptr;
    }
  }

  // Optional implements list: implements Identifier [<...>] { , Identifier
  // [<...>] }
  if (stream.peek().getType() == tokens::TokenType::IMPLEMENTS) {
    stream.advance();
    while (!stream.isAtEnd()) {
      if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) break;
      classDecl->interfaces.push_back(stream.peek());
      stream.advance();
      // Skip optional type arguments
      if (stream.peek().getType() == tokens::TokenType::LESS) {
        int d = 0;
        do {
          if (stream.peek().getType() == tokens::TokenType::LESS) d++;
          if (stream.peek().getType() == tokens::TokenType::GREATER) d--;
          stream.advance();
        } while (!stream.isAtEnd() && d > 0);
      }
      if (stream.peek().getType() == tokens::TokenType::COMMA) {
        stream.advance();
        continue;
      }
      break;
    }
  }
  classDecl->modifier = modifier;
  classDecl->modifiers = modifiers;
  classDecl->name = className;
  classDecl->location = classLoc;
  classDecl->body = std::make_shared<ast::BlockStmt>();
  if (stream.peek().getType() != tokens::TokenType::LEFT_BRACE) {
    // No body provided; error out gracefully
    std::cerr << "Error: Class body is missing at " << classLoc.toString()
              << std::endl;
    return nullptr;
  }
  // Parse class body with members: fields, methods, constructor
  stream.advance();  // consume '{'
  while (!stream.isAtEnd() &&
         stream.peek().getType() != tokens::TokenType::RIGHT_BRACE) {
    // Skip stray semicolons
    if (stream.peek().getType() == tokens::TokenType::SEMICOLON) {
      stream.advance();
      continue;
    }

    // Capture optional access modifier for members
    ast::AccessModifier access = ast::AccessModifier::Default;
    if (tokens::isAccessModifier(stream.peek().getType())) {
      switch (stream.peek().getType()) {
        case tokens::TokenType::PUBLIC:
          access = ast::AccessModifier::Public;
          break;
        case tokens::TokenType::PRIVATE:
          access = ast::AccessModifier::Private;
          break;
        case tokens::TokenType::PROTECTED:
          access = ast::AccessModifier::Protected;
          break;
        default:
          break;
      }
      stream.advance();
    }

    // Collect optional attribute-style modifiers before a member
    // - Storage qualifiers for fields: #static/#heap/#stack
    // - Field modifiers: #readonly/#volatile
    // - Function modifiers for methods: #constexpr/#simd/#atomic/... (ignored
    //   if not a method)
    // - Method attributes: #inline/#virtual/#override (attach to methods)
    // - Generic attributes we don't recognize: skip safely
    ast::StorageQualifier memberStorage = ast::StorageQualifier::None;
    ast::FunctionModifier memberFuncMod = ast::FunctionModifier::None;
    std::vector<ast::FieldModifier> collectedFieldMods;
    std::vector<ast::MethodAttribute> collectedMethodAttrs;
    while (true) {
      auto t = stream.peek().getType();
      // Storage qualifiers (fields only)
      if (t == tokens::TokenType::STACK) {
        memberStorage = ast::StorageQualifier::Stack;
        stream.advance();
        continue;
      }
      if (t == tokens::TokenType::HEAP) {
        memberStorage = ast::StorageQualifier::Heap;
        stream.advance();
        continue;
      }
      // Handle 'static' carefully:
      // - 'static' followed by '{' is a static initialization block
      // - '#static' before a method should be treated as
      // MethodAttribute::Static
      // - '#static' before a field acts as StorageQualifier::Static
      if (t == tokens::TokenType::STATIC) {
        // Lookahead to decide usage without consuming other tokens eagerly
        auto nextT = stream.peekNext().getType();
        if (nextT == tokens::TokenType::LEFT_BRACE) {
          // static { ... }
          stream.advance();  // consume 'static'
          auto staticBody = StatementBuilder::buildBlock(stream);
          if (staticBody) {
            classDecl->body->statements.push_back(staticBody);
            // Static block consumed fully; move to next member
            continue;  // continue outer while-loop
          } else {
            // recovery
            continue;
          }
        }
        if (nextT == tokens::TokenType::FUNCTION) {
          // #static before a method: record as method attribute
          collectedMethodAttrs.push_back(ast::MethodAttribute::Static);
          stream.advance();  // consume 'static'
          continue;
        }
        // Otherwise treat as storage qualifier for fields
        memberStorage = ast::StorageQualifier::Static;
        stream.advance();
        continue;
      }
      // Field modifiers
      if (tokens::isFieldModifier(t)) {
        if (t == tokens::TokenType::READONLY)
          collectedFieldMods.push_back(ast::FieldModifier::Readonly);
        else if (t == tokens::TokenType::VOLATILE)
          collectedFieldMods.push_back(ast::FieldModifier::Volatile);
        stream.advance();
        continue;
      }
      // Treat #atomic and #constexpr before a field as field modifiers too
      if ((t == tokens::TokenType::ATOMIC ||
           t == tokens::TokenType::CONSTEXPR) &&
          (stream.peekNext().getType() == tokens::TokenType::LET ||
           stream.peekNext().getType() == tokens::TokenType::CONST ||
           stream.peekNext().getType() == tokens::TokenType::IDENTIFIER)) {
        if (t == tokens::TokenType::ATOMIC)
          collectedFieldMods.push_back(ast::FieldModifier::Atomic);
        else
          collectedFieldMods.push_back(ast::FieldModifier::Constexpr);
        stream.advance();
        continue;
      }
      // Function modifiers before methods
      if (tokens::isFunctionModifier(t)) {
        switch (t) {
          case tokens::TokenType::CONST_FUNCTION:
            memberFuncMod = ast::FunctionModifier::Const;
            break;
          case tokens::TokenType::CONSTEXPR:
            memberFuncMod = ast::FunctionModifier::Constexpr;
            break;
          case tokens::TokenType::ZEROCAST:
            memberFuncMod = ast::FunctionModifier::Zerocast;
            break;
          case tokens::TokenType::SIMD:
            memberFuncMod = ast::FunctionModifier::Simd;
            break;
          case tokens::TokenType::PREFETCH:
            memberFuncMod = ast::FunctionModifier::Prefetch;
            break;
          case tokens::TokenType::ATOMIC:
            memberFuncMod = ast::FunctionModifier::Atomic;
            break;
          case tokens::TokenType::PINNED:
            memberFuncMod = ast::FunctionModifier::Pinned;
            break;
          default:
            break;
        }
        stream.advance();
        continue;
      }
      // Method attributes
      if (tokens::isMethodAttribute(t) || t == tokens::TokenType::ABSTRACT) {
        switch (t) {
          case tokens::TokenType::INLINE:
            collectedMethodAttrs.push_back(ast::MethodAttribute::Inline);
            break;
          case tokens::TokenType::VIRTUAL:
            collectedMethodAttrs.push_back(ast::MethodAttribute::Virtual);
            break;
          case tokens::TokenType::OVERRIDE:
            collectedMethodAttrs.push_back(ast::MethodAttribute::Override);
            break;
          case tokens::TokenType::ABSTRACT:
            collectedMethodAttrs.push_back(ast::MethodAttribute::Abstract);
            break;
          default:
            break;
        }
        stream.advance();
        continue;
      }
      // Unrecognized attribute tokens: skip them so syntax like #inline doesn't
      // block parsing
      if (t == tokens::TokenType::ATTRIBUTE) {
        stream.advance();
        continue;
      }
      break;
    }

    // Methods: 'function' Identifier ... (allow identifier-like keywords
    // GET/SET)
    if (stream.peek().getType() == tokens::TokenType::FUNCTION) {
      auto method = buildFunction(stream, memberFuncMod);
      if (method) {
        method->access = access;
        method->methodAttributes = std::move(collectedMethodAttrs);
        classDecl->methods.push_back(method);
        continue;
      } else {
        // error recovery
        stream.advance();
        continue;
      }
    }

    // Constructor: 'constructor' '(' ... ')' Block
    if (stream.peek().getType() == tokens::TokenType::CONSTRUCTOR) {
      tokens::Token ctorTok = stream.peek();
      auto ctorLoc = ctorTok.getLocation();
      stream.advance();  // consume 'constructor'
      if (stream.peek().getType() != tokens::TokenType::LEFT_PAREN) {
        // malformed, recover
        stream.advance();
        continue;
      }
      stream.advance();  // consume '('
      auto ctor = std::make_shared<ast::ConstructorDecl>();
      ctor->keyword = ctorTok;
      ctor->location = ctorLoc;
      ctor->access = access;
      // Parameters
      while (stream.peek().getType() != tokens::TokenType::RIGHT_PAREN &&
             !stream.isAtEnd()) {
        if (stream.peek().getType() == tokens::TokenType::COMMA) {
          stream.advance();
          continue;
        }
        auto param = std::make_shared<ast::Parameter>();
        if (stream.peek().getType() == tokens::TokenType::IDENTIFIER) {
          tokens::Token paramName = stream.peek();
          auto paramLoc = stream.peek().getLocation();
          stream.advance();  // consume identifier
          if (stream.peek().getType() == tokens::TokenType::COLON) {
            stream.advance();  // consume ':'
            auto paramType = TypeBuilder::build(stream);
            if (!paramType) {
              stream.advance();
            } else {
              param->name = paramName;
              param->type = paramType;
              param->location = paramLoc;
              ctor->params.push_back(param);
            }
          }
        } else {
          // skip unexpected token
          stream.advance();
        }
      }
      if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN)
        stream.advance();
      // Body
      if (stream.peek().getType() == tokens::TokenType::LEFT_BRACE) {
        auto body = StatementBuilder::buildBlock(stream);
        ctor->body = body;
      } else {
        // recover
        stream.advance();
      }
      classDecl->constructor = ctor;
      continue;
    }

    // Fields: let/const identifier [: Type] [= expr] ;
    if (stream.peek().getType() == tokens::TokenType::LET ||
        stream.peek().getType() == tokens::TokenType::CONST) {
      auto fieldStmt = buildVariable(stream, memberStorage);
      if (fieldStmt) {
        if (auto var = std::dynamic_pointer_cast<ast::VarDecl>(fieldStmt)) {
          var->access = access;
          var->fieldModifiers = std::move(collectedFieldMods);
          classDecl->fields.push_back(var);
        }
        classDecl->body->statements.push_back(fieldStmt);
        continue;
      } else {
        stream.advance();
        continue;
      }
    }

    // Bare field syntax: Identifier ':' Type [ '=' initializer ] ';'
    if (stream.peek().getType() == tokens::TokenType::IDENTIFIER &&
        stream.peekNext().getType() == tokens::TokenType::COLON) {
      tokens::Token fieldName = stream.peek();
      auto fieldLoc = fieldName.getLocation();
      stream.advance();  // consume identifier
      stream.advance();  // consume ':'

      Shared(ast::TypeNode) fieldType = TypeBuilder::build(stream);
      if (!fieldType) {
        // malformed type, try to recover
        stream.advance();
        continue;
      }

      Shared(ast::Expr) initializer = nullptr;
      if (stream.peek().getType() == tokens::TokenType::EQUALS) {
        stream.advance();  // consume '='
        initializer = ExpressionBuilder::build(stream);
      }
      if (stream.peek().getType() == tokens::TokenType::SEMICOLON) {
        stream.advance();  // consume ';'
      }

      auto varDecl = std::make_shared<ast::VarDecl>();
      varDecl->name = fieldName;
      varDecl->type = fieldType;
      varDecl->initializer = initializer;
      varDecl->qualifier = ast::StorageQualifier::None;
      varDecl->isConst = false;
      varDecl->access = access;
      varDecl->fieldModifiers = std::move(collectedFieldMods);
      varDecl->location = fieldLoc;

      classDecl->fields.push_back(varDecl);
      classDecl->body->statements.push_back(varDecl);
      continue;
    }

    // Fallback: parse any statement for resilience
    auto stmt = StatementBuilder::build(stream);
    if (stmt) {
      classDecl->body->statements.push_back(stmt);
    } else {
      // error recovery to avoid infinite loop
      stream.advance();
    }
  }
  if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
    stream.advance();  // consume '}'
  } else {
    std::cerr << "Error: Class body is not properly closed at "
              << classLoc.toString() << std::endl;
  }
  return classDecl;
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

  // Optional generic params after interface name: <...> (skip)
  if (stream.peek().getType() == tokens::TokenType::LESS) {
    int depth = 0;
    do {
      if (stream.peek().getType() == tokens::TokenType::LESS) depth++;
      if (stream.peek().getType() == tokens::TokenType::GREATER) depth--;
      stream.advance();
    } while (!stream.isAtEnd() && depth > 0);
  }

  // Optional: extends Interface { , Interface }
  if (stream.peek().getType() == tokens::TokenType::EXTENDS) {
    stream.advance();
    while (!stream.isAtEnd()) {
      if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) break;
      interfaceDecl->bases.push_back(stream.peek());
      stream.advance();
      // Skip optional type arguments <...>
      if (stream.peek().getType() == tokens::TokenType::LESS) {
        int d = 0;
        do {
          if (stream.peek().getType() == tokens::TokenType::LESS) d++;
          if (stream.peek().getType() == tokens::TokenType::GREATER) d--;
          stream.advance();
        } while (!stream.isAtEnd() && d > 0);
      }
      if (stream.peek().getType() == tokens::TokenType::COMMA) {
        stream.advance();
        continue;
      }
      break;
    }
  }

  // Body: '{' (propertySig | methodSig | ';')* '}'
  if (stream.peek().getType() != tokens::TokenType::LEFT_BRACE) {
    std::cerr << "Error: Interface body is missing at "
              << interfaceLoc.toString() << std::endl;
    return nullptr;
  }
  stream.advance();  // consume '{'
  while (!stream.isAtEnd() &&
         stream.peek().getType() != tokens::TokenType::RIGHT_BRACE) {
    if (stream.peek().getType() == tokens::TokenType::SEMICOLON) {
      stream.advance();
      continue;
    }

    // Method signature: 'function' Identifier '(' [params] ')' [':' Type] ';'
    if (stream.peek().getType() == tokens::TokenType::FUNCTION) {
      auto funcDecl = std::make_shared<ast::FunctionDecl>();
      funcDecl->modifier = ast::FunctionModifier::None;
      stream.advance();  // consume 'function'
      auto nameTokType = stream.peek().getType();
      if (!(nameTokType == tokens::TokenType::IDENTIFIER ||
            nameTokType == tokens::TokenType::GET ||
            nameTokType == tokens::TokenType::SET)) {
        // recovery
        stream.advance();
        continue;
      }
      funcDecl->name = stream.peek();
      funcDecl->location = stream.peek().getLocation();
      stream.advance();  // consume name
      if (stream.peek().getType() != tokens::TokenType::LEFT_PAREN) {
        // recovery
        stream.advance();
        continue;
      }
      stream.advance();  // consume '('
      // Parse parameter list: Identifier ':' Type { ',' Identifier ':' Type }
      while (!stream.isAtEnd() &&
             stream.peek().getType() != tokens::TokenType::RIGHT_PAREN) {
        if (stream.peek().getType() == tokens::TokenType::COMMA) {
          stream.advance();
          continue;
        }
        if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
          // skip unexpected token
          stream.advance();
          continue;
        }
        tokens::Token paramName = stream.peek();
        auto paramLoc = paramName.getLocation();
        stream.advance();  // consume name
        if (stream.peek().getType() != tokens::TokenType::COLON) {
          // missing type annotation; recover by skipping to next comma/paren
          while (!stream.isAtEnd() &&
                 stream.peek().getType() != tokens::TokenType::COMMA &&
                 stream.peek().getType() != tokens::TokenType::RIGHT_PAREN) {
            stream.advance();
          }
          continue;
        }
        stream.advance();  // consume ':'
        auto paramType = TypeBuilder::build(stream);
        if (!paramType) {
          // recover similarly
          while (!stream.isAtEnd() &&
                 stream.peek().getType() != tokens::TokenType::COMMA &&
                 stream.peek().getType() != tokens::TokenType::RIGHT_PAREN) {
            stream.advance();
          }
          continue;
        }
        auto param = std::make_shared<ast::Parameter>();
        param->name = paramName;
        param->type = paramType;
        param->location = paramLoc;
        funcDecl->params.push_back(param);
      }
      if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN) {
        stream.advance();  // consume ')'
      }
      if (stream.peek().getType() == tokens::TokenType::COLON) {
        stream.advance();  // consume ':'
        funcDecl->returnType = TypeBuilder::build(stream);
      }
      // Expect optional ';'
      if (stream.peek().getType() == tokens::TokenType::SEMICOLON) {
        stream.advance();
      }
      funcDecl->body = nullptr;  // interface method has no body
      interfaceDecl->methods.push_back(funcDecl);
      continue;
    }

    // Property signature: Identifier ':' Type ';'
    if (stream.peek().getType() == tokens::TokenType::IDENTIFIER &&
        stream.peekNext().getType() == tokens::TokenType::COLON) {
      tokens::Token propName = stream.peek();
      auto propLoc = propName.getLocation();
      stream.advance();  // name
      stream.advance();  // ':'
      auto typeNode = TypeBuilder::build(stream);
      if (!typeNode) {
        // malformed type; recover forward
        stream.advance();
        continue;
      }
      if (stream.peek().getType() == tokens::TokenType::SEMICOLON) {
        stream.advance();
      }
      auto var = std::make_shared<ast::VarDecl>();
      var->name = propName;
      var->type = typeNode;
      var->qualifier = ast::StorageQualifier::None;
      var->isConst = false;
      var->location = propLoc;
      interfaceDecl->properties.push_back(var);
      continue;
    }

    // Unknown member: try to recover
    stream.advance();
  }
  if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
    stream.advance();
  } else {
    std::cerr << "Error: Interface body is not properly closed at "
              << interfaceLoc.toString() << std::endl;
  }
  return interfaceDecl;
}

}  // namespace parser
