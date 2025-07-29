#pragma once
// Regex and pattern definitions for lexer

#include <regex>
#include <string>

namespace lexer {
namespace patterns {
// Example patterns for identifiers, numbers, strings, operators
static const std::regex identifier_pattern(R"([a-zA-Z_][a-zA-Z0-9_]*)");
static const std::regex number_pattern(R"(\d+(\.\d+)?)");
static const std::regex string_pattern(R"(".*?"|'.*?')");
static const std::regex operator_pattern(R"([+\-*/=<>!&|^%]+)");
}  // namespace patterns
}  // namespace lexer
