#include <llvm/Support/ManagedStatic.h>

#include <fstream>
#include <iostream>
#include <string>

#include "codegen/LLVMCodeGenerator.h"
#include "lexer/lexer.h"
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
  std::cerr << "[PHASE] Lexing..." << std::endl;
  lexer::Lexer lexer(source);
  auto tokens = lexer.tokenize();
  (void)tokens;  // suppress unused warning if not inspected further
  std::cerr << "[PHASE] Lexing done." << std::endl;

  // Parse into AST
  std::cerr << "[PHASE] Parsing..." << std::endl;
  auto ast = parser::buildAST(tokens);
  std::cerr << "[PHASE] Parsing done." << std::endl;
  if (ast) {
    // === Semantic Analysis ===
    std::cerr << "[PHASE] Semantic analysis..." << std::endl;
    ast::SemanticAnalyzerVisitor semanticAnalyzer;
    ast->accept(semanticAnalyzer);
    std::cerr << "[PHASE] Semantic analysis done." << std::endl;

    const auto& errors = semanticAnalyzer.getErrors();
    if (!errors.empty()) {
      std::cerr << "Semantic analysis errors found:" << std::endl;
      for (const auto& error : errors) {
        std::cerr << "  Error: " << error << std::endl;
      }
      // Continue with codegen even if there are semantic errors (for now)
    }

    // === LLVM Code Generation ===
    {
      std::cerr << "[PHASE] Codegen..." << std::endl;
      codegen::LLVMCodeGenerator codegen(&semanticAnalyzer);
      // Write IR to file: <input>.ll
      std::string outFile = std::string(argv[1]) + ".ll";
      codegen.generate(ast.get(), outFile);
      std::cerr << "[PHASE] Codegen done." << std::endl;
    }  // codegen and its module destroyed here before shutdown
  } else {
    std::cerr << "Failed to build AST." << std::endl;
    return 1;
  }

  return 0;
}
