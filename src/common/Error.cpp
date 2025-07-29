#include "Error.h"

namespace common {
Error::Error(const std::string& msg, const Location& loc) : msg_(msg), loc_(loc) {}
const std::string& Error::message() const { return msg_; }
const Location& Error::location() const { return loc_; }
std::string Error::toString() const {
    return "Error: " + msg_ + " at " + loc_.toString();
}
}
