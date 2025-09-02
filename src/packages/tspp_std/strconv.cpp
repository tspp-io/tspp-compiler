#include "strconv.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cctype>
#include <climits>
#include <cfloat>

// Integer conversion functions
int64_t tspp_strconv_atoi(const char* s, bool* ok) {
    return tspp_strconv_parse_int(s, 10, 0, ok);
}

int64_t tspp_strconv_parse_int(const char* s, int base, int bit_size, bool* ok) {
    if (!s || !ok) {
        if (ok) *ok = false;
        return 0;
    }
    
    *ok = true;
    errno = 0;
    
    char* endptr;
    long long result = strtoll(s, &endptr, base);
    
    // Check for conversion errors
    if (errno == ERANGE || endptr == s || *endptr != '\0') {
        *ok = false;
        return 0;
    }
    
    // Check bit size constraints
    if (bit_size > 0) {
        int64_t max_val, min_val;
        switch (bit_size) {
            case 8:
                max_val = 127;
                min_val = -128;
                break;
            case 16:
                max_val = 32767;
                min_val = -32768;
                break;
            case 32:
                max_val = 2147483647;
                min_val = -2147483648LL;
                break;
            case 64:
                max_val = LLONG_MAX;
                min_val = LLONG_MIN;
                break;
            default:
                *ok = false;
                return 0;
        }
        
        if (result > max_val || result < min_val) {
            *ok = false;
            return 0;
        }
    }
    
    return (int64_t)result;
}

uint64_t tspp_strconv_parse_uint(const char* s, int base, int bit_size, bool* ok) {
    if (!s || !ok) {
        if (ok) *ok = false;
        return 0;
    }
    
    *ok = true;
    errno = 0;
    
    char* endptr;
    unsigned long long result = strtoull(s, &endptr, base);
    
    // Check for conversion errors
    if (errno == ERANGE || endptr == s || *endptr != '\0') {
        *ok = false;
        return 0;
    }
    
    // Check bit size constraints
    if (bit_size > 0) {
        uint64_t max_val;
        switch (bit_size) {
            case 8:
                max_val = 255;
                break;
            case 16:
                max_val = 65535;
                break;
            case 32:
                max_val = 4294967295U;
                break;
            case 64:
                max_val = ULLONG_MAX;
                break;
            default:
                *ok = false;
                return 0;
        }
        
        if (result > max_val) {
            *ok = false;
            return 0;
        }
    }
    
    return (uint64_t)result;
}

// Error-returning versions
TsppIntResult tspp_strconv_atoi_err(const char* s) {
    return tspp_strconv_parse_int_err(s, 10, 0);
}

TsppIntResult tspp_strconv_parse_int_err(const char* s, int base, int bit_size) {
    TsppIntResult result;
    bool ok;
    result.value = tspp_strconv_parse_int(s, base, bit_size, &ok);
    
    if (ok) {
        result.error = nullptr;
    } else {
        result.error = strdup("strconv.ParseInt: invalid syntax");
    }
    
    return result;
}

TsppUintResult tspp_strconv_parse_uint_err(const char* s, int base, int bit_size) {
    TsppUintResult result;
    bool ok;
    result.value = tspp_strconv_parse_uint(s, base, bit_size, &ok);
    
    if (ok) {
        result.error = nullptr;
    } else {
        result.error = strdup("strconv.ParseUint: invalid syntax");
    }
    
    return result;
}

// Integer to string conversion
char* tspp_strconv_itoa(int64_t i) {
    return tspp_strconv_format_int(i, 10);
}

char* tspp_strconv_format_int(int64_t i, int base) {
    if (base < 2 || base > 36) {
        return strdup("");
    }
    
    char buffer[65]; // Enough for 64-bit int in binary
    snprintf(buffer, sizeof(buffer), "%lld", (long long)i);
    
    if (base != 10) {
        // Convert to specified base
        bool negative = i < 0;
        uint64_t abs_val = negative ? -i : i;
        
        char temp[65];
        int pos = 0;
        
        if (abs_val == 0) {
            temp[pos++] = '0';
        } else {
            while (abs_val > 0) {
                int digit = abs_val % base;
                temp[pos++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
                abs_val /= base;
            }
        }
        
        if (negative) {
            temp[pos++] = '-';
        }
        
        temp[pos] = '\0';
        
        // Reverse the string
        for (int i = 0; i < pos / 2; i++) {
            char tmp = temp[i];
            temp[i] = temp[pos - 1 - i];
            temp[pos - 1 - i] = tmp;
        }
        
        return strdup(temp);
    }
    
    return strdup(buffer);
}

char* tspp_strconv_format_uint(uint64_t i, int base) {
    if (base < 2 || base > 36) {
        return strdup("");
    }
    
    if (base == 10) {
        char buffer[21]; // Enough for 64-bit uint in decimal
        snprintf(buffer, sizeof(buffer), "%llu", (unsigned long long)i);
        return strdup(buffer);
    }
    
    char buffer[65];
    int pos = 0;
    
    if (i == 0) {
        buffer[pos++] = '0';
    } else {
        while (i > 0) {
            int digit = i % base;
            buffer[pos++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            i /= base;
        }
    }
    
    buffer[pos] = '\0';
    
    // Reverse the string
    for (int j = 0; j < pos / 2; j++) {
        char tmp = buffer[j];
        buffer[j] = buffer[pos - 1 - j];
        buffer[pos - 1 - j] = tmp;
    }
    
    return strdup(buffer);
}

// Floating point conversion
double tspp_strconv_parse_float(const char* s, int bit_size, bool* ok) {
    if (!s || !ok) {
        if (ok) *ok = false;
        return 0.0;
    }
    
    *ok = true;
    errno = 0;
    
    char* endptr;
    double result = strtod(s, &endptr);
    
    if (errno == ERANGE || endptr == s || *endptr != '\0') {
        *ok = false;
        return 0.0;
    }
    
    // Check bit size constraints for float32
    if (bit_size == 32) {
        if (result > FLT_MAX || result < -FLT_MAX) {
            *ok = false;
            return 0.0;
        }
    }
    
    return result;
}

char* tspp_strconv_format_float(double f, char fmt, int prec, int bit_size) {
    char buffer[64];
    
    switch (fmt) {
        case 'f':
        case 'F':
            snprintf(buffer, sizeof(buffer), "%.*f", prec, f);
            break;
        case 'e':
            snprintf(buffer, sizeof(buffer), "%.*e", prec, f);
            break;
        case 'E':
            snprintf(buffer, sizeof(buffer), "%.*E", prec, f);
            break;
        case 'g':
            snprintf(buffer, sizeof(buffer), "%.*g", prec, f);
            break;
        case 'G':
            snprintf(buffer, sizeof(buffer), "%.*G", prec, f);
            break;
        default:
            snprintf(buffer, sizeof(buffer), "%g", f);
            break;
    }
    
    return strdup(buffer);
}

TsppFloatResult tspp_strconv_parse_float_err(const char* s, int bit_size) {
    TsppFloatResult result;
    bool ok;
    result.value = tspp_strconv_parse_float(s, bit_size, &ok);
    
    if (ok) {
        result.error = nullptr;
    } else {
        result.error = strdup("strconv.ParseFloat: invalid syntax");
    }
    
    return result;
}

// Boolean conversion
bool tspp_strconv_parse_bool(const char* s, bool* ok) {
    if (!s || !ok) {
        if (ok) *ok = false;
        return false;
    }
    
    *ok = true;
    
    if (strcmp(s, "true") == 0 || strcmp(s, "1") == 0) {
        return true;
    } else if (strcmp(s, "false") == 0 || strcmp(s, "0") == 0) {
        return false;
    }
    
    *ok = false;
    return false;
}

char* tspp_strconv_format_bool(bool b) {
    return strdup(b ? "true" : "false");
}

TsppBoolResult tspp_strconv_parse_bool_err(const char* s) {
    TsppBoolResult result;
    bool ok;
    result.value = tspp_strconv_parse_bool(s, &ok);
    
    if (ok) {
        result.error = nullptr;
    } else {
        result.error = strdup("strconv.ParseBool: invalid syntax");
    }
    
    return result;
}

// Quote functions (basic implementation)
char* tspp_strconv_quote(const char* s) {
    if (!s) return strdup("\"\"");
    
    size_t len = strlen(s);
    char* result = (char*)malloc(len * 4 + 3); // Worst case: all chars need escaping + quotes
    if (!result) return nullptr;
    
    char* p = result;
    *p++ = '"';
    
    for (const char* c = s; *c; c++) {
        switch (*c) {
            case '"':
                *p++ = '\\';
                *p++ = '"';
                break;
            case '\\':
                *p++ = '\\';
                *p++ = '\\';
                break;
            case '\n':
                *p++ = '\\';
                *p++ = 'n';
                break;
            case '\r':
                *p++ = '\\';
                *p++ = 'r';
                break;
            case '\t':
                *p++ = '\\';
                *p++ = 't';
                break;
            default:
                if (isprint(*c)) {
                    *p++ = *c;
                } else {
                    sprintf(p, "\\x%02x", (unsigned char)*c);
                    p += 4;
                }
                break;
        }
    }
    
    *p++ = '"';
    *p = '\0';
    
    return result;
}

TsppUnquoteResult tspp_strconv_unquote(const char* s) {
    TsppUnquoteResult result;
    
    if (!s || strlen(s) < 2 || s[0] != '"' || s[strlen(s)-1] != '"') {
        result.value = nullptr;
        result.error = strdup("strconv.Unquote: invalid syntax");
        return result;
    }
    
    size_t len = strlen(s) - 2; // Remove quotes
    char* unquoted = (char*)malloc(len + 1);
    if (!unquoted) {
        result.value = nullptr;
        result.error = strdup("strconv.Unquote: out of memory");
        return result;
    }
    
    const char* src = s + 1; // Skip opening quote
    char* dst = unquoted;
    
    for (size_t i = 0; i < len; i++) {
        if (src[i] == '\\' && i + 1 < len) {
            switch (src[i + 1]) {
                case 'n': *dst++ = '\n'; i++; break;
                case 'r': *dst++ = '\r'; i++; break;
                case 't': *dst++ = '\t'; i++; break;
                case '\\': *dst++ = '\\'; i++; break;
                case '"': *dst++ = '"'; i++; break;
                default: *dst++ = src[i]; break;
            }
        } else {
            *dst++ = src[i];
        }
    }
    
    *dst = '\0';
    
    result.value = unquoted;
    result.error = nullptr;
    return result;
}

// Memory management functions
void tspp_strconv_int_result_free(TsppIntResult* result) {
    if (result && result->error) {
        free(result->error);
        result->error = nullptr;
    }
}

void tspp_strconv_uint_result_free(TsppUintResult* result) {
    if (result && result->error) {
        free(result->error);
        result->error = nullptr;
    }
}

void tspp_strconv_float_result_free(TsppFloatResult* result) {
    if (result && result->error) {
        free(result->error);
        result->error = nullptr;
    }
}

void tspp_strconv_bool_result_free(TsppBoolResult* result) {
    if (result && result->error) {
        free(result->error);
        result->error = nullptr;
    }
}

void tspp_strconv_unquote_result_free(TsppUnquoteResult* result) {
    if (result) {
        if (result->value) {
            free(result->value);
            result->value = nullptr;
        }
        if (result->error) {
            free(result->error);
            result->error = nullptr;
        }
    }
}