// tspp driver: source -> LLVM IR -> (llvm-as) -> clang link -> run.
//
// Child processes are started with llvm::sys::ExecuteAndWait and an argv
// vector; nothing goes through a shell, so file names are always literal.
// Intermediates live in a private temp directory, the runtime (libcore.a) is
// located relative to this executable, and every stage's exit status is
// checked.

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "codegen/LLVMCodeGenerator.h"
#include "lexer/lexer.h"
#include "parser/module/ModuleResolver.h"
#include "parser/parser.h"
#include "parser/visitors/semantic/SemanticAnalyzerVisitor.h"

#ifndef TSPP_VERSION
#define TSPP_VERSION "unknown"
#endif
#ifndef TSPP_LLVM_TOOLS_DIR
#define TSPP_LLVM_TOOLS_DIR ""
#endif

namespace {

constexpr int kExitFailure = 1;
constexpr int kExitUsage = 2;

struct Options {
  std::string input;                     // empty: REPL
  std::string output;                    // -o
  bool emitLLVM = false;                 // --emit-llvm
  bool keepTemps = false;                // --keep-temps
  bool asan = false;                     // --asan
  std::vector<std::string> programArgs;  // after "--"
};

void printUsage(llvm::raw_ostream& os) {
  os << "Usage: tspp [options] <file.tspp> [-- program-args...]\n"
        "       tspp                 start the REPL\n"
        "\n"
        "Without -o or --emit-llvm, the program is compiled, linked and run;\n"
        "tspp exits with the program's exit status.\n"
        "\n"
        "Options:\n"
        "  -o <file>      write the executable (or, with --emit-llvm, the IR)\n"
        "                 to <file> and do not run it\n"
        "  --emit-llvm    write LLVM IR and stop (default: <file.tspp>.ll)\n"
        "  --keep-temps   keep the temporary build directory and print its "
        "path\n"
        "  --asan         link the program with AddressSanitizer\n"
        "  --version      print the version and exit\n"
        "  -h, --help     print this help and exit\n"
        "\n"
        "Environment:\n"
        "  TSPP_CLANG        clang used to link programs (default: clang on "
        "PATH)\n"
        "  TSPP_LLVM_AS      llvm-as used to check the IR (default: llvm-as on "
        "PATH)\n"
        "  TSPP_RUNTIME_LIB  runtime archive (default: libcore.a next to "
        "tspp)\n";
}

// Returns 0 on success, otherwise the exit code to use (usage error).
// Sets `exitNow` when the process should exit successfully right away.
int parseArgs(int argc, char* argv[], Options& opts, bool& exitNow) {
  exitNow = false;
  for (int i = 1; i < argc; ++i) {
    llvm::StringRef arg(argv[i]);
    if (arg == "--") {
      for (++i; i < argc; ++i) opts.programArgs.push_back(argv[i]);
      break;
    }
    if (arg == "-h" || arg == "--help") {
      printUsage(llvm::outs());
      exitNow = true;
      return 0;
    }
    if (arg == "--version") {
      llvm::outs() << "tspp " << TSPP_VERSION << "\n";
      exitNow = true;
      return 0;
    }
    if (arg == "-o") {
      if (i + 1 >= argc) {
        llvm::errs() << "tspp: error: -o requires an argument\n";
        return kExitUsage;
      }
      opts.output = argv[++i];
    } else if (arg.size() > 2 && arg.substr(0, 2) == "-o") {
      opts.output = arg.drop_front(2).str();
    } else if (arg == "--emit-llvm") {
      opts.emitLLVM = true;
    } else if (arg == "--keep-temps") {
      opts.keepTemps = true;
    } else if (arg == "--asan") {
      opts.asan = true;
    } else if (arg.size() > 1 && arg.front() == '-') {
      llvm::errs() << "tspp: error: unknown option '" << arg << "'\n";
      printUsage(llvm::errs());
      return kExitUsage;
    } else if (opts.input.empty()) {
      opts.input = arg.str();
    } else {
      llvm::errs() << "tspp: error: more than one input file ('" << opts.input
                   << "', '" << arg << "'); pass program arguments after --\n";
      return kExitUsage;
    }
  }
  if (opts.input.empty() &&
      (!opts.output.empty() || opts.emitLLVM || !opts.programArgs.empty())) {
    llvm::errs() << "tspp: error: no input file\n";
    return kExitUsage;
  }
  return 0;
}

// Anchor for getMainExecutable (taking the address of main is ill-formed).
void exeAnchor() {}

std::string mainExecutablePath(const char* argv0) {
  return llvm::sys::fs::getMainExecutable(argv0,
                                          reinterpret_cast<void*>(&exeAnchor));
}

// Finds a tool: $envVar if set, else `name` on PATH, else in the LLVM tools
// directory this compiler was built against.
std::optional<std::string> findTool(const char* envVar, llvm::StringRef name) {
  if (std::optional<std::string> env = llvm::sys::Process::GetEnv(envVar)) {
    if (!env->empty()) {
      // A bare name is looked up on PATH; a path is used as given.
      if (env->find('/') == std::string::npos) {
        if (auto p = llvm::sys::findProgramByName(*env)) return *p;
        llvm::errs() << "tspp: error: " << envVar << "='" << *env
                     << "' not found on PATH\n";
        return std::nullopt;
      }
      return *env;
    }
  }
  if (auto p = llvm::sys::findProgramByName(name)) return *p;
  llvm::StringRef toolsDir(TSPP_LLVM_TOOLS_DIR);
  if (!toolsDir.empty()) {
    if (auto p = llvm::sys::findProgramByName(name, {toolsDir})) return *p;
  }
  llvm::errs() << "tspp: error: '" << name << "' not found on PATH (set "
               << envVar << ")\n";
  return std::nullopt;
}

// $TSPP_RUNTIME_LIB, else libcore.a next to the tspp executable (build tree:
// build/src/{tspp,libcore.a}), else <prefix>/lib/libcore.a (installed).
std::optional<std::string> findRuntimeLib(const std::string& exePath) {
  if (std::optional<std::string> env =
          llvm::sys::Process::GetEnv("TSPP_RUNTIME_LIB")) {
    if (!env->empty()) {
      if (llvm::sys::fs::exists(*env)) return *env;
      llvm::errs() << "tspp: error: TSPP_RUNTIME_LIB='" << *env
                   << "' does not exist\n";
      return std::nullopt;
    }
  }
  llvm::SmallString<256> exeDir(exePath);
  llvm::sys::path::remove_filename(exeDir);
  llvm::SmallString<256> candidates[2];
  candidates[0] = exeDir;
  llvm::sys::path::append(candidates[0], "libcore.a");
  candidates[1] = exeDir;
  llvm::sys::path::append(candidates[1], "..", "lib", "libcore.a");
  for (auto& c : candidates) {
    if (llvm::sys::fs::exists(c)) return std::string(c.str());
  }
  llvm::errs() << "tspp: error: runtime library libcore.a not found next to "
               << exePath << " (set TSPP_RUNTIME_LIB)\n";
  return std::nullopt;
}

// Runs a child process without a shell. Returns its exit status, or -1 if it
// could not be started or was killed by a signal (`errMsg` says which).
int runProcess(const std::string& program, const std::vector<std::string>& args,
               std::string& errMsg) {
  std::vector<llvm::StringRef> argv;
  argv.reserve(args.size());
  for (const auto& a : args) argv.push_back(a);
  bool execFailed = false;
  errMsg.clear();
  int rc = llvm::sys::ExecuteAndWait(program, argv, /*Env=*/std::nullopt,
                                     /*Redirects=*/{}, /*SecondsToWait=*/0,
                                     /*MemoryLimit=*/0, &errMsg, &execFailed);
  if (execFailed) return -1;
  if (rc < 0 && errMsg.empty()) errMsg = "terminated abnormally";
  return rc < 0 ? -1 : rc;
}

// Runs a build stage; prints which stage failed. Returns true on success.
bool runStage(const char* stage, const std::string& program,
              const std::vector<std::string>& args) {
  std::string errMsg;
  int rc = runProcess(program, args, errMsg);
  if (rc == 0) return true;
  llvm::errs() << "tspp: error: " << stage << " failed (" << program;
  if (rc > 0)
    llvm::errs() << " exited with status " << rc;
  else
    llvm::errs() << ": " << errMsg;
  llvm::errs() << ")\n";
  return false;
}

// Front end: source -> LLVM IR file. `inputPath` is used to resolve imports;
// empty for the REPL (no import resolution). Returns true on success.
bool compileToIR(const std::string& source, const std::string& inputPath,
                 const std::string& irPath) {
  lexer::Lexer lexer(source);
  auto tokens = lexer.tokenize();
  auto ast = parser::buildAST(tokens);
  if (!ast) {
    llvm::errs() << "Parse error\n";
    return false;
  }
  if (!inputPath.empty()) {
    parser::modules::ModuleResolver resolver;
    if (!resolver.resolveAndMerge(ast.get(), inputPath)) {
      llvm::errs() << "Import resolution failed\n";
      return false;
    }
  }
  // Semantic analysis gives codegen resolved types.
  ast::SemanticAnalyzerVisitor semanticAnalyzer;
  ast->accept(semanticAnalyzer);
  if (!semanticAnalyzer.getErrors().empty()) {
    llvm::errs() << "Semantic error(s):\n";
    for (auto& e : semanticAnalyzer.getErrors()) llvm::errs() << e << "\n";
    return false;
  }
  // Remove a stale file first so a failed write cannot be mistaken for output.
  llvm::sys::fs::remove(irPath);
  codegen::LLVMCodeGenerator codegen(&semanticAnalyzer);
  codegen.generate(ast.get(), irPath);
  if (!llvm::sys::fs::exists(irPath)) {
    llvm::errs() << "tspp: error: could not write LLVM IR to " << irPath
                 << "\n";
    return false;
  }
  return true;
}

// Unique temp directory, removed on destruction unless kept.
class TempDir {
 public:
  explicit TempDir(bool keep) : keep_(keep) {
    llvm::SmallString<128> path;
    if (std::error_code ec =
            llvm::sys::fs::createUniqueDirectory("tspp", path)) {
      llvm::errs() << "tspp: error: cannot create temp directory: "
                   << ec.message() << "\n";
      return;
    }
    path_ = std::string(path.str());
  }
  ~TempDir() {
    if (path_.empty()) return;
    if (keep_) {
      llvm::errs() << "tspp: kept temporary files in " << path_ << "\n";
    } else {
      llvm::sys::fs::remove_directories(path_);
    }
  }
  TempDir(const TempDir&) = delete;
  TempDir& operator=(const TempDir&) = delete;
  bool ok() const {
    return !path_.empty();
  }
  std::string file(llvm::StringRef name) const {
    llvm::SmallString<256> p(path_);
    llvm::sys::path::append(p, name);
    return std::string(p.str());
  }

 private:
  std::string path_;
  bool keep_;
};

struct Toolchain {
  std::string clang;
  std::string llvmAs;
  std::string runtimeLib;
};

std::optional<Toolchain> findToolchain(const std::string& exePath) {
  auto clang = findTool("TSPP_CLANG", "clang");
  auto llvmAs = findTool("TSPP_LLVM_AS", "llvm-as");
  auto runtime = findRuntimeLib(exePath);
  if (!clang || !llvmAs || !runtime) return std::nullopt;
  return Toolchain{*clang, *llvmAs, *runtime};
}

// IR -> verified bitcode -> linked executable.
bool buildExecutable(const Toolchain& tc, const Options& opts,
                     const TempDir& tmp, const std::string& irPath,
                     const std::string& exePath) {
  std::string bcPath = tmp.file("program.bc");
  if (!runStage("IR verification (llvm-as)", tc.llvmAs,
                {tc.llvmAs, irPath, "-o", bcPath}))
    return false;
  std::vector<std::string> link = {tc.clang,
                                   bcPath,
                                   tc.runtimeLib,
                                   "-O3",
                                   "-o",
                                   exePath,
                                   "-lcurl",
                                   "-lstdc++",
                                   "-lgc",
                                   "-pthread",
                                   "-Wno-override-module"};
  if (opts.asan) link.push_back("-fsanitize=address");
  return runStage("link (clang)", tc.clang, link);
}

// Runs the built program; returns its exit status (or 1 if it crashed).
int runProgram(const std::string& exePath,
               const std::vector<std::string>& programArgs) {
  std::vector<std::string> args = {exePath};
  args.insert(args.end(), programArgs.begin(), programArgs.end());
  llvm::outs().flush();
  std::cout.flush();
  std::string errMsg;
  int rc = runProcess(exePath, args, errMsg);
  if (rc < 0) {
    llvm::errs() << "tspp: error: program " << errMsg << "\n";
    return kExitFailure;
  }
  return rc;
}

int compileFile(const Options& opts, const std::string& exeSelf) {
  std::ifstream file(opts.input, std::ios::binary);
  if (!file) {
    llvm::errs() << "tspp: error: could not open file: " << opts.input << "\n";
    return kExitFailure;
  }
  std::string source((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

  if (opts.emitLLVM) {
    std::string irOut = opts.output.empty() ? opts.input + ".ll" : opts.output;
    return compileToIR(source, opts.input, irOut) ? 0 : kExitFailure;
  }

  auto tc = findToolchain(exeSelf);
  if (!tc) return kExitFailure;
  TempDir tmp(opts.keepTemps);
  if (!tmp.ok()) return kExitFailure;
  std::string irPath = tmp.file("program.ll");
  if (!compileToIR(source, opts.input, irPath)) return kExitFailure;

  if (!opts.output.empty()) {
    return buildExecutable(*tc, opts, tmp, irPath, opts.output) ? 0
                                                                : kExitFailure;
  }
  std::string exePath = tmp.file("program");
  if (!buildExecutable(*tc, opts, tmp, irPath, exePath)) return kExitFailure;
  return runProgram(exePath, opts.programArgs);
}

// REPL: each complete chunk is compiled as a fresh program and run.
int runRepl(const Options& opts, const std::string& exeSelf) {
  auto tc = findToolchain(exeSelf);
  if (!tc) return kExitFailure;
  std::cout << "tspp REPL (each input is compiled and run as a new program)\n";
  std::string line, codeBuffer;
  while (true) {
    std::cout << ">>> " << std::flush;
    if (!std::getline(std::cin, line)) break;
    if (line.empty()) continue;
    codeBuffer += line + "\n";
    if (line.find('{') != std::string::npos &&
        line.find('}') == std::string::npos) {
      continue;
    }
    TempDir tmp(opts.keepTemps);
    if (tmp.ok()) {
      std::string irPath = tmp.file("program.ll");
      std::string exePath = tmp.file("program");
      if (compileToIR(codeBuffer, "", irPath) &&
          buildExecutable(*tc, opts, tmp, irPath, exePath)) {
        int rc = runProgram(exePath, {});
        if (rc != 0) std::cout << "[exit status " << rc << "]\n";
      }
    }
    codeBuffer.clear();
  }
  return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
  llvm::llvm_shutdown_obj shutdown_on_exit;
  Options opts;
  bool exitNow = false;
  if (int rc = parseArgs(argc, argv, opts, exitNow)) return rc;
  if (exitNow) return 0;

  std::string exeSelf = mainExecutablePath(argv[0]);
  if (opts.input.empty()) return runRepl(opts, exeSelf);
  return compileFile(opts, exeSelf);
}
