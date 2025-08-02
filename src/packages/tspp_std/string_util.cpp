#include <cstdio>
#include <cstdlib>

extern "C" char* tspp_int_to_string(int value) {
  char* buf = (char*)malloc(32);
  if (buf) {
    snprintf(buf, 32, "%d", value);
  }
  return buf;
}

extern "C" char* tspp_float_to_string(float value) {
  char* buf = (char*)malloc(64);
  if (buf) {
    snprintf(buf, 64, "%g", value);
  }
  return buf;
}

extern "C" void tspp_free_string(char* p) {
  free(p);
}
