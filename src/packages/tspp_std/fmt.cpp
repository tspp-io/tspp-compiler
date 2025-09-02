#include "fmt.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

// FormattedError implementation  
FormattedError::FormattedError(const char* format, va_list args) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    message = std::string(buffer);
}

// StringBuilder implementation
void StringBuilder::write_string(const char* s) {
    if (s) buffer << s;
}

void StringBuilder::write_int(int i) {
    buffer << i;
}

void StringBuilder::write_float(double f) {
    buffer << f;
}

void StringBuilder::write_char(char c) {
    buffer << c;
}

std::string StringBuilder::string() const {
    return buffer.str();
}

void StringBuilder::reset() {
    buffer.str("");
    buffer.clear();
}

// Error formatting
FormattedError* tspp_fmt_errorf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    FormattedError* err = new FormattedError(format, args);
    va_end(args);
    return err;
}

// Printf family implementations
int tspp_fmt_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

int tspp_fmt_fprintf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfprintf(stream, format, args);
    va_end(args);
    return result;
}

char* tspp_fmt_sprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // Calculate required size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, format, args_copy) + 1;
    va_end(args_copy);
    
    // Allocate and format
    char* buffer = (char*)malloc(size);
    if (buffer) {
        vsnprintf(buffer, size, format, args);
    }
    
    va_end(args);
    return buffer;
}

// Simple print implementations (simulating variadic with common types)
int tspp_fmt_print_string(const char* s) {
    return printf("%s", s);
}

int tspp_fmt_print_int(int i) {
    return printf("%d", i);
}

int tspp_fmt_print_float(double f) {
    return printf("%g", f);
}

int tspp_fmt_println_string(const char* s) {
    return printf("%s\n", s);
}

int tspp_fmt_println_int(int i) {
    return printf("%d\n", i);
}

int tspp_fmt_println_float(double f) {
    return printf("%g\n", f);
}

// Scan family implementations
int tspp_fmt_scanf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vscanf(format, args);
    va_end(args);
    return result;
}

int tspp_fmt_fscanf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfscanf(stream, format, args);
    va_end(args);
    return result;
}

int tspp_fmt_sscanf(const char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsscanf(str, format, args);
    va_end(args);
    return result;
}

StringBuilder* tspp_fmt_new_builder() {
    return new StringBuilder();
}

void tspp_fmt_builder_free(StringBuilder* sb) {
    delete sb;
}