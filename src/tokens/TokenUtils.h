#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "token_type.h"
/**
 * TokenUtils - Helper utilities for token classification and lookup.
 */
namespace tokens {
namespace TokenUtils {
inline bool isKeyword(const std::string &word) {
  static const std::unordered_set<std::string> keywords = {
      // Declarations & Types
      "let", "const", "function", "class", "interface", "enum", "constructor",
      "typedef", "namespace", "template", "new", "get", "set", "cast", "where",
      "throws", "public", "private", "protected", "if", "else", "switch",
      "case", "default", "while", "do", "for", "of", "break", "continue",
      "return", "try", "catch", "finally", "throw", "void", "int", "float",
      "boolean", "string",
      // Storage Qualifiers
      "stack", "heap", "static",
      // Smart Pointer Modifiers
      "shared", "unique", "weak",
      // Function Modifiers
      "const", "constexpr", "zerocast", "simd", "prefetch", "atomic", "pinned",
      // Pointer Type Modifiers
      "unsafe", "aligned",
      // Class Modifiers
      "packed", "abstract",
      // Other
      "attribute", "extends", "implements", "sizeof", "alignof", "typeof",
      "asm", "true", "false", "null_value", "undefined", "this", "yield",
      "async", "await"};
  return keywords.count(word) > 0;
}
inline TokenType keywordToToken(const std::string &word) {
  static const std::unordered_map<std::string, TokenType> keywordMap = {
      // Declarations & Types
      {"let", TokenType::LET},
      {"const", TokenType::CONST},
      {"function", TokenType::FUNCTION},
      {"class", TokenType::CLASS},
      {"interface", TokenType::INTERFACE},
      {"enum", TokenType::ENUM},
      {"constructor", TokenType::CONSTRUCTOR},
      {"typedef", TokenType::TYPEDEF},
      {"namespace", TokenType::NAMESPACE},
      {"template", TokenType::TEMPLATE},
      {"new", TokenType::NEW},
      {"get", TokenType::GET},
      {"set", TokenType::SET},
      {"cast", TokenType::CAST},
      {"where", TokenType::WHERE},
      {"throws", TokenType::THROWS},
      {"public", TokenType::PUBLIC},
      {"private", TokenType::PRIVATE},
      {"protected", TokenType::PROTECTED},
      {"if", TokenType::IF},
      {"else", TokenType::ELSE},
      {"switch", TokenType::SWITCH},
      {"case", TokenType::CASE},
      {"default", TokenType::DEFAULT},
      {"while", TokenType::WHILE},
      {"do", TokenType::DO},
      {"for", TokenType::FOR},
      {"of", TokenType::OF},
      {"break", TokenType::BREAK},
      {"continue", TokenType::CONTINUE},
      {"return", TokenType::RETURN},
      {"try", TokenType::TRY},
      {"catch", TokenType::CATCH},
      {"finally", TokenType::FINALLY},
      {"throw", TokenType::THROW},
      {"void", TokenType::VOID},
      {"int", TokenType::INT},
      {"float", TokenType::FLOAT},
      {"boolean", TokenType::BOOLEAN},
      {"string", TokenType::STRING},
      // Storage Qualifiers
      {"stack", TokenType::STACK},
      {"heap", TokenType::HEAP},
      {"static", TokenType::STATIC},
      // Smart Pointer Modifiers
      {"shared", TokenType::SHARED},
      {"unique", TokenType::UNIQUE},
      {"weak", TokenType::WEAK},
      // Function Modifiers
      {"const", TokenType::CONST_FUNCTION},
      {"constexpr", TokenType::CONSTEXPR},
      {"zerocast", TokenType::ZEROCAST},
      {"simd", TokenType::SIMD},
      {"prefetch", TokenType::PREFETCH},
      {"atomic", TokenType::ATOMIC},
      {"pinned", TokenType::PINNED},
      // Pointer Type Modifiers
      {"unsafe", TokenType::UNSAFE},
      {"aligned", TokenType::ALIGNED},
      // Class Modifiers
      {"packed", TokenType::PACKED},
      {"abstract", TokenType::ABSTRACT},
      // Other
      {"attribute", TokenType::ATTRIBUTE},
      {"extends", TokenType::EXTENDS},
      {"implements", TokenType::IMPLEMENTS},
      {"sizeof", TokenType::SIZEOF},
      {"alignof", TokenType::ALIGNOF},
      {"typeof", TokenType::TYPEOF},
      {"asm", TokenType::ASM},
      {"true", TokenType::TRUE},
      {"false", TokenType::FALSE},
      {"null_value", TokenType::NULL_VALUE},
      {"undefined", TokenType::UNDEFINED},
      {"this", TokenType::THIS},
      {"yield", TokenType::YIELD},
      {"async", TokenType::ASYNC},
      {"await", TokenType::AWAIT}};
  auto it = keywordMap.find(word);
  return it != keywordMap.end() ? it->second : TokenType::IDENTIFIER;
}
inline std::string tokenTypeToString(TokenType type) {
#define CASE(x)      \
  case TokenType::x: \
    return #x;
  switch (type) {
    CASE(LET)
    CASE(CONST)
    CASE(FUNCTION)
    CASE(CLASS)
    CASE(INTERFACE)
    CASE(ENUM)
    CASE(CONSTRUCTOR)
    CASE(TYPEDEF)
    CASE(NAMESPACE)
    CASE(TEMPLATE)
    CASE(NEW)
    CASE(GET)
    CASE(SET)
    CASE(CAST)
    CASE(PRIVATE)
    CASE(PROTECTED)
    CASE(IF)
    CASE(ELSE)
    CASE(SWITCH)
    CASE(CASE)
    CASE(DEFAULT)
    CASE(WHILE)
    CASE(DO)
    CASE(FOR)
    CASE(OF)
    CASE(BREAK)
    CASE(CONTINUE)
    CASE(RETURN)
    CASE(TRY)
    CASE(CATCH)
    CASE(FINALLY)
    CASE(THROW)
    CASE(VOID)
    CASE(INT)
    CASE(FLOAT)
    CASE(BOOLEAN)
    CASE(STRING)
    CASE(STACK)
    CASE(HEAP)
    CASE(STATIC)
    CASE(SHARED)
    CASE(UNIQUE)
    CASE(WEAK)
    CASE(ATTRIBUTE)
    CASE(CONST_FUNCTION)
    CASE(CONSTEXPR)
    CASE(ZEROCAST)
    CASE(SIMD)
    CASE(PREFETCH)
    CASE(ATOMIC)
    CASE(PINNED)
    CASE(UNSAFE)
    CASE(ALIGNED)
    CASE(PACKED)
    CASE(ABSTRACT)
    CASE(EXTENDS)
    CASE(IMPLEMENTS)
    CASE(SIZEOF)
    CASE(ALIGNOF)
    CASE(TYPEOF)
    CASE(ASM)
    CASE(TRUE)
    CASE(FALSE)
    CASE(NULL_VALUE)
    CASE(UNDEFINED)
    CASE(THIS)
    CASE(YIELD)
    CASE(ASYNC)
    CASE(AWAIT)
    default:
      return "UNKNOWN";
  }
#undef CASE
}
inline bool isOperator(TokenType type) {
  // Example: treat some types as operators
  return type == TokenType::PLUS || type == TokenType::MINUS ||
         type == TokenType::STAR || type == TokenType::SLASH;
}
inline bool isLiteral(TokenType type) {
  return type == TokenType::INT || type == TokenType::FLOAT ||
         type == TokenType::STRING || type == TokenType::BOOLEAN;
}
inline bool isValidIdentifier(const std::string &s) {
  if (s.empty() || std::isdigit(s[0])) return false;
  for (char c : s)
    if (!std::isalnum(c) && c != '_') return false;
  return true;
}
inline bool isTypeKeyword(const std::string &word) {
  return word == "int" || word == "float" || word == "string" ||
         word == "boolean";
}
inline bool isAmbiguousToken(TokenType type) {
  // Example: treat ERROR_TOKEN as ambiguous
  return type == TokenType::ERROR_TOKEN;
}
}  // namespace TokenUtils
}  // namespace tokens
