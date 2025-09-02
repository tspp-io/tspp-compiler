#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdio>

// Include all the new package headers
#include "errors.h"
#include "fmt.h"
#include "strings.h"
#include "strconv.h"
#include "io.h"

// Test colors for output
#define GREEN "\033[32m"
#define RED "\033[31m" 
#define RESET "\033[0m"

int test_count = 0;
int passed_tests = 0;

void test_assert(bool condition, const char* test_name) {
    test_count++;
    if (condition) {
        std::cout << GREEN << "✅ " << test_name << RESET << std::endl;
        passed_tests++;
    } else {
        std::cout << RED << "❌ " << test_name << RESET << std::endl;
    }
}

void test_errors_package() {
    std::cout << "\n=== Testing errors package ===" << std::endl;
    
    // Test basic error creation
    TsppError* err1 = tspp_errors_new("test error");
    test_assert(err1 != nullptr, "errors.New creates error");
    test_assert(strcmp(err1->what(), "test error") == 0, "Error message matches");
    error_free(err1);
    
    // Test error wrapping
    TsppError* base_err = new_error("base error");
    TsppError* wrapped = tspp_errors_wrap(base_err, "wrapper");
    test_assert(wrapped != nullptr, "Error wrapping works");
    test_assert(strstr(wrapped->what(), "wrapper") != nullptr, "Wrapped error contains wrapper text");
    test_assert(strstr(wrapped->what(), "base error") != nullptr, "Wrapped error contains base error");
    error_free(base_err);
    error_free(wrapped);
}

void test_fmt_package() {
    std::cout << "\n=== Testing fmt package ===" << std::endl;
    
    // Test sprintf functionality
    char* result = tspp_fmt_sprintf("Hello %s, number %d", "world", 42);
    test_assert(result != nullptr, "fmt.Sprintf works");
    test_assert(strcmp(result, "Hello world, number 42") == 0, "fmt.Sprintf formats correctly");
    free(result);
    
    // Test StringBuilder
    StringBuilder* sb = tspp_fmt_new_builder();
    test_assert(sb != nullptr, "StringBuilder creation");
    
    sb->write_string("Hello ");
    sb->write_string("world!");
    sb->write_int(123);
    
    std::string str_result = sb->string();
    test_assert(str_result == "Hello world!123", "StringBuilder concatenation works");
    
    sb->reset();
    sb->write_string("Reset test");
    std::string reset_result = sb->string();
    test_assert(reset_result == "Reset test", "StringBuilder reset works");
    
    tspp_fmt_builder_free(sb);
    
    // Test error formatting
    FormattedError* fmt_err = tspp_fmt_errorf("Error code: %d, message: %s", 404, "Not Found");
    test_assert(fmt_err != nullptr, "fmt.Errorf works");
    test_assert(strstr(fmt_err->what(), "404") != nullptr, "fmt.Errorf formats numbers");
    test_assert(strstr(fmt_err->what(), "Not Found") != nullptr, "fmt.Errorf formats strings");
    delete fmt_err;
}

void test_strings_package() {
    std::cout << "\n=== Testing strings package ===" << std::endl;
    
    // Test string comparison
    test_assert(tspp_strings_compare("abc", "abc") == 0, "strings.Compare equal strings");
    test_assert(tspp_strings_compare("abc", "def") < 0, "strings.Compare abc < def");
    test_assert(tspp_strings_equal_fold("ABC", "abc"), "strings.EqualFold case insensitive");
    
    // Test string searching
    test_assert(tspp_strings_contains("hello world", "world"), "strings.Contains finds substring");
    test_assert(!tspp_strings_contains("hello", "world"), "strings.Contains doesn't find missing substring");
    test_assert(tspp_strings_has_prefix("hello world", "hello"), "strings.HasPrefix works");
    test_assert(tspp_strings_has_suffix("hello world", "world"), "strings.HasSuffix works");
    
    // Test string manipulation
    const char* join_parts[] = {"a", "b", "c"};
    char* joined = tspp_strings_join(join_parts, 3, ",");
    test_assert(strcmp(joined, "a,b,c") == 0, "strings.Join works");
    free(joined);
    
    char* repeated = tspp_strings_repeat("abc", 3);
    test_assert(strcmp(repeated, "abcabcabc") == 0, "strings.Repeat works");
    free(repeated);
    
    char* replaced = tspp_strings_replace_all("hello world world", "world", "universe");
    test_assert(strcmp(replaced, "hello universe universe") == 0, "strings.ReplaceAll works");
    free(replaced);
    
    // Test string splitting
    TsppStringArray* parts = tspp_strings_split("a,b,c", ",");
    test_assert(parts != nullptr && parts->len == 3, "strings.Split creates correct number of parts");
    test_assert(strcmp(parts->data[0], "a") == 0, "strings.Split first part correct");
    test_assert(strcmp(parts->data[1], "b") == 0, "strings.Split second part correct");
    test_assert(strcmp(parts->data[2], "c") == 0, "strings.Split third part correct");
    tspp_strings_array_free(parts);
    
    // Test trimming
    char* trimmed = tspp_strings_trim_space("  hello world  ");
    test_assert(strcmp(trimmed, "hello world") == 0, "strings.TrimSpace works");
    free(trimmed);
    
    // Test case conversion
    char* lower = tspp_strings_to_lower("HELLO WORLD");
    test_assert(strcmp(lower, "hello world") == 0, "strings.ToLower works");
    free(lower);
    
    char* upper = tspp_strings_to_upper("hello world");
    test_assert(strcmp(upper, "HELLO WORLD") == 0, "strings.ToUpper works");
    free(upper);
    
    // Test string builder
    TsppStringBuilder* builder = tspp_strings_new_builder();
    test_assert(builder != nullptr, "strings.Builder creation");
    
    tspp_strings_builder_write_string(builder, "Hello");
    tspp_strings_builder_write_byte(builder, ' ');
    tspp_strings_builder_write_string(builder, "World!");
    
    char* built = tspp_strings_builder_string(builder);
    test_assert(strcmp(built, "Hello World!") == 0, "strings.Builder concatenation");
    free(built);
    
    tspp_strings_builder_free(builder);
}

void test_strconv_package() {
    std::cout << "\n=== Testing strconv package ===" << std::endl;
    
    // Test integer parsing
    bool ok;
    int64_t num = tspp_strconv_atoi("123", &ok);
    test_assert(ok && num == 123, "strconv.Atoi parses positive number");
    
    num = tspp_strconv_atoi("-456", &ok);
    test_assert(ok && num == -456, "strconv.Atoi parses negative number");
    
    num = tspp_strconv_atoi("invalid", &ok);
    test_assert(!ok, "strconv.Atoi rejects invalid input");
    
    // Test different bases
    num = tspp_strconv_parse_int("ff", 16, 0, &ok);
    test_assert(ok && num == 255, "strconv.ParseInt handles hex");
    
    num = tspp_strconv_parse_int("1010", 2, 0, &ok);
    test_assert(ok && num == 10, "strconv.ParseInt handles binary");
    
    // Test unsigned parsing
    uint64_t unum = tspp_strconv_parse_uint("123", 10, 0, &ok);
    test_assert(ok && unum == 123, "strconv.ParseUint works");
    
    // Test integer formatting
    char* int_str = tspp_strconv_itoa(123);
    test_assert(strcmp(int_str, "123") == 0, "strconv.Itoa formats positive");
    free(int_str);
    
    int_str = tspp_strconv_itoa(-456);
    test_assert(strcmp(int_str, "-456") == 0, "strconv.Itoa formats negative");
    free(int_str);
    
    char* hex_str = tspp_strconv_format_int(255, 16);
    test_assert(strcmp(hex_str, "ff") == 0, "strconv.FormatInt handles hex");
    free(hex_str);
    
    // Test float parsing
    double f = tspp_strconv_parse_float("3.14", 64, &ok);
    test_assert(ok && f > 3.13 && f < 3.15, "strconv.ParseFloat works");
    
    f = tspp_strconv_parse_float("invalid", 64, &ok);
    test_assert(!ok, "strconv.ParseFloat rejects invalid input");
    
    // Test float formatting
    char* float_str = tspp_strconv_format_float(3.14159, 'f', 2, 64);
    test_assert(strcmp(float_str, "3.14") == 0, "strconv.FormatFloat works");
    free(float_str);
    
    // Test boolean parsing
    bool b = tspp_strconv_parse_bool("true", &ok);
    test_assert(ok && b == true, "strconv.ParseBool parses true");
    
    b = tspp_strconv_parse_bool("false", &ok);
    test_assert(ok && b == false, "strconv.ParseBool parses false");
    
    b = tspp_strconv_parse_bool("1", &ok);
    test_assert(ok && b == true, "strconv.ParseBool parses 1 as true");
    
    b = tspp_strconv_parse_bool("0", &ok);
    test_assert(ok && b == false, "strconv.ParseBool parses 0 as false");
    
    // Test boolean formatting
    char* bool_str = tspp_strconv_format_bool(true);
    test_assert(strcmp(bool_str, "true") == 0, "strconv.FormatBool formats true");
    free(bool_str);
    
    bool_str = tspp_strconv_format_bool(false);
    test_assert(strcmp(bool_str, "false") == 0, "strconv.FormatBool formats false");
    free(bool_str);
    
    // Test quoting
    char* quoted = tspp_strconv_quote("hello \"world\"");
    test_assert(strstr(quoted, "\\\"") != nullptr, "strconv.Quote escapes quotes");
    free(quoted);
}

void test_io_package() {
    std::cout << "\n=== Testing io package ===" << std::endl;
    
    // Test string reader
    TsppStringReader* sr = tspp_io_new_string_reader("Hello World!");
    test_assert(sr != nullptr, "io.StringReader creation");
    
    char buffer[20];
    int bytes_read = tspp_io_string_reader_read(sr, buffer, 5);
    test_assert(bytes_read == 5, "io.StringReader reads correct amount");
    buffer[5] = '\0';
    test_assert(strcmp(buffer, "Hello") == 0, "io.StringReader reads correct data");
    
    tspp_io_string_reader_free(sr);
    
    // Test string writer
    TsppStringWriter* sw = tspp_io_new_string_writer();
    test_assert(sw != nullptr, "io.StringWriter creation");
    
    int bytes_written = tspp_io_string_writer_write(sw, "Hello ", 6);
    test_assert(bytes_written == 6, "io.StringWriter writes correct amount");
    
    bytes_written = tspp_io_string_writer_write(sw, "World!", 6);
    test_assert(bytes_written == 6, "io.StringWriter appends correctly");
    
    char* result = tspp_io_string_writer_string(sw);
    test_assert(strcmp(result, "Hello World!") == 0, "io.StringWriter builds correct string");
    free(result);
    
    tspp_io_string_writer_free(sw);
    
    // Test buffered I/O with string reader/writer
    sr = tspp_io_new_string_reader("Line 1\nLine 2\nLine 3");
    TsppReader reader = { sr, (int (*)(void*, char*, size_t))tspp_io_string_reader_read, nullptr };
    
    TsppBufReader* br = tspp_io_new_buf_reader(&reader);
    test_assert(br != nullptr, "io.BufReader creation");
    
    bytes_read = tspp_io_buf_reader_read(br, buffer, 6);
    test_assert(bytes_read == 6, "io.BufReader reads data");
    buffer[6] = '\0';
    test_assert(strcmp(buffer, "Line 1") == 0, "io.BufReader reads correct data");
    
    tspp_io_buf_reader_free(br);
    tspp_io_string_reader_free(sr);
}

int main() {
    std::cout << "🚀 Running comprehensive Go standard library tests for TSPP++\n" << std::endl;
    
    test_errors_package();
    test_fmt_package();
    test_strings_package(); 
    test_strconv_package();
    test_io_package();
    
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Passed: " << GREEN << passed_tests << "/" << test_count << RESET << std::endl;
    
    if (passed_tests == test_count) {
        std::cout << GREEN << "🎉 All tests passed! Go standard library implementation is working correctly." << RESET << std::endl;
        return 0;
    } else {
        std::cout << RED << "❌ " << (test_count - passed_tests) << " tests failed." << RESET << std::endl;
        return 1;
    }
}