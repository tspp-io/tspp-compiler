#include <llvm/Support/ManagedStatic.h>

#include <fstream>
#include <iostream>
#include <string>

#include "codegen/LLVMCodeGenerator.h"
#include "interpreter/interpreter.h"
#include "lexer/lexer.h"
#include "parser/module/ModuleResolver.h"
#include "parser/parser.h"
#include "parser/visitors/semantic/SemanticAnalyzerVisitor.h"

int main(int argc, char* argv[]) {
  llvm::llvm_shutdown_obj shutdown_on_exit;
  if (argc < 2) {
    // REPL mode
    std::cout << "TSPP++ REPL (JIT mode)\n";
    interpreter::JITInterpreter repl;
    std::string line, codeBuffer;
    while (true) {
      std::cout << ">>> ";
      if (!std::getline(std::cin, line)) break;
      if (line.empty()) continue;
      codeBuffer += line + "\n";
      if (line.find('{') != std::string::npos &&
          line.find('}') == std::string::npos) {
        continue;
      }
      // Write codeBuffer to temp file
      std::string tempFile = "repl_input.tspp";
      std::ofstream out(tempFile);
      out << codeBuffer;
      out.close();
      // Compile to LLVM IR
      std::string irFile = tempFile + ".ll";
      codegen::LLVMCodeGenerator codegen;
      lexer::Lexer lexer(codeBuffer);
      auto tokens = lexer.tokenize();
      auto ast = parser::buildAST(tokens);
      if (ast) {
        codegen.generate(ast.get(), irFile);
        // Remove target triple
        std::string sedCmd =
            "sed -i '/^[[:space:]]*target triple[[:space:]]*=/Id' " + irFile;
        std::system(sedCmd.c_str());
        // Assemble
        std::string asCmd = "llvm-as " + irFile + " -o temp.bc";
        std::system(asCmd.c_str());
        // Link and build
        std::string stdlib = "./build/src/packages/tspp_std/libtspp_std.a";
        std::string clangCmd = "clang " + irFile + " " + stdlib +
                               " -o temp_exec -lstdc++ -lgc -fsanitize=address "
                               "-no-pie -Wno-override-module";
        std::system(clangCmd.c_str());
        // Run and print output
        std::string runCmd = "./temp_exec";
        std::cout << "Output: " << std::endl;
        std::system(runCmd.c_str());
      } else {
        std::cerr << "Parse error\n";
      }
      codeBuffer.clear();
    }
    return 0;
  }
  // File execution mode
  std::string inputFile = argv[1];
  std::ifstream file(inputFile);
  if (!file) {
    std::cerr << "Could not open file: " << inputFile << std::endl;
    return 1;
  }
  std::string source((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
  // Compile to LLVM IR
  std::string irFile = inputFile + ".ll";
  lexer::Lexer lexer(source);
  auto tokens = lexer.tokenize();
  auto ast = parser::buildAST(tokens);
  if (ast) {
    codegen::LLVMCodeGenerator codegen;
    codegen.generate(ast.get(), irFile);
    // Remove target triple
    std::string sedCmd =
        "sed -i '/^[[:space:]]*target triple[[:space:]]*=/Id' " + irFile;
    std::system(sedCmd.c_str());
    // Assemble
    std::string asCmd = "llvm-as " + irFile + " -o temp.bc";
    std::system(asCmd.c_str());
    // Link and build
    std::string stdlib = "./build/src/packages/tspp_std/libtspp_std.a";
    std::string clangCmd = "clang " + irFile + " " + stdlib +
                           " -o temp_exec -lstdc++ -lgc -fsanitize=address "
                           "-no-pie -Wno-override-module";
    std::system(clangCmd.c_str());
    // Run and print output
    std::string runCmd = "./temp_exec";
    std::cout << "Output: " << std::endl;
    std::system(runCmd.c_str());
  } else {
    std::cerr << "Parse error\n";
    return 1;
  }
  return 0;
}
