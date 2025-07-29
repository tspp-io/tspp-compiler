#pragma once
#include <string>
namespace common {
/**
 * Location - Represents a position in source code (line, column).
 */
class Location {
public:
    Location(int line = 1, int col = 1) : line_(line), col_(col) {}
    int line() const { return line_; }
    int column() const { return col_; }
    std::string toString() const {
        return "[line " + std::to_string(line_) + ", col " + std::to_string(col_) + "]";
    }
private:
    int line_;
    int col_;
};
}
