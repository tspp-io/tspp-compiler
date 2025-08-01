#include "console.h"

#include <iostream>

extern "C" {

void tspp_console_log(const char* msg) {
  std::cout << msg << std::endl;
}

void tspp_console_error(const char* msg) {
  std::cerr << "\033[1;31m[error]\033[0m " << msg << std::endl;
}

void tspp_console_warn(const char* msg) {
  std::cerr << "\033[1;33m[warn]\033[0m " << msg << std::endl;
}

}  // extern "C"
