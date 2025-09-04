#pragma once

#include <cstdio>
#include <cstdarg>
#include <string>
#include <sstream>

// Error formatting
class FormattedError {
private:
    std::string message;
public:
    FormattedError(const char* format, va_list args);
    const char* what() const { return message.c_str(); }
    std::string error() const { return message; }
};

// String builders for complex formatting
class StringBuilder {
private:
    std::ostringstream buffer;
public:
    void write_string(const char* s);
    void write_int(int i);
    void write_float(double f);
    void write_char(char c);
    std::string string() const;
    void reset();
};

extern "C" {
    // Printf family - formatted printing to stdout
    int tspp_fmt_printf(const char* format, ...);
    int tspp_fmt_fprintf(FILE* stream, const char* format, ...);
    char* tspp_fmt_sprintf(const char* format, ...);

    // Print family - space-separated values
    int tspp_fmt_print_string(const char* s);
    int tspp_fmt_print_int(int i);
    int tspp_fmt_print_float(double f);
    
    // Println family - space-separated values with newline
    int tspp_fmt_println_string(const char* s);
    int tspp_fmt_println_int(int i);
    int tspp_fmt_println_float(double f);

    // Scan family - formatted input
    int tspp_fmt_scanf(const char* format, ...);
    int tspp_fmt_fscanf(FILE* stream, const char* format, ...);
    int tspp_fmt_sscanf(const char* str, const char* format, ...);

    FormattedError* tspp_fmt_errorf(const char* format, ...);

    StringBuilder* tspp_fmt_new_builder();
    void tspp_fmt_builder_free(StringBuilder* sb);
}