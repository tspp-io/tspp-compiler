#pragma once
#ifdef __cplusplus
extern "C" {
#endif
char* tspp_int_to_string(int value);
char* tspp_float_to_string(float value);
char* tspp_bool_to_string(bool value);
char* tspp_ptr_to_string(void* value);
// Concatenate two C-strings; result is heap-allocated and managed by the GC.
char* tspp_string_concat(const char* a, const char* b);
void tspp_free_string(char* p);
#ifdef __cplusplus
}
#endif
