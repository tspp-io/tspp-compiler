#pragma once

#include <cstddef>

// Forward declare to avoid conflicts with io.h
struct TsppStringReader;

// String array structure
struct TsppStringArray {
    char** data;
    size_t len;
    size_t cap;
};

// String builder for efficient string concatenation
struct TsppStringBuilder {
    void* internal;  // Will be std::string* in implementation
};

extern "C" {
    // String comparison functions
    int tspp_strings_compare(const char* a, const char* b);
    bool tspp_strings_equal_fold(const char* s, const char* t);

    // String searching functions  
    bool tspp_strings_contains(const char* s, const char* substr);
    bool tspp_strings_contains_any(const char* s, const char* chars);
    bool tspp_strings_contains_rune(const char* s, int r);
    int tspp_strings_count(const char* s, const char* substr);
    bool tspp_strings_has_prefix(const char* s, const char* prefix);
    bool tspp_strings_has_suffix(const char* s, const char* suffix);
    int tspp_strings_index(const char* s, const char* substr);
    int tspp_strings_index_any(const char* s, const char* chars);
    int tspp_strings_index_byte(const char* s, char c);
    int tspp_strings_index_rune(const char* s, int r);
    int tspp_strings_last_index(const char* s, const char* substr);
    int tspp_strings_last_index_any(const char* s, const char* chars);
    int tspp_strings_last_index_byte(const char* s, char c);

    // String manipulation functions
    char* tspp_strings_join(const char** elems, size_t count, const char* sep);
    char* tspp_strings_repeat(const char* s, int count);
    char* tspp_strings_replace(const char* s, const char* old_str, const char* new_str, int n);
    char* tspp_strings_replace_all(const char* s, const char* old_str, const char* new_str);

    // String splitting functions
    TsppStringArray* tspp_strings_split(const char* s, const char* sep);
    TsppStringArray* tspp_strings_split_n(const char* s, const char* sep, int n);
    TsppStringArray* tspp_strings_split_after(const char* s, const char* sep);
    TsppStringArray* tspp_strings_split_after_n(const char* s, const char* sep, int n);
    TsppStringArray* tspp_strings_fields(const char* s);
    TsppStringArray* tspp_strings_fields_func(const char* s, bool (*f)(int));

    // String trimming functions
    char* tspp_strings_trim(const char* s, const char* cutset);
    char* tspp_strings_trim_left(const char* s, const char* cutset);
    char* tspp_strings_trim_right(const char* s, const char* cutset);
    char* tspp_strings_trim_space(const char* s);
    char* tspp_strings_trim_prefix(const char* s, const char* prefix);
    char* tspp_strings_trim_suffix(const char* s, const char* suffix);

    // Case conversion functions
    char* tspp_strings_to_lower(const char* s);
    char* tspp_strings_to_upper(const char* s);
    char* tspp_strings_to_title(const char* s);

    // String builder functions
    TsppStringBuilder* tspp_strings_new_builder();
    void tspp_strings_builder_write_string(TsppStringBuilder* b, const char* s);
    void tspp_strings_builder_write_byte(TsppStringBuilder* b, char c);
    void tspp_strings_builder_write_rune(TsppStringBuilder* b, int r);
    char* tspp_strings_builder_string(TsppStringBuilder* b);
    void tspp_strings_builder_reset(TsppStringBuilder* b);
    size_t tspp_strings_builder_len(TsppStringBuilder* b);
    size_t tspp_strings_builder_cap(TsppStringBuilder* b);
    void tspp_strings_builder_grow(TsppStringBuilder* b, size_t n);
    void tspp_strings_builder_free(TsppStringBuilder* b);

    // Memory management for string arrays
    void tspp_strings_array_free(TsppStringArray* arr);
}