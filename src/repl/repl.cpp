// Placeholder for repl implementation
#include <iostream>
#include <string>

#include "../interpreter/interpreter.h"

int main() {
  interpreter::JITInterpreter repl;
  std::string line, codeBuffer;
  std::cout << "tspp REPL (JIT mode)\n";
  while (true) {
    std::cout << ">>> ";
    if (!std::getline(std::cin, line)) break;
    if (line.empty()) continue;
    codeBuffer += line + "\n";
    // Simple multiline detection: if line ends with '{', keep reading
    if (line.find('{') != std::string::npos &&
        line.find('}') == std::string::npos) {
      continue;
    }
    if (!repl.executeLine(codeBuffer)) {
      std::cerr << "Error executing line\n";
    }
    codeBuffer.clear();
  }
  return 0;
}
// Placeholder for repl implementation
