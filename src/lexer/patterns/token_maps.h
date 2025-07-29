#pragma once
// Token type and pattern mapping tables

#include <string>
#include <unordered_map>

#include "../../tokens/TokenType.h"

namespace lexer {
namespace patterns {
// Example operator to token type map
static const std::unordered_map<std::string, tokens::TokenType> operator_map = {
    {"+", tokens::TokenType::PLUS},   {"-", tokens::TokenType::MINUS},
    {"*", tokens::TokenType::STAR},   {"/", tokens::TokenType::SLASH},
    {"=", tokens::TokenType::EQUALS},
    // ... add more as needed
};
}  // namespace patterns
}  // namespace lexer
