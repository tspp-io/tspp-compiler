#pragma once
#include <string>
namespace lexer {
namespace state {
class LexerState {
public:
    LexerState() : pos_(0), line_(1), col_(1) {}
    size_t pos() const { return pos_; }
    int line() const { return line_; }
    int col() const { return col_; }
    void advance(char c);
private:
    size_t pos_;
    int line_;
    int col_;
};
}
}
