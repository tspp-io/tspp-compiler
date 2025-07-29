#include "DebugLog.h"
namespace common {
namespace DebugLog {
    void info(const std::string& msg) {
        std::cout << "[INFO] " << msg << std::endl;
    }
    void warn(const std::string& msg) {
        std::cout << "[WARN] " << msg << std::endl;
    }
    void error(const std::string& msg) {
        std::cerr << "[ERROR] " << msg << std::endl;
    }
}
}
