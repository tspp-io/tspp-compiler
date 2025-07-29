#include <fstream>
#include <iostream>
#include <string>

#include "core/utils/log_utils.h"
#include "lexer/lexer.h"
// #include "parser/parser.h"
// #include "interpreter/interpreter.h"

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
  lexer::Lexer lexer(source);
  auto tokens = lexer.tokenize();
  // TODO: Parse tokens and interpret
  std::cout << "Tokenized " << tokens.size() << " tokens." << std::endl;
  for (const auto& token : tokens) {
    printToken(token);
  }
  return 0;
}
