#include "os.h"

#include <string>
#include <thread>

static std::string tspp_os_platform_buffer;

extern "C" {

const char* tspp_os_platform() {
#if defined(_WIN32)
  tspp_os_platform_buffer = "win32";
#elif defined(__linux__)
  tspp_os_platform_buffer = "linux";
#elif defined(__APPLE__)
  tspp_os_platform_buffer = "darwin";
#else
  tspp_os_platform_buffer = "unknown";
#endif
  return tspp_os_platform_buffer.c_str();
}

int tspp_os_cpuCount() {
  return static_cast<int>(std::thread::hardware_concurrency());
}

}  // extern "C"
