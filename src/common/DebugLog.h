#pragma once
#include <string>
#include <iostream>
namespace common {
/**
 * DebugLog - Utility for debug logging.
 */
namespace DebugLog {
    inline void info(const std::string& msg) {
        std::cout << "[INFO] " << msg << std::endl;
    }
    inline void warn(const std::string& msg) {
        std::cout << "[WARN] " << msg << std::endl;
    }
    inline void error(const std::string& msg) {
        std::cerr << "[ERROR] " << msg << std::endl;
    }
}
}
