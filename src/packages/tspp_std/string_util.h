#pragma once
#ifdef __cplusplus
extern "C" {
#endif
char* tspp_int_to_string(int value);
char* tspp_float_to_string(float value);
void tspp_free_string(char* p);
#ifdef __cplusplus
}
#endif
