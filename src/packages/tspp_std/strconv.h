#pragma once

#include <stdint.h>
#include <stdbool.h>

// Result structures for error handling
struct TsppIntResult {
    int64_t value;
    char* error;
};

struct TsppUintResult {
    uint64_t value;
    char* error;
};

struct TsppFloatResult {
    double value;
    char* error;
};

struct TsppBoolResult {
    bool value;
    char* error;
};

struct TsppUnquoteResult {
    char* value;
    char* error;
};

extern "C" {
    // Integer conversion functions
    int64_t tspp_strconv_atoi(const char* s, bool* ok);
    int64_t tspp_strconv_parse_int(const char* s, int base, int bit_size, bool* ok);
    uint64_t tspp_strconv_parse_uint(const char* s, int base, int bit_size, bool* ok);

    // Error-returning integer functions
    TsppIntResult tspp_strconv_atoi_err(const char* s);
    TsppIntResult tspp_strconv_parse_int_err(const char* s, int base, int bit_size);
    TsppUintResult tspp_strconv_parse_uint_err(const char* s, int base, int bit_size);

    // Integer to string conversion
    char* tspp_strconv_itoa(int64_t i);
    char* tspp_strconv_format_int(int64_t i, int base);
    char* tspp_strconv_format_uint(uint64_t i, int base);

    // Floating point conversion functions
    double tspp_strconv_parse_float(const char* s, int bit_size, bool* ok);
    char* tspp_strconv_format_float(double f, char fmt, int prec, int bit_size);
    TsppFloatResult tspp_strconv_parse_float_err(const char* s, int bit_size);

    // Boolean conversion functions
    bool tspp_strconv_parse_bool(const char* s, bool* ok);
    char* tspp_strconv_format_bool(bool b);
    TsppBoolResult tspp_strconv_parse_bool_err(const char* s);

    // Quote and unquote functions for string literals
    char* tspp_strconv_quote(const char* s);
    char* tspp_strconv_quote_rune(int r);
    char* tspp_strconv_quote_to_ascii(const char* s);
    char* tspp_strconv_quote_to_graphic(const char* s);
    TsppUnquoteResult tspp_strconv_unquote(const char* s);

    // Utility functions for checking if string can be unquoted
    bool tspp_strconv_can_backquote(const char* s);
    bool tspp_strconv_is_graphic(int r);
    bool tspp_strconv_is_print(int r);

    // Append functions for efficient string building
    char* tspp_strconv_append_bool(char* dst, bool b);
    char* tspp_strconv_append_int(char* dst, int64_t i, int base);
    char* tspp_strconv_append_uint(char* dst, uint64_t i, int base);
    char* tspp_strconv_append_float(char* dst, double f, char fmt, int prec, int bit_size);
    char* tspp_strconv_append_quote(char* dst, const char* s);
    char* tspp_strconv_append_quote_rune(char* dst, int r);
    char* tspp_strconv_append_quote_to_ascii(char* dst, const char* s);
    char* tspp_strconv_append_quote_to_graphic(char* dst, const char* s);

    // Memory management for results
    void tspp_strconv_int_result_free(TsppIntResult* result);
    void tspp_strconv_uint_result_free(TsppUintResult* result);
    void tspp_strconv_float_result_free(TsppFloatResult* result);
    void tspp_strconv_bool_result_free(TsppBoolResult* result);
    void tspp_strconv_unquote_result_free(TsppUnquoteResult* result);
}