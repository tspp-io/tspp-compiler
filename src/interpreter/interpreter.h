#pragma once
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "../codegen/LLVMCodeGenerator.h"

namespace interpreter {

class JITInterpreter {
 public:
  JITInterpreter();
  ~JITInterpreter();

  // Execute a line of tspp code, returns true if successful
  bool executeLine(const std::string& code);

  // For REPL variable persistence
  std::unordered_map<std::string, llvm::Value*> globals_;

 private:
  std::unique_ptr<llvm::orc::LLJIT> jit_;
  std::unique_ptr<codegen::LLVMCodeGenerator> codegen_;
  bool initializeJIT();
  void registerBuiltins();
};

}  // namespace interpreter
