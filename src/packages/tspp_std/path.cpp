#include "path.h"

#include <cstring>
#include <string>

static std::string tspp_path_buffer;

extern "C" {

const char* tspp_path_join(const char* a, const char* b) {
  tspp_path_buffer = std::string(a);
  if (!tspp_path_buffer.empty() && tspp_path_buffer.back() != '/')
    tspp_path_buffer += "/";
  tspp_path_buffer += b;
  return tspp_path_buffer.c_str();
}

const char* tspp_path_basename(const char* path) {
  const char* base = strrchr(path, '/');
  if (base) return base + 1;
  return path;
}

}  // extern "C"
