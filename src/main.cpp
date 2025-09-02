#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <sys/wait.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "codegen/LLVMCodeGenerator.h"
#include "interpreter/interpreter.h"
#include "lexer/lexer.h"
#include "parser/module/ModuleResolver.h"
#include "parser/parser.h"
#include "parser/visitors/semantic/SemanticAnalyzerVisitor.h"

static bool hasFlag(int argc, char* argv[], const char* flag) {
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == flag) return true;
  }
  return false;
}

static std::vector<const char*> filterArgs(
    int argc, char* argv[], const std::vector<std::string>& toStrip) {
  std::vector<const char*> out;
  for (int i = 0; i < argc; ++i) {
    bool skip = false;
    for (auto& s : toStrip) {
      if (std::string(argv[i]) == s) {
        skip = true;
        break;
      }
    }
    if (!skip) out.push_back(argv[i]);
  }
  return out;
}

int main(int argc, char* argv[]) {
  llvm::llvm_shutdown_obj shutdown_on_exit;
  // Simple CLI flags
  bool forceJIT =
      hasFlag(argc, argv, "--jit") || hasFlag(argc, argv, "--native");
  bool noJIT = hasFlag(argc, argv, "--no-jit");

  // Compute positional args (strip our flags)
  auto args = filterArgs(argc, argv, {"--jit", "--native", "--no-jit"});
  int pargc = static_cast<int>(args.size());

  if (pargc < 2) {
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
      // In-memory compile and JIT
      codegen::LLVMCodeGenerator codegen;
      lexer::Lexer lexer(codeBuffer);
      auto tokens = lexer.tokenize();
      auto ast = parser::buildAST(tokens);
      if (ast) {
        // Prefer in-process JIT for REPL snippets
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();
        auto jitExpected = llvm::orc::LLJITBuilder().create();
        if (!jitExpected) {
          llvm::errs() << "Failed to create LLJIT: "
                       << llvm::toString(jitExpected.takeError()) << "\n";
        } else {
          auto jit = std::move(*jitExpected);
          // Make host process symbols (including statically linked stdlib) visible
          if (auto gen = llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
                  jit->getDataLayout().getGlobalPrefix())) {
            jit->getMainJITDylib().addGenerator(std::move(*gen));
          }
          // Generate module in-memory
          codegen.generate(ast.get());
          std::unique_ptr<llvm::Module> M = codegen.takeModule();
          // Serialize to textual IR and re-parse into dedicated context for ORC
          std::string ir;
          llvm::raw_string_ostream irOS(ir);
          M->print(irOS, nullptr);
          irOS.flush();
          auto TSCtx = std::make_unique<llvm::LLVMContext>();
          llvm::SMDiagnostic diag;
          auto memBuf = llvm::MemoryBuffer::getMemBuffer(ir);
          std::unique_ptr<llvm::Module> parsedM =
              llvm::parseIR(memBuf->getMemBufferRef(), diag, *TSCtx);
          if (!parsedM) {
            llvm::errs() << "Failed to parse in-memory IR for REPL: "
                         << diag.getMessage() << "\n";
            continue;  // skip this snippet and prompt again
          }
          M = std::move(parsedM);
          // Set DL and optimize O3
          M->setTargetTriple("");
          M->setDataLayout(jit->getDataLayout());
          llvm::PassBuilder pb;
          llvm::LoopAnalysisManager lam;
          llvm::FunctionAnalysisManager fam;
          llvm::CGSCCAnalysisManager cam;
          llvm::ModuleAnalysisManager mam;
          pb.registerModuleAnalyses(mam);
          pb.registerCGSCCAnalyses(cam);
          pb.registerFunctionAnalyses(fam);
          pb.registerLoopAnalyses(lam);
          pb.crossRegisterProxies(lam, fam, cam, mam);
          llvm::ModulePassManager mpm =
              pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
          mpm.run(*M, mam);
          auto tsm =
              llvm::orc::ThreadSafeModule(std::move(M), std::move(TSCtx));
          if (auto err = jit->addIRModule(std::move(tsm))) {
            llvm::errs() << "JIT add module error: "
                         << llvm::toString(std::move(err)) << "\n";
          } else {
            // run main() if present (void or i32)
            if (auto sym = jit->lookup("main")) {
              // We can only safely call no-arg variants in REPL
              using IntMain = int (*)();
              using VoidMain = void (*)();
              // Try int() then void()
              IntMain ifn = sym->toPtr<IntMain>();
              int result = ifn ? ifn() : 0;
              if (!ifn) {
                if (auto vfn = sym->toPtr<VoidMain>()) vfn();
              }
              std::cout << "Output (exit code): " << result << "\n";
            }
          }
        }
      } else {
        std::cerr << "Parse error\n";
      }
      codeBuffer.clear();
    }
    return 0;
  }
  // File execution mode
  std::string inputFile = args[1];
  std::ifstream file(inputFile);
  if (!file) {
    std::cerr << "Could not open file: " << inputFile << std::endl;
    return 1;
  }
  std::string source((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
  // Compile to LLVM IR (in-memory)
  lexer::Lexer lexer(source);
  auto tokens = lexer.tokenize();
  auto ast = parser::buildAST(tokens);
  if (ast) {
    // Decide path: JIT fast path by default unless explicitly disabled
    bool useJIT = forceJIT || !noJIT;  // default ON unless --no-jit
    if (useJIT) {
      // Initialize JIT
      llvm::InitializeNativeTarget();
      llvm::InitializeNativeTargetAsmPrinter();
      llvm::InitializeNativeTargetAsmParser();
      auto jitExpected = llvm::orc::LLJITBuilder().create();
      if (!jitExpected) {
        llvm::errs() << "Failed to create LLJIT: "
                     << llvm::toString(jitExpected.takeError()) << "\n";
      } else {
        auto jit = std::move(*jitExpected);
        // Ensure host process symbols are visible prior to materialization
        auto genOrErr = llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            jit->getDataLayout().getGlobalPrefix());
        if (genOrErr) {
          jit->getMainJITDylib().addGenerator(std::move(*genOrErr));
        }
        // Build module in-memory and re-parse via textual IR to a fresh context
        codegen::LLVMCodeGenerator codegen;
        codegen.generate(ast.get());
        std::unique_ptr<llvm::Module> M = codegen.takeModule();
        auto TSCtx = std::make_unique<llvm::LLVMContext>();
        std::string ir;
        llvm::raw_string_ostream irOS(ir);
        M->print(irOS, nullptr);
        irOS.flush();
        llvm::SMDiagnostic diag;
        auto memBuf = llvm::MemoryBuffer::getMemBuffer(ir);
        std::unique_ptr<llvm::Module> parsedM =
            llvm::parseIR(memBuf->getMemBufferRef(), diag, *TSCtx);
        if (!parsedM) {
          llvm::errs() << "Failed to parse in-memory IR: "
                       << diag.getMessage() << "\n";
        } else {
          M = std::move(parsedM);
          // Detect main() signature and set DL
          bool hasMain = false;
          bool mainIsInt0 = false;
          bool mainIsVoid0 = false;
          bool mainIsIntArgv = false;
          if (M) {
            if (auto* F = M->getFunction("main")) {
              hasMain = true;
              auto* FTy = F->getFunctionType();
              auto* ret = FTy->getReturnType();
              unsigned n = FTy->getNumParams();
              mainIsInt0 = ret->isIntegerTy(32) && n == 0;
              mainIsVoid0 = ret->isVoidTy() && n == 0;
              if (ret->isIntegerTy(32) && n == 2) {
                auto* p0 = FTy->getParamType(0);
                auto* p1 = FTy->getParamType(1);
                bool p0ok = p0->isIntegerTy(32);
                bool p1ok = p1->isPointerTy();  // accept any pointer (opaque)
                mainIsIntArgv = p0ok && p1ok;
              }
            }
          }
          M->setTargetTriple("");
          M->setDataLayout(jit->getDataLayout());
          // Optimize O3
          llvm::PassBuilder pb;
          llvm::LoopAnalysisManager lam;
          llvm::FunctionAnalysisManager fam;
          llvm::CGSCCAnalysisManager cam;
          llvm::ModuleAnalysisManager mam;
          pb.registerModuleAnalyses(mam);
          pb.registerCGSCCAnalyses(cam);
          pb.registerFunctionAnalyses(fam);
          pb.registerLoopAnalyses(lam);
          pb.crossRegisterProxies(lam, fam, cam, mam);
          llvm::ModulePassManager mpm =
              pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
          mpm.run(*M, mam);
          auto tsm =
              llvm::orc::ThreadSafeModule(std::move(M), std::move(TSCtx));
          if (auto err = jit->addIRModule(std::move(tsm))) {
            llvm::errs() << "JIT add module error: "
                         << llvm::toString(std::move(err)) << "\n";
          } else {
            // Lookup and call main directly for supported signatures
            if (hasMain) {
              if (mainIsInt0) {
                if (auto sym = jit->lookup("main")) {
                  using IntMain = int (*)();
                  IntMain fn = sym->toPtr<IntMain>();
                  return fn();
                }
              } else if (mainIsVoid0) {
                if (auto sym = jit->lookup("main")) {
                  using VoidMain = void (*)();
                  VoidMain fn = sym->toPtr<VoidMain>();
                  fn();
                  return 0;
                }
              } else if (mainIsIntArgv) {
                if (auto sym = jit->lookup("main")) {
                  using IntArgvMain = int (*)(int, char**);
                  IntArgvMain fn = sym->toPtr<IntArgvMain>();
                  int cargc = pargc;
                  // Build a temporary argv (drop program name to mimic C
                  // argv[0]=inputFile)
                  std::vector<std::string> argStorage;
                  std::vector<char*> argvVec;
                  argStorage.reserve(pargc);
                  argvVec.reserve(pargc + 1);
                  for (int i = 0; i < pargc; ++i) {
                    argStorage.emplace_back(args[i]);
                  }
                  for (int i = 0; i < pargc; ++i) {
                    argvVec.push_back(argStorage[i].data());
                  }
                  argvVec.push_back(nullptr);
                  return fn(cargc, argvVec.data());
                }
              }
            }
          }
        }
      }
      // If we couldn't JIT, fall through to AOT path
    }
    // AOT fallback: current pipeline using clang (debug-only)
    codegen::LLVMCodeGenerator codegen;
    std::string irFile = inputFile + ".ll";
    codegen.generate(ast.get(), irFile);
    std::string sedCmd =
        "sed -i '/^[[:space:]]*target triple[[:space:]]*=/Id' " + irFile;
    std::system(sedCmd.c_str());
    std::string asCmd = "llvm-as " + irFile + " -o temp.bc";
    std::system(asCmd.c_str());
    std::string stdlib = "./src/packages/tspp_std/libtspp_std.a";
    std::string clangCmd =
        "clang " + irFile + " " + stdlib +
        " -O3 -o temp_exec -lstdc++ -lgc -no-pie -Wno-override-module";
    std::system(clangCmd.c_str());
    std::string runCmd = "./temp_exec";
    int status = std::system(runCmd.c_str());
#ifdef WEXITSTATUS
    return WEXITSTATUS(status);
#else
    return (status >> 8) & 0xFF;
#endif
  } else {
    std::cerr << "Parse error\n";
    return 1;
  }
  return 0;
}
