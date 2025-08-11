#include <gc.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

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

extern "C" char* tspp_string_concat(const char* a, const char* b) {
  if (!a && !b) return nullptr;
  if (!a) a = "";
  if (!b) b = "";
  size_t la = std::strlen(a);
  size_t lb = std::strlen(b);
  // Allocate with Boehm GC so the IR side doesn't need to free explicitly
  char* out = (char*)GC_MALLOC(la + lb + 1);
  if (!out) return nullptr;
  std::memcpy(out, a, la);
  std::memcpy(out + la, b, lb);
  out[la + lb] = '\0';
  return out;
}

extern "C" void tspp_free_string(char* p) {
  free(p);
}
