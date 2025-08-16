#include "TypeBuilder.h"

#include <iostream>
#include <memory>

#include "core/common/macros.h"

namespace parser {

Shared(ast::TypeNode) TypeBuilder::build(tokens::TokenStream& stream) {
  if (stream.isAtEnd()) return nullptr;
  auto type = stream.peek().getType();

  // Handle object type literals: '{' Identifier ':' Type (';' Identifier ':'
  // Type)* '}'
  if (type == tokens::TokenType::LEFT_BRACE) {
    auto startTok = stream.peek();
    stream.advance();  // consume '{'
    auto objTy = std::make_shared<ast::ObjectTypeNode>();
    objTy->location = startTok.getLocation();
    // Empty object type
    if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
      stream.advance();
      // Allow union/intersection continuations after object type: {..} ('|' T |
      // '&' T)*
      Shared(ast::TypeNode) resultType = objTy;
      std::vector<std::pair<char, Shared(ast::TypeNode)>> seq;
      while (true) {
        if (stream.peek().getType() == tokens::TokenType::PIPE ||
            stream.peek().getType() == tokens::TokenType::AMPERSAND) {
          char op =
              (stream.peek().getType() == tokens::TokenType::PIPE) ? '|' : '&';
          stream.advance();
          auto nextType = build(stream);
          if (!nextType) break;
          seq.emplace_back(op, nextType);
        } else {
          break;
        }
      }
      if (!seq.empty()) {
        std::vector<Shared(ast::TypeNode)> unionParts;
        std::vector<Shared(ast::TypeNode)> interParts{resultType};
        auto flushIntersection = [&]() {
          if (interParts.size() == 1) {
            unionParts.push_back(interParts[0]);
          } else {
            auto interNode = std::make_shared<ast::IntersectionTypeNode>();
            interNode->types = interParts;
            interNode->location = interParts[0]->location;
            unionParts.push_back(interNode);
          }
          interParts.clear();
        };
        for (auto& [op, ty] : seq) {
          if (op == '&')
            interParts.push_back(ty);
          else {
            flushIntersection();
            interParts.push_back(ty);
          }
        }
        flushIntersection();
        if (unionParts.size() == 1)
          resultType = unionParts[0];
        else {
          auto unionNode = std::make_shared<ast::UnionTypeNode>();
          unionNode->types = unionParts;
          unionNode->location = unionParts[0]->location;
          resultType = unionNode;
        }
      }
      return resultType;
    }
    while (!stream.isAtEnd() &&
           stream.peek().getType() != tokens::TokenType::RIGHT_BRACE) {
      if (stream.peek().getType() != tokens::TokenType::IDENTIFIER) {
        // Recovery: skip until '}'
        while (!stream.isAtEnd() &&
               stream.peek().getType() != tokens::TokenType::RIGHT_BRACE) {
          stream.advance();
        }
        if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE)
          stream.advance();
        return objTy;
      }
      auto keyTok = stream.peek();
      stream.advance();  // consume key
      if (stream.peek().getType() == tokens::TokenType::COLON) stream.advance();
      auto fieldTy = build(stream);
      if (!fieldTy) {
        // recovery: break
        break;
      }
      objTy->fields.push_back(ast::ObjectTypeNode::Field{keyTok, fieldTy});
      // Optional ';' or ',' between fields
      if (stream.peek().getType() == tokens::TokenType::SEMICOLON ||
          stream.peek().getType() == tokens::TokenType::COMMA) {
        stream.advance();
      }
    }
    if (stream.peek().getType() == tokens::TokenType::RIGHT_BRACE) {
      stream.advance();
    }
    // Allow union/intersection continuations after non-empty object type:
    // {..} ('|' T | '&' T)*
    Shared(ast::TypeNode) resultType = objTy;
    std::vector<std::pair<char, Shared(ast::TypeNode)>> seq;
    while (true) {
      if (stream.peek().getType() == tokens::TokenType::PIPE ||
          stream.peek().getType() == tokens::TokenType::AMPERSAND) {
        char op =
            (stream.peek().getType() == tokens::TokenType::PIPE) ? '|' : '&';
        stream.advance();
        auto nextType = build(stream);
        if (!nextType) break;
        seq.emplace_back(op, nextType);
      } else {
        break;
      }
    }
    if (!seq.empty()) {
      std::vector<Shared(ast::TypeNode)> unionParts;
      std::vector<Shared(ast::TypeNode)> interParts{resultType};
      auto flushIntersection = [&]() {
        if (interParts.size() == 1) {
          unionParts.push_back(interParts[0]);
        } else {
          auto interNode = std::make_shared<ast::IntersectionTypeNode>();
          interNode->types = interParts;
          interNode->location = interParts[0]->location;
          unionParts.push_back(interNode);
        }
        interParts.clear();
      };
      for (auto& [op, ty] : seq) {
        if (op == '&')
          interParts.push_back(ty);
        else {
          flushIntersection();
          interParts.push_back(ty);
        }
      }
      flushIntersection();
      if (unionParts.size() == 1) {
        resultType = unionParts[0];
      } else {
        auto unionNode = std::make_shared<ast::UnionTypeNode>();
        unionNode->types = unionParts;
        unionNode->location = unionParts[0]->location;
        resultType = unionNode;
      }
    }
    return resultType;
  }

  // Handle grouped types: '(' Type ')'
  if (type == tokens::TokenType::LEFT_PAREN) {
    auto lp = stream.peek();
    stream.advance();  // '('
    auto inner = build(stream);
    if (stream.peek().getType() == tokens::TokenType::RIGHT_PAREN) {
      stream.advance();  // ')'
    }
    if (!inner) return nullptr;
    // After a grouped type, continue to process union/intersection chains etc.
    Shared(ast::TypeNode) resultType = inner;
    // Pointer suffixes right after group: (T) '*'+
    while (stream.peek().getType() == tokens::TokenType::STAR) {
      stream.advance();
      auto ptrNode = std::make_shared<ast::PointerTypeNode>();
      ptrNode->baseType = resultType;
      ptrNode->location = resultType->location;
      resultType = ptrNode;
    }
    // Union/Intersection continuation
    std::vector<std::pair<char, Shared(ast::TypeNode)>> seq;
    while (true) {
      if (stream.peek().getType() == tokens::TokenType::PIPE ||
          stream.peek().getType() == tokens::TokenType::AMPERSAND) {
        char op =
            (stream.peek().getType() == tokens::TokenType::PIPE) ? '|' : '&';
        stream.advance();
        auto nextType = build(stream);
        if (!nextType) break;
        seq.emplace_back(op, nextType);
      } else {
        break;
      }
    }
    if (!seq.empty()) {
      std::vector<Shared(ast::TypeNode)> unionParts;
      std::vector<Shared(ast::TypeNode)> interParts{resultType};
      auto flushIntersection = [&]() {
        if (interParts.size() == 1) {
          unionParts.push_back(interParts[0]);
        } else {
          auto interNode = std::make_shared<ast::IntersectionTypeNode>();
          interNode->types = interParts;
          interNode->location = interParts[0]->location;
          unionParts.push_back(interNode);
        }
        interParts.clear();
      };
      for (auto& [op, ty] : seq) {
        if (op == '&')
          interParts.push_back(ty);
        else {
          flushIntersection();
          interParts.push_back(ty);
        }
      }
      flushIntersection();
      if (unionParts.size() == 1)
        resultType = unionParts[0];
      else {
        auto unionNode = std::make_shared<ast::UnionTypeNode>();
        unionNode->types = unionParts;
        unionNode->location = unionParts[0]->location;
        resultType = unionNode;
      }
    }
    return resultType;
  }

  // Handle pointer types: [ '@' ('unsafe' | 'aligned') ] Type '*'
  if (type == tokens::TokenType::AT) {
    stream.advance();  // consume '@'
    auto modType = stream.peek().getType();
    tokens::Token modifier;
    if (modType == tokens::TokenType::UNSAFE ||
        modType == tokens::TokenType::ALIGNED) {
      modifier = stream.peek();
      stream.advance();
    }
    auto baseType = buildBasic(stream);
    if (!baseType) {
      // base type build failed
    }
    if (stream.peek().getType() == tokens::TokenType::STAR) {
      stream.advance();  // consume '*'
      auto ptrNode = std::make_shared<ast::PointerTypeNode>();
      ptrNode->qualifier = modifier;
      ptrNode->baseType = baseType;
      ptrNode->location =
          baseType ? baseType->location : tokens::Token().getLocation();
      return ptrNode;
    }
    // expected '*'
    return nullptr;
  }

  // Handle smart pointer types: '#' ('shared' | 'unique' | 'weak') '<' Type '>'
  if (type == tokens::TokenType::ATTRIBUTE) {
    stream.advance();  // consume '#'
    auto smartType = stream.peek().getType();
    if (smartType == tokens::TokenType::SHARED ||
        smartType == tokens::TokenType::UNIQUE ||
        smartType == tokens::TokenType::WEAK) {
      auto smartToken = stream.peek();
      stream.advance();
      if (stream.peek().getType() == tokens::TokenType::LESS) {
        stream.advance();  // consume '<'
        auto innerType = build(stream);
        if (stream.peek().getType() == tokens::TokenType::GREATER) {
          stream.advance();  // consume '>'
          auto smartPtrNode = std::make_shared<ast::SmartPointerTypeNode>();
          smartPtrNode->kind = smartToken;
          smartPtrNode->target = innerType;
          smartPtrNode->location = smartToken.getLocation();
          return smartPtrNode;
        }
      }
    }
    // Error: not a valid smart pointer type
    return nullptr;
  }

  // Handle basic types and identifiers
  if (isBasicType(type) || type == tokens::TokenType::IDENTIFIER) {
    // Peek identifier or primitive
    auto startTok = stream.peek();
    auto baseBasic = buildBasic(stream);
    if (!baseBasic) return nullptr;

    // Convert to TypeNode for further processing
    Shared(ast::TypeNode) resultType =
        std::static_pointer_cast<ast::TypeNode>(baseBasic);

    // Handle generic type application: Identifier '<' Type {',' Type} '>'
    if (startTok.getType() == tokens::TokenType::IDENTIFIER &&
        stream.peek().getType() == tokens::TokenType::LESS) {
      stream.advance();  // consume '<'
      auto genNode = std::make_shared<ast::GenericTypeNode>();
      genNode->name = startTok;
      genNode->location = startTok.getLocation();
      // Parse one or more type arguments until '>'
      while (!stream.isAtEnd() &&
             stream.peek().getType() != tokens::TokenType::GREATER) {
        auto argTy = build(stream);
        if (argTy) genNode->typeArgs.push_back(argTy);
        if (stream.peek().getType() == tokens::TokenType::COMMA) {
          stream.advance();  // consume ',' and continue
        } else {
          break;
        }
      }
      if (stream.peek().getType() == tokens::TokenType::GREATER)
        stream.advance();  // consume '>' if present
      resultType = genNode;
    }

    // Check for pointer suffix chain: Type '*'* (without @ prefix)
    while (stream.peek().getType() == tokens::TokenType::STAR) {
      stream.advance();  // consume '*'
      auto ptrNode = std::make_shared<ast::PointerTypeNode>();
      ptrNode->qualifier = tokens::Token();  // no qualifier for suffix syntax
      ptrNode->baseType = resultType;
      ptrNode->location = resultType->location;
      resultType = ptrNode;
    }

    // Check for union/intersection chains: Type ('|' Type | '&' Type)*
    // First gather a list with operator separators
    std::vector<std::pair<char, Shared(ast::TypeNode)>> seq;
    // Seed with the first type (as if after a dummy '+')
    while (true) {
      if (stream.peek().getType() == tokens::TokenType::PIPE ||
          stream.peek().getType() == tokens::TokenType::AMPERSAND) {
        char op =
            (stream.peek().getType() == tokens::TokenType::PIPE) ? '|' : '&';
        stream.advance();
        auto nextType = build(stream);
        if (!nextType) break;
        seq.emplace_back(op, nextType);
      } else {
        break;
      }
    }
    if (!seq.empty()) {
      // Group by operator precedence: '&' binds tighter than '|'
      // First, build segments of intersections
      std::vector<Shared(ast::TypeNode)> unionParts;
      Shared(ast::TypeNode) current = resultType;
      std::vector<Shared(ast::TypeNode)> interParts;
      interParts.push_back(current);
      auto flushIntersection = [&]() {
        if (interParts.size() == 1) {
          unionParts.push_back(interParts[0]);
        } else {
          auto interNode = std::make_shared<ast::IntersectionTypeNode>();
          interNode->types = interParts;
          interNode->location = interParts[0]->location;
          unionParts.push_back(interNode);
        }
        interParts.clear();
      };
      for (auto& [op, ty] : seq) {
        if (op == '&') {
          interParts.push_back(ty);
        } else {  // '|'
          flushIntersection();
          interParts.push_back(ty);
        }
      }
      flushIntersection();
      if (unionParts.size() == 1) {
        resultType = unionParts[0];
      } else {
        auto unionNode = std::make_shared<ast::UnionTypeNode>();
        unionNode->types = unionParts;
        unionNode->location = unionParts[0]->location;
        resultType = unionNode;
      }
    }

    // Check for type constraints: Type [ 'extends' Type ]
    if (stream.peek().getType() == tokens::TokenType::EXTENDS) {
      stream.advance();  // consume 'extends'
      auto constraintType = build(stream);
      if (constraintType) {
        auto constraintNode = std::make_shared<ast::TypeConstraintNode>();
        constraintNode->base = resultType;
        constraintNode->constraint = constraintType;
        constraintNode->location = resultType->location;
        return constraintNode;
      }
    }

    return resultType;
  }

  // Unrecognized type
  stream.advance();
  return nullptr;
}

// Parse a basic type (int, float, boolean, string, or identifier)
Shared(ast::BasicTypeNode)
    TypeBuilder::buildBasic(tokens::TokenStream& stream) {
  if (stream.isAtEnd()) return nullptr;
  auto type = stream.peek().getType();
  if (isBasicType(type) || type == tokens::TokenType::IDENTIFIER) {
    auto basicTypeNode = std::make_shared<ast::BasicTypeNode>();
    basicTypeNode->name = stream.peek();
    basicTypeNode->location = stream.peek().getLocation();
    stream.advance();
    return basicTypeNode;
  }
  // Unrecognized basic type
  stream.advance();
  return nullptr;
}

// Parse a pointer type (already handled in build)
Shared(ast::PointerTypeNode) TypeBuilder::buildPointer(
    tokens::TokenStream& stream, Shared(ast::TypeNode) baseType) {
  // Not used directly; handled in build()
  return nullptr;
}

// Parse a smart pointer type (already handled in build)
Shared(ast::SmartPointerTypeNode)
    TypeBuilder::buildSmartPointer(tokens::TokenStream& stream) {
  // Not used directly; handled in build()
  return nullptr;
}

// Utility: is this a basic type?
bool TypeBuilder::isBasicType(tokens::TokenType type) {
  switch (type) {
    case tokens::TokenType::INT:
    case tokens::TokenType::FLOAT:
    case tokens::TokenType::BOOLEAN:
    case tokens::TokenType::STRING:
    case tokens::TokenType::VOID:
      return true;
    default:
      return false;
  }
}

}  // namespace parser
