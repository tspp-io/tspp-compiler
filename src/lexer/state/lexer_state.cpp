#include "lexer_state.h"
// Lexer state implementation
namespace lexer {
namespace state {
void LexerState::advance(char c) {
    ++pos_;
    if (c == '\n') {
        ++line_;
        col_ = 1;
    } else {
        ++col_;
    }
}
}
}
