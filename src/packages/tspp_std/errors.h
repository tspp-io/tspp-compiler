#pragma once

#include <stdexcept>
#include <string>
#include <memory>

// Error interface - base class for all errors
class TsppError {
public:
    virtual ~TsppError() = default;
    virtual const char* what() const noexcept = 0;
    virtual std::string error() const = 0;
};

// Basic error implementation
class BasicError : public TsppError {
private:
    std::string message;
    
public:
    explicit BasicError(const std::string& msg) : message(msg) {}
    
    const char* what() const noexcept override {
        return message.c_str();
    }
    
    std::string error() const override {
        return message;
    }
};

extern "C" {
    // Go errors package functions
    TsppError* tspp_errors_new(const char* text);
    TsppError* tspp_errors_wrap(TsppError* err, const char* message);
    TsppError* tspp_errors_unwrap(TsppError* err);
    bool tspp_errors_is(TsppError* err, TsppError* target);
    bool tspp_errors_as(TsppError* err, TsppError** target);

    // Error creation convenience functions
    TsppError* new_error(const char* text);
    void error_free(TsppError* err);
}