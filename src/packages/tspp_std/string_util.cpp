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

extern "C" char* tspp_bool_to_string(bool value) {
  char* buf = (char*)malloc(8);
  if (buf) {
    snprintf(buf, 8, "%s", value ? "true" : "false");
  }
  return buf;
}

extern "C" char* tspp_ptr_to_string(void* value) {
  char* buf = (char*)malloc(32);
  if (buf) {
    // Print pointer as 0x... with platform-appropriate width
    snprintf(buf, 32, "%p", value);
  }
  return buf;
}

extern "C" void tspp_free_string(char* p) {
  free(p);
}
