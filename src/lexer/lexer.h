#pragma once
#include <string>
#include <vector>

enum class TokenType {
    Identifier,
    Keyword,
    Symbol,
    Literal,
    EndOfFile
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
public:
    Lexer(const std::string& src);
    std::vector<Token> tokenize();
private:
    std::string source;
    size_t pos;
    char peek() const;
    char get();
    void skipWhitespace();
    Token identifier();
    Token number();
    Token symbol();
};
