#include "errors.h"
#include <cstring>
#include <memory>

// Create a new error with the given text
TsppError* tspp_errors_new(const char* text) {
    if (!text) return nullptr;
    return new BasicError(std::string(text));
}

// Wrap an error with additional context
TsppError* tspp_errors_wrap(TsppError* err, const char* message) {
    if (!err || !message) return err;
    
    std::string wrapped_msg = std::string(message) + ": " + err->error();
    return new BasicError(wrapped_msg);
}

// Unwrap returns the result of calling the Unwrap method on err
// For basic errors, this returns nullptr
TsppError* tspp_errors_unwrap(TsppError* err) {
    // Basic implementation - in a full implementation, 
    // errors could implement an Unwrap() method
    return nullptr;
}

// Is reports whether any error in err's chain matches target
bool tspp_errors_is(TsppError* err, TsppError* target) {
    if (!err || !target) return false;
    
    // Simple string comparison for basic implementation
    return err->error() == target->error();
}

// As finds the first error in err's chain that matches target
bool tspp_errors_as(TsppError* err, TsppError** target) {
    if (!err || !target) return false;
    
    // Basic implementation - just assign if they match
    if (err->error() == (*target)->error()) {
        *target = err;
        return true;
    }
    return false;
}

// Convenience functions
TsppError* new_error(const char* text) {
    return tspp_errors_new(text);
}

void error_free(TsppError* err) {
    delete err;
}