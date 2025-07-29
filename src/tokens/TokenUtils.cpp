#include "TokenUtils.h"

#include <cctype>
#include <unordered_map>
#include <unordered_set>

#include "TokenType.h"

namespace tokens {
namespace TokenUtils {
static const std::unordered_set<std::string> keywords = {
    "let",      "const",       "function",  "class",      "interface",
    "enum",     "constructor", "typedef",   "namespace",  "template",
    "new",      "get",         "set",       "cast",       "where",
    "throws",   "public",      "private",   "protected",  "if",
    "else",     "switch",      "case",      "default",    "while",
    "do",       "for",         "of",        "break",      "continue",
    "return",   "try",         "catch",     "finally",    "throw",
    "void",     "int",         "float",     "boolean",    "string",
    "stack",    "heap",        "static",    "shared",     "unique",
    "weak",     "attribute",   "inline",    "virtual",    "unsafe",
    "simd",     "target",      "ref",       "aligned",    "packed",
    "abstract", "zerocast",    "extends",   "implements", "const_expr",
    "sizeof",   "alignof",     "typeof",    "asm",        "true",
    "false",    "null_value",  "undefined", "this",       "yield",
    "async",    "await"};

static const std::unordered_map<std::string, TokenType> keywordMap = {
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
    {"stack", TokenType::STACK},
    {"heap", TokenType::HEAP},
    {"static", TokenType::STATIC},
    {"shared", TokenType::SHARED},
    {"unique", TokenType::UNIQUE},
    {"weak", TokenType::WEAK},
    {"attribute", TokenType::ATTRIBUTE},
    {"inline", TokenType::INLINE},
    {"virtual", TokenType::VIRTUAL},
    {"unsafe", TokenType::UNSAFE},
    {"simd", TokenType::SIMD},
    {"target", TokenType::TARGET},
    {"ref", TokenType::REF},
    {"aligned", TokenType::ALIGNED},
    {"packed", TokenType::PACKED},
    {"abstract", TokenType::ABSTRACT},
    {"zerocast", TokenType::ZEROCAST},
    {"extends", TokenType::EXTENDS},
    {"implements", TokenType::IMPLEMENTS},
    {"const_expr", TokenType::CONST_EXPR},
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

bool isKeyword(const std::string &word) {
  return keywords.count(word) > 0;
}
TokenType keywordToToken(const std::string &word) {
  auto it = keywordMap.find(word);
  return it != keywordMap.end() ? it->second : TokenType::IDENTIFIER;
}
std::string tokenTypeToString(TokenType type) {
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
    // CASE(WHERE) // Duplicate, removed
    // CASE(THROWS) // Duplicate, removed
    // CASE(PUBLIC) // Duplicate, removed
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
    CASE(INLINE)
    CASE(VIRTUAL)
    CASE(UNSAFE)
    CASE(SIMD)
    CASE(TARGET)
    CASE(REF)
    CASE(ALIGNED)
    CASE(PACKED)
    CASE(ABSTRACT)
    CASE(ZEROCAST)
    CASE(EXTENDS)
    CASE(IMPLEMENTS)
    CASE(CONST_EXPR)
    CASE(SIZEOF)
    CASE(ALIGNOF)
    CASE(TYPEOF)
    CASE(ASM)
    CASE(IDENTIFIER)
    CASE(NUMBER)
    CASE(STRING_LITERAL)
    CASE(CHAR_LITERAL)
    CASE(TRUE)
    CASE(FALSE)
    CASE(NULL_VALUE)
    CASE(UNDEFINED)
    CASE(THIS)
    CASE(PLUS)
    CASE(MINUS)
    CASE(STAR)
    CASE(SLASH)
    CASE(PERCENT)
    CASE(AMPERSAND)
    CASE(PIPE)
    CASE(CARET)
    CASE(TILDE)
    CASE(RIGHT_SHIFT)
    CASE(LEFT_SHIFT)
    CASE(EXCLAIM)
    CASE(AMPERSAND_AMPERSAND)
    CASE(PIPE_PIPE)
    CASE(EQUALS)
    CASE(EQUALS_EQUALS)
    CASE(EXCLAIM_EQUALS)
    CASE(LESS)
    CASE(GREATER)
    CASE(LESS_EQUALS)
    CASE(GREATER_EQUALS)
    CASE(PLUS_EQUALS)
    CASE(MINUS_EQUALS)
    CASE(STAR_EQUALS)
    CASE(SLASH_EQUALS)
    CASE(PERCENT_EQUALS)
    CASE(AMPERSAND_EQUALS)
    CASE(PIPE_EQUALS)
    CASE(CARET_EQUALS)
    CASE(PLUS_PLUS)
    CASE(MINUS_MINUS)
    CASE(QUESTION)
    CASE(COLON)
    CASE(ARROW)
    CASE(DOT)
    CASE(AT)
    CASE(LEFT_PAREN)
    CASE(RIGHT_PAREN)
    CASE(LEFT_BRACE)
    CASE(RIGHT_BRACE)
    CASE(LEFT_BRACKET)
    CASE(RIGHT_BRACKET)
    CASE(SEMICOLON)
    CASE(COMMA)
    CASE(YIELD)
    CASE(ASYNC)
    CASE(AWAIT)
    CASE(ERROR_TOKEN)
    CASE(END_OF_FILE)
    default:
      return "UNKNOWN";
  }
#undef CASE
}
bool isOperator(TokenType type) {
  return type >= TokenType::OPERATOR_BEGIN && type <= TokenType::OPERATOR_END;
}
bool isLiteral(TokenType type) {
  return type >= TokenType::LITERAL_BEGIN && type <= TokenType::LITERAL_END;
}
bool isValidIdentifier(const std::string &s) {
  if (s.empty() || !std::isalpha(s[0]) && s[0] != '_') return false;
  for (char c : s)
    if (!std::isalnum(c) && c != '_') return false;
  return true;
}
bool isTypeKeyword(const std::string &word) {
  return word == "int" || word == "float" || word == "boolean" ||
         word == "string" || word == "void";
}
bool isAmbiguousToken(TokenType type) {
  return type == TokenType::LESS || type == TokenType::GREATER ||
         type == TokenType::AT || type == TokenType::ATTRIBUTE ||
         type == TokenType::PIPE;
}
}  // namespace TokenUtils
}  // namespace tokens
