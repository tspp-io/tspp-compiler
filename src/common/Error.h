#pragma once
#include <string>
#include "Location.h"
namespace common {
/**
 * Error - Represents a language error with location and message.
 */
class Error {
public:
    Error(const std::string& msg, const Location& loc) : msg_(msg), loc_(loc) {}
    const std::string& message() const { return msg_; }
    const Location& location() const { return loc_; }
    std::string toString() const {
        return "Error: " + msg_ + " at " + loc_.toString();
    }
private:
    std::string msg_;
    Location loc_;
};
}
