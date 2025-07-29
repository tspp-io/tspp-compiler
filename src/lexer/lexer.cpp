#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& src) : source(src), pos(0) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (pos < source.size()) {
        skipWhitespace();
        if (pos >= source.size()) break;
        char c = peek();
        if (std::isalpha(c) || c == '_') {
            tokens.push_back(identifier());
        } else if (std::isdigit(c)) {
            tokens.push_back(number());
        } else {
            // Note: In TSPP++, semicolons (;) are optional at statement boundaries.
            // The lexer will tokenize ';' as a symbol, but the parser will handle its optionality.
            tokens.push_back(symbol());
        }
    }
    tokens.push_back({TokenType::EndOfFile, ""});
    return tokens;
}

char Lexer::peek() const {
    return pos < source.size() ? source[pos] : '\0';
}

char Lexer::get() {
    return pos < source.size() ? source[pos++] : '\0';
}

void Lexer::skipWhitespace() {
    while (pos < source.size() && std::isspace(source[pos])) ++pos;
}

Token Lexer::identifier() {
    size_t start = pos;
    while (pos < source.size() && (std::isalnum(source[pos]) || source[pos] == '_')) ++pos;
    std::string value = source.substr(start, pos - start);
    // TODO: Check for keywords
    return {TokenType::Identifier, value};
}

Token Lexer::number() {
    size_t start = pos;
    while (pos < source.size() && std::isdigit(source[pos])) ++pos;
    std::string value = source.substr(start, pos - start);
    return {TokenType::Literal, value};
}

Token Lexer::symbol() {
    char c = get();
    return {TokenType::Symbol, std::string(1, c)};
}
