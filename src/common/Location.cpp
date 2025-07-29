#include "Location.h"
#include <sstream>

namespace common {
Location::Location(int line, int col) : line_(line), col_(col) {}
int Location::line() const { return line_; }
int Location::column() const { return col_; }
std::string Location::toString() const {
    std::ostringstream oss;
    oss << "[line " << line_ << ", col " << col_ << "]";
    return oss.str();
}
}
