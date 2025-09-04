#include "strings.h"
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#include <sstream>

// String builder implementation
TsppStringBuilder* tspp_strings_new_builder() {
    TsppStringBuilder* builder = (TsppStringBuilder*)malloc(sizeof(TsppStringBuilder));
    if (builder) {
        builder->internal = new std::string();
    }
    return builder;
}

void tspp_strings_builder_write_string(TsppStringBuilder* b, const char* s) {
    if (b && b->internal && s) {
        ((std::string*)b->internal)->append(s);
    }
}

void tspp_strings_builder_write_byte(TsppStringBuilder* b, char c) {
    if (b && b->internal) {
        ((std::string*)b->internal)->push_back(c);
    }
}

char* tspp_strings_builder_string(TsppStringBuilder* b) {
    if (b && b->internal) {
        return strdup(((std::string*)b->internal)->c_str());
    }
    return strdup("");
}

void tspp_strings_builder_reset(TsppStringBuilder* b) {
    if (b && b->internal) {
        ((std::string*)b->internal)->clear();
    }
}

size_t tspp_strings_builder_len(TsppStringBuilder* b) {
    if (b && b->internal) {
        return ((std::string*)b->internal)->length();
    }
    return 0;
}

void tspp_strings_builder_free(TsppStringBuilder* b) {
    if (b) {
        delete (std::string*)b->internal;
        free(b);
    }
}

// String comparison functions
int tspp_strings_compare(const char* a, const char* b) {
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    return strcmp(a, b);
}

bool tspp_strings_equal_fold(const char* s, const char* t) {
    if (!s || !t) return s == t;
    
    size_t len_s = strlen(s);
    size_t len_t = strlen(t);
    if (len_s != len_t) return false;
    
    for (size_t i = 0; i < len_s; i++) {
        if (tolower(s[i]) != tolower(t[i])) return false;
    }
    return true;
}

// String searching functions
bool tspp_strings_contains(const char* s, const char* substr) {
    if (!s || !substr) return false;
    return strstr(s, substr) != nullptr;
}

bool tspp_strings_contains_any(const char* s, const char* chars) {
    if (!s || !chars) return false;
    return strpbrk(s, chars) != nullptr;
}

bool tspp_strings_contains_rune(const char* s, int r) {
    if (!s) return false;
    return strchr(s, (char)r) != nullptr;
}

int tspp_strings_count(const char* s, const char* substr) {
    if (!s || !substr) return 0;
    
    int count = 0;
    size_t substr_len = strlen(substr);
    const char* pos = s;
    
    while ((pos = strstr(pos, substr)) != nullptr) {
        count++;
        pos += substr_len;
    }
    return count;
}

bool tspp_strings_has_prefix(const char* s, const char* prefix) {
    if (!s || !prefix) return false;
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

bool tspp_strings_has_suffix(const char* s, const char* suffix) {
    if (!s || !suffix) return false;
    
    size_t s_len = strlen(s);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > s_len) return false;
    
    return strcmp(s + s_len - suffix_len, suffix) == 0;
}

int tspp_strings_index(const char* s, const char* substr) {
    if (!s || !substr) return -1;
    
    const char* pos = strstr(s, substr);
    return pos ? (int)(pos - s) : -1;
}

int tspp_strings_index_byte(const char* s, char c) {
    if (!s) return -1;
    
    const char* pos = strchr(s, c);
    return pos ? (int)(pos - s) : -1;
}

int tspp_strings_last_index(const char* s, const char* substr) {
    if (!s || !substr) return -1;
    
    const char* last_pos = nullptr;
    const char* pos = s;
    size_t substr_len = strlen(substr);
    
    while ((pos = strstr(pos, substr)) != nullptr) {
        last_pos = pos;
        pos += substr_len;
    }
    
    return last_pos ? (int)(last_pos - s) : -1;
}

int tspp_strings_last_index_byte(const char* s, char c) {
    if (!s) return -1;
    
    const char* pos = strrchr(s, c);
    return pos ? (int)(pos - s) : -1;
}

// String manipulation functions
char* tspp_strings_join(const char** elems, size_t count, const char* sep) {
    if (!elems || count == 0) return strdup("");
    if (!sep) sep = "";
    
    // Calculate total length
    size_t total_len = 0;
    size_t sep_len = strlen(sep);
    
    for (size_t i = 0; i < count; i++) {
        if (elems[i]) {
            total_len += strlen(elems[i]);
            if (i < count - 1) total_len += sep_len;
        }
    }
    
    char* result = (char*)malloc(total_len + 1);
    if (!result) return nullptr;
    
    result[0] = '\0';
    for (size_t i = 0; i < count; i++) {
        if (elems[i]) {
            strcat(result, elems[i]);
            if (i < count - 1) strcat(result, sep);
        }
    }
    
    return result;
}

char* tspp_strings_repeat(const char* s, int count) {
    if (!s || count <= 0) return strdup("");
    
    size_t s_len = strlen(s);
    size_t total_len = s_len * count;
    char* result = (char*)malloc(total_len + 1);
    if (!result) return nullptr;
    
    result[0] = '\0';
    for (int i = 0; i < count; i++) {
        strcat(result, s);
    }
    
    return result;
}

char* tspp_strings_replace_all(const char* s, const char* old_str, const char* new_str) {
    return tspp_strings_replace(s, old_str, new_str, -1);
}

char* tspp_strings_replace(const char* s, const char* old_str, const char* new_str, int n) {
    if (!s || !old_str || !new_str) return strdup(s ? s : "");
    
    std::string result(s);
    size_t old_len = strlen(old_str);
    size_t new_len = strlen(new_str);
    size_t pos = 0;
    int replacements = 0;
    
    while ((pos = result.find(old_str, pos)) != std::string::npos && (n < 0 || replacements < n)) {
        result.replace(pos, old_len, new_str);
        pos += new_len;
        replacements++;
    }
    
    return strdup(result.c_str());
}

// String splitting functions
TsppStringArray* tspp_strings_split(const char* s, const char* sep) {
    return tspp_strings_split_n(s, sep, -1);
}

TsppStringArray* tspp_strings_split_n(const char* s, const char* sep, int n) {
    if (!s) return nullptr;
    
    TsppStringArray* arr = (TsppStringArray*)malloc(sizeof(TsppStringArray));
    if (!arr) return nullptr;
    
    std::vector<std::string> parts;
    std::string str(s);
    std::string separator = sep ? sep : "";
    
    if (separator.empty()) {
        // Split into individual characters
        for (char c : str) {
            parts.push_back(std::string(1, c));
            if (n > 0 && (int)parts.size() >= n) break;
        }
    } else {
        size_t start = 0;
        size_t end = 0;
        
        while ((end = str.find(separator, start)) != std::string::npos && (n < 0 || (int)parts.size() < n - 1)) {
            parts.push_back(str.substr(start, end - start));
            start = end + separator.length();
        }
        parts.push_back(str.substr(start));
    }
    
    // Convert to C array
    arr->len = parts.size();
    arr->cap = arr->len;
    arr->data = (char**)malloc(arr->len * sizeof(char*));
    
    if (arr->data) {
        for (size_t i = 0; i < arr->len; i++) {
            arr->data[i] = strdup(parts[i].c_str());
        }
    }
    
    return arr;
}

TsppStringArray* tspp_strings_fields(const char* s) {
    if (!s) return nullptr;
    
    TsppStringArray* arr = (TsppStringArray*)malloc(sizeof(TsppStringArray));
    if (!arr) return nullptr;
    
    std::vector<std::string> fields;
    std::istringstream iss(s);
    std::string field;
    
    while (iss >> field) {
        fields.push_back(field);
    }
    
    arr->len = fields.size();
    arr->cap = arr->len;
    arr->data = (char**)malloc(arr->len * sizeof(char*));
    
    if (arr->data) {
        for (size_t i = 0; i < arr->len; i++) {
            arr->data[i] = strdup(fields[i].c_str());
        }
    }
    
    return arr;
}

// String trimming functions
char* tspp_strings_trim_space(const char* s) {
    if (!s) return nullptr;
    
    // Find start of non-whitespace
    while (*s && isspace(*s)) s++;
    
    if (*s == 0) return strdup("");
    
    // Find end of non-whitespace  
    const char* end = s + strlen(s) - 1;
    while (end > s && isspace(*end)) end--;
    
    size_t len = end - s + 1;
    char* result = (char*)malloc(len + 1);
    if (result) {
        strncpy(result, s, len);
        result[len] = '\0';
    }
    
    return result;
}

char* tspp_strings_trim_prefix(const char* s, const char* prefix) {
    if (!s || !prefix) return strdup(s ? s : "");
    
    if (tspp_strings_has_prefix(s, prefix)) {
        return strdup(s + strlen(prefix));
    }
    return strdup(s);
}

char* tspp_strings_trim_suffix(const char* s, const char* suffix) {
    if (!s || !suffix) return strdup(s ? s : "");
    
    if (tspp_strings_has_suffix(s, suffix)) {
        size_t s_len = strlen(s);
        size_t suffix_len = strlen(suffix);
        char* result = (char*)malloc(s_len - suffix_len + 1);
        if (result) {
            strncpy(result, s, s_len - suffix_len);
            result[s_len - suffix_len] = '\0';
        }
        return result;
    }
    return strdup(s);
}

// Case conversion functions  
char* tspp_strings_to_lower(const char* s) {
    if (!s) return nullptr;
    
    char* result = strdup(s);
    if (result) {
        for (char* p = result; *p; p++) {
            *p = tolower(*p);
        }
    }
    return result;
}

char* tspp_strings_to_upper(const char* s) {
    if (!s) return nullptr;
    
    char* result = strdup(s);
    if (result) {
        for (char* p = result; *p; p++) {
            *p = toupper(*p);
        }
    }
    return result;
}

// Memory management
void tspp_strings_array_free(TsppStringArray* arr) {
    if (arr) {
        if (arr->data) {
            for (size_t i = 0; i < arr->len; i++) {
                free(arr->data[i]);
            }
            free(arr->data);
        }
        free(arr);
    }
}