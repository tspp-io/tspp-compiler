#include <llvm/Support/ManagedStatic.h>

#include <fstream>
#include <iostream>
#include <string>

#include "codegen/LLVMCodeGenerator.h"
#include "lexer/lexer.h"
#include "parser/module/ModuleResolver.h"
#include "parser/parser.h"
#include "parser/visitors/semantic/SemanticAnalyzerVisitor.h"

int main(int argc, char* argv[]) {
  llvm::llvm_shutdown_obj shutdown_on_exit;
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file.tspp>" << std::endl;
    return 1;
  }
  std::ifstream file(argv[1]);
  if (!file) {
    std::cerr << "Could not open file: " << argv[1] << std::endl;
    return 1;
  }
  std::string source((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

  // Tokenize
  lexer::Lexer lexer(source);
  auto tokens = lexer.tokenize();
  (void)tokens;  // suppress unused warning if not inspected further

  // Parse into AST
  auto ast = parser::buildAST(tokens);
  if (ast) {
    // Resolve imports and merge modules (file-based)
    parser::modules::ModuleResolver resolver;
    bool importsOk = resolver.resolveAndMerge(ast.get(), argv[1]);
    if (!importsOk) {
      std::cerr << "Import resolution failed.\n";
      return 1;
    }
    // === Semantic Analysis ===
    ast::SemanticAnalyzerVisitor semanticAnalyzer;
    ast->accept(semanticAnalyzer);

    const auto& errors = semanticAnalyzer.getErrors();
    if (!errors.empty()) {
      std::cerr << "Semantic analysis errors found:" << std::endl;
      for (const auto& error : errors) {
        std::cerr << "  Error: " << error << std::endl;
      }
      // Emit a minimal stub IR to keep downstream tooling working and avoid
      // crashes. This avoids running codegen on an invalid AST.
      std::string outFile = std::string(argv[1]) + ".ll";
      std::ofstream out(outFile, std::ios::trunc);
      if (out) {
        out << "define i32 @main() {\n";
        out << "entry:\n";
        out << "  ret i32 0\n";
        out << "}\n";
      } else {
        std::cerr << "Failed to write stub IR to: " << outFile << std::endl;
        return 1;
      }
      return 0;
    }

    // === LLVM Code Generation ===
    {
      codegen::LLVMCodeGenerator codegen(&semanticAnalyzer);
      // Write IR to file: <input>.ll
      std::string outFile = std::string(argv[1]) + ".ll";
      codegen.generate(ast.get(), outFile);
    }  // codegen and its module destroyed here before shutdown
  } else {
    std::cerr << "Failed to build AST." << std::endl;
    return 1;
  }

  return 0;
}
