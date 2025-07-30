#include <fstream>
#include <iostream>
#include <string>

#include "ast/ASTPrinter.h"
#include "core/utils/log_utils.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

int main(int argc, char* argv[]) {
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
  std::cout << "Tokenized " << tokens.size() << " tokens." << std::endl;

  // printing tokens
  for (const auto& token : tokens) {
    printToken(token);
  }
  // Parse into AST
  auto ast = parser::buildAST(tokens);
  if (ast) {
    std::cout << "Successfully built AST!" << std::endl;

    // Print AST
    ast::ASTPrinter printer;
    std::cout << "AST Structure:" << std::endl;
    ast->accept(printer);
    std::cout << std::endl;
  } else {
    std::cout << "Failed to build AST." << std::endl;
    return 1;
  }

  return 0;
}
