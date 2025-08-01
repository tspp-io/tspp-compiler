#include "fs.h"

#include <sys/stat.h>

#include <fstream>
#include <mutex>
#include <string>

// Simple static buffer for returned strings (not thread-safe, but safe for
// demo)
static std::string tspp_fs_buffer;
static std::mutex tspp_fs_mutex;

extern "C" {

const char* tspp_fs_readFile(const char* path) {
  std::lock_guard<std::mutex> lock(tspp_fs_mutex);
  std::ifstream file(path);
  if (!file) {
    tspp_fs_buffer = "";
    return tspp_fs_buffer.c_str();
  }
  tspp_fs_buffer.assign((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
  return tspp_fs_buffer.c_str();
}

void tspp_fs_writeFile(const char* path, const char* content) {
  std::ofstream file(path);
  if (file) {
    file << content;
  }
}

bool tspp_fs_exists(const char* path) {
  struct stat buffer;
  return (stat(path, &buffer) == 0);
}

}  // extern "C"
