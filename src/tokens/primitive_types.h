#pragma once
/**
 * The primitive type names of tspp, in one table.
 *
 * Read by the lexer (which names are type keywords), the semantic analyzer
 * (canonical type names) and codegen (getLLVMType and "is this a primitive"
 * checks). Add or rename a primitive type here, nowhere else.
 *
 * `boolean` (TypeScript) and `bool` are both accepted and mean the same type,
 * LLVM i1; its canonical name, used in codegen's type strings, is "bool".
 */
#include <string>
#include <string_view>

#include "token_type.h"

namespace tokens {

enum class PrimitiveKind { Int, Float, Double, Bool, String, Void };

struct PrimitiveType {
  std::string_view name;  // spelling in source
  PrimitiveKind kind;
  // Token the lexer produces for this word; IDENTIFIER if it is not a
  // keyword (it is then still recognized as a type name).
  TokenType keyword;
};

inline constexpr PrimitiveType kPrimitiveTypes[] = {
    {"int", PrimitiveKind::Int, TokenType::INT},
    {"float", PrimitiveKind::Float, TokenType::FLOAT},
    {"double", PrimitiveKind::Double, TokenType::IDENTIFIER},
    {"boolean", PrimitiveKind::Bool, TokenType::BOOLEAN},
    {"bool", PrimitiveKind::Bool, TokenType::BOOLEAN},
    {"string", PrimitiveKind::String, TokenType::STRING},
    {"void", PrimitiveKind::Void, TokenType::VOID},
};

// The primitive type spelled `name`, or nullptr.
inline const PrimitiveType* findPrimitiveType(std::string_view name) {
  for (const auto& p : kPrimitiveTypes)
    if (p.name == name) return &p;
  return nullptr;
}

inline bool isPrimitiveTypeName(std::string_view name) {
  return findPrimitiveType(name) != nullptr;
}

// The name codegen uses for a primitive kind ("bool" for boolean/bool).
inline const char* canonicalName(PrimitiveKind kind) {
  switch (kind) {
    case PrimitiveKind::Int:
      return "int";
    case PrimitiveKind::Float:
      return "float";
    case PrimitiveKind::Double:
      return "double";
    case PrimitiveKind::Bool:
      return "bool";
    case PrimitiveKind::String:
      return "string";
    case PrimitiveKind::Void:
      return "void";
  }
  return "";
}

// `name` with a primitive spelling replaced by its canonical name
// ("boolean" -> "bool"); any other name is returned unchanged.
inline std::string canonicalTypeName(const std::string& name) {
  const PrimitiveType* p = findPrimitiveType(name);
  return p ? canonicalName(p->kind) : name;
}

}  // namespace tokens
