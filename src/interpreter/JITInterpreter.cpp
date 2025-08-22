#include <llvm/Support/TargetSelect.h>

#include <iostream>

#include "interpreter.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

namespace interpreter {

JITInterpreter::JITInterpreter() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
  auto jitExpected = llvm::orc::LLJITBuilder().create();
  if (!jitExpected) {
    llvm::errs() << "Failed to create LLJIT: "
                 << llvm::toString(jitExpected.takeError()) << "\n";
    jit_ = nullptr;
  } else {
    jit_ = std::move(*jitExpected);
  }
  codegen_ = std::make_unique<codegen::LLVMCodeGenerator>();
  registerBuiltins();
}

JITInterpreter::~JITInterpreter() {}

bool JITInterpreter::executeLine(const std::string& code) {
  // Tokenize and parse
  lexer::Lexer lexer(code);
  auto tokens = lexer.tokenize();
  auto ast = parser::buildAST(tokens);
  if (!ast) {
    std::cerr << "Parse error\n";
    return false;
  }
  // Semantic analysis
  ast::SemanticAnalyzerVisitor semanticAnalyzer;
  ast->accept(semanticAnalyzer);
  if (!semanticAnalyzer.getErrors().empty()) {
    std::cerr << "Semantic error\n";
    return false;
  }
  // Codegen for REPL
  auto module = codegen_->generateModuleForREPL(ast.get(), globals_);
  if (!module) {
    std::cerr << "Codegen error\n";
    return false;
  }
  // Add module to JIT
  if (auto err = jit_->addIRModule(llvm::orc::ThreadSafeModule(
          std::move(module), std::make_unique<llvm::LLVMContext>()))) {
    std::cerr << "JIT error\n";
    return false;
  }
  // Optionally run main or top-level expr
  return true;
}

bool JITInterpreter::initializeJIT() {
  // Already handled in constructor
  return true;
}

void JITInterpreter::registerBuiltins() {
  // Register built-in functions for REPL
}

}  // namespace interpreter
