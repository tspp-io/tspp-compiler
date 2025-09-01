// main.cpp
#include <llvm/Support/ManagedStatic.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
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

// ----------------------------- Utilities -----------------------------

// Simple helper: check if file exists
static bool file_exists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0;
}

// Return true if 'a' is newer than 'b' (or b missing -> true)
static bool file_a_newer_than_b(const std::string &a, const std::string &b) {
  struct stat sa, sb;
  if (stat(a.c_str(), &sa) != 0) return false;
  if (stat(b.c_str(), &sb) != 0)
    return true;  // b missing -> a considered newer
  return sa.st_mtime > sb.st_mtime;
}

// compare files quickly (size then chunk-compare)
static bool files_are_equal(const std::string &a, const std::string &b) {
  struct stat sa, sb;
  if (stat(a.c_str(), &sa) != 0 || stat(b.c_str(), &sb) != 0) return false;
  if (sa.st_size != sb.st_size) return false;
  static const size_t BUF = 1 << 16;
  std::ifstream fa(a, std::ios::binary);
  std::ifstream fb(b, std::ios::binary);
  if (!fa || !fb) return false;
  std::vector<char> ba(BUF), bb(BUF);
  while (fa && fb) {
    fa.read(ba.data(), BUF);
    fb.read(bb.data(), BUF);
    std::streamsize ra = fa.gcount(), rb = fb.gcount();
    if (ra != rb) return false;
    if (ra == 0) break;
    if (std::memcmp(ba.data(), bb.data(), static_cast<size_t>(ra)) != 0)
      return false;
  }
  return true;
}

// Run a command using fork+execvp (args vector). Returns exit code (or -1 on
// error).
static int run_execvp(const std::vector<std::string> &args,
                      double *elapsed_seconds = nullptr) {
  if (args.empty()) return -1;
  std::vector<char *> cargs;
  cargs.reserve(args.size() + 1);
  for (const auto &s : args) cargs.push_back(const_cast<char *>(s.c_str()));
  cargs.push_back(nullptr);

  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return -1;
  }
  if (pid == 0) {
    // Child
    execvp(cargs[0], cargs.data());
    // If execvp returns, there was an error
    perror("execvp");
    _exit(127);
  } else {
    // Parent: wait and optionally time
    int status = 0;
    auto t0 = std::chrono::steady_clock::now();
    if (waitpid(pid, &status, 0) == -1) {
      perror("waitpid");
      return -1;
    }
    auto t1 = std::chrono::steady_clock::now();
    if (elapsed_seconds) {
      *elapsed_seconds = std::chrono::duration<double>(t1 - t0).count();
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;
  }
}

// Remove "target triple = ..." lines from IR in a portable way (re-writes file)
static bool remove_target_triple_line(const std::string &path) {
  std::ifstream in(path);
  if (!in) return false;
  std::string tmp = path + ".tmp";
  std::ofstream out(tmp);
  if (!out) return false;
  std::string line;
  while (std::getline(in, line)) {
    // case-insensitive check for "target triple"
    std::string lower = line;
    for (char &c : lower) c = static_cast<char>(std::tolower((unsigned char)c));
    if (lower.find("target triple") != std::string::npos) {
      // skip this line
      continue;
    }
    out << line << '\n';
  }
  out.close();
  in.close();
  if (std::rename(tmp.c_str(), path.c_str()) != 0) {
    perror("rename");
    return false;
  }
  return true;
}

// Run a built binary and measure time (returns pair<exit_code,
// elapsed_seconds>)
static std::pair<int, double> run_binary_and_time(const std::string &binPath) {
  std::vector<std::string> args = {binPath};
  double elapsed = 0.0;
  int rc = run_execvp(args, &elapsed);
  return {rc, elapsed};
}

// ----------------------------- Main -----------------------------

int main(int argc, char *argv[]) {
  llvm::llvm_shutdown_obj shutdown_on_exit;

  // --- REPL mode (if no args) ---
  if (argc < 2) {
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
      {
        std::ofstream out(tempFile);
        out << codeBuffer;
      }

      // Compile to LLVM IR
      std::string irFile = tempFile + ".ll";
      std::string tmpIr = irFile + ".new";
      codegen::LLVMCodeGenerator codegen;
      lexer::Lexer lexer(codeBuffer);
      auto tokens = lexer.tokenize();
      auto ast = parser::buildAST(tokens);
      if (ast) {
        codegen.generate(ast.get(), tmpIr);
        // sanitize IR (remove target triple) in tmp
        remove_target_triple_line(tmpIr);

        // If final IR exists and identical, remove tmp and skip rebuild
        if (file_exists(irFile) && files_are_equal(tmpIr, irFile)) {
          std::remove(tmpIr.c_str());
          std::cout << "IR unchanged; using cached IR (REPL)\n";
        } else {
          // move tmp -> irFile (overwrite)
          if (std::rename(tmpIr.c_str(), irFile.c_str()) != 0) {
            // fallback to copy
            std::ifstream in(tmpIr, std::ios::binary);
            std::ofstream out(irFile, std::ios::binary);
            out << in.rdbuf();
            in.close();
            out.close();
            std::remove(tmpIr.c_str());
          }
        }

        // Assemble + link + run (REPL uses quick flow; adjust flags if you want
        // sanitizers)
        std::string bcFile = "repl_temp.bc";
        if (run_execvp({"llvm-as", irFile, "-o", bcFile}) != 0) {
          std::cerr << "llvm-as failed\n";
        } else {
          std::string stdlib = "./build/src/packages/tspp_std/libtspp_std.a";
          int rc = run_execvp(
              {"clang", irFile, stdlib, "-o", "repl_temp_exec", "-lstdc++"});
          if (rc != 0) {
            std::cerr << "clang link failed\n";
          } else {
            std::cout << "Output: \n";
            run_execvp({"./repl_temp_exec"});
          }
        }
      } else {
        std::cerr << "Parse error\n";
      }
      codeBuffer.clear();
    }
    return 0;
  }

  // --- File execution mode ---
  std::string inputFile = argv[1];
  std::ifstream file(inputFile);
  if (!file) {
    std::cerr << "Could not open file: " << inputFile << std::endl;
    return 1;
  }
  std::string source((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

  // Emit IR (to temp file first)
  std::string irFile = inputFile + ".ll";
  std::string tmpIr = irFile + ".new";
  lexer::Lexer lexer(source);
  auto tokens = lexer.tokenize();
  auto ast = parser::buildAST(tokens);
  if (!ast) {
    std::cerr << "Parse error\n";
    return 1;
  }

  codegen::LLVMCodeGenerator codegen;
  codegen.generate(ast.get(), tmpIr);
  // sanitize tmp IR (remove target triple)
  remove_target_triple_line(tmpIr);

  // If final IR exists and identical, remove tmp and skip rebuild
  bool ir_changed = true;
  if (file_exists(irFile) && files_are_equal(tmpIr, irFile)) {
    std::remove(tmpIr.c_str());
    ir_changed = false;
    std::cout << "IR unchanged; skipping IR update.\n";
  } else {
    // Replace final IR (atomic-ish via rename)
    if (std::rename(tmpIr.c_str(), irFile.c_str()) != 0) {
      // fallback to copy
      std::ifstream in(tmpIr, std::ios::binary);
      std::ofstream out(irFile, std::ios::binary);
      out << in.rdbuf();
      in.close();
      out.close();
      std::remove(tmpIr.c_str());
    }
    ir_changed = true;
  }

  // Build artifact names
  const std::string outOpt = "benchmark.opt.ll";
  const std::string objFile = "benchmark.o";
  const std::string binFile = "bench_native";

  // If binary exists AND binary is newer than IR, and IR didn't change, we can
  // reuse it.
  if (file_exists(binFile) && !ir_changed &&
      !file_a_newer_than_b(irFile, binFile)) {
    std::cout << "Using cached " << binFile << " (up-to-date)\n";
  } else {
    std::cout << "Optimizing IR -> " << outOpt << std::endl;
    // opt -S -O3 in-process via exec
    if (run_execvp({"opt", "-S", "-O3", irFile, "-o", outOpt}) != 0) {
      std::cerr << "Error: IR optimization failed." << std::endl;
      return 2;
    }

    std::cout << "Assembling -> " << objFile << std::endl;
    if (run_execvp({"llc", "-filetype=obj", outOpt, "-o", objFile}) != 0) {
      std::cerr << "Error: Assembly failed." << std::endl;
      return 3;
    }

    std::cout << "Linking -> " << binFile << std::endl;
    if (run_execvp({"clang", "-O3", "-march=native", objFile, "-o", binFile,
                    "-flto"}) != 0) {
      std::cerr << "Error: Linking failed." << std::endl;
      return 4;
    }
  }

  std::cout << "Built " << binFile << "; running time..." << std::endl;
  auto [rc, elapsed] = run_binary_and_time("./" + binFile);
  if (rc < 0) {
    std::cerr << "Error running " << binFile << std::endl;
    return 5;
  }
  std::cout << "Exit code: " << rc << ", elapsed: " << elapsed << "s\n";

  return 0;
}
