#include "ModuleResolver.h"

#include <filesystem>
#include <iostream>

#include "core/utils/file_utils.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

namespace fs = std::filesystem;

namespace parser::modules {

ModuleResolver::ModuleResolver() = default;

static std::string dirOf(const std::string& path) {
  fs::path p(path);
  return p.has_parent_path() ? p.parent_path().string() : std::string(".");
}

ImportInfo ModuleResolver::toImportInfo(const ast::ImportDecl& decl) {
  ImportInfo info;
  info.identifier = decl.name.getLexeme();
  if (!decl.fromPath.empty()) {
    info.source = stripQuotes(decl.fromPath);
    info.type = ImportType::FILE;
  } else {
    info.type = ImportType::LIBRARY;
  }
  return info;
}

std::string ModuleResolver::stripQuotes(const std::string& s) {
  if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                        (s.front() == '\'' && s.back() == '\''))) {
    return s.substr(1, s.size() - 2);
  }
  return s;
}

bool ModuleResolver::isRelative(const std::string& s) {
  return s.rfind("./", 0) == 0 || s.rfind("../", 0) == 0;
}

std::string ModuleResolver::resolvePath(const std::string& spec,
                                        const std::string& currentFile) {
  // Legacy: try to produce a canonical path, but don't assume it exists
  fs::path base = isRelative(spec) ? fs::path(dirOf(currentFile))
                                   : fs::path(dirOf(currentFile));
  fs::path p = isRelative(spec) ? (base / spec) : fs::path(spec);
  return fs::weakly_canonical(p).string();
}

bool ModuleResolver::checkCircular(const std::string& path) {
  for (const auto& p : dependencyStack_) {
    if (p == path) {
      std::cerr << "Circular dependency detected: ";
      for (const auto& x : dependencyStack_) std::cerr << x << " -> ";
      std::cerr << path << std::endl;
      return true;
    }
  }
  return false;
}

void ModuleResolver::enter(const std::string& path) {
  dependencyStack_.push_back(path);
}
void ModuleResolver::exit(const std::string& path) {
  if (!dependencyStack_.empty() && dependencyStack_.back() == path)
    dependencyStack_.pop_back();
}

std::shared_ptr<ast::ProgramNode> ModuleResolver::parseModuleFile(
    const std::string& absPath) {
  auto it = moduleCache_.find(absPath);
  if (it != moduleCache_.end()) return it->second;

  auto contentOpt = core::utils::FileUtils::readFile(absPath);
  if (!contentOpt) {
    std::cerr << "Failed to read module: " << absPath << std::endl;
    return nullptr;
  }
  lexer::Lexer lx(*contentOpt, absPath);
  auto toks = lx.tokenize();
  auto program = parser::buildAST(toks);
  moduleCache_[absPath] = program;
  return program;
}

void ModuleResolver::mergeInto(
    ast::ProgramNode* root, const std::string& modulePath,
    const std::shared_ptr<ast::ProgramNode>& module) {
  if (!module) return;
  if (mergedOnce_.count(modulePath)) return;  // already merged
  mergedOnce_.insert(modulePath);

  // Prepend declarations and statements so they are available/executed before
  // root code
  root->declarations.insert(root->declarations.begin(),
                            module->declarations.begin(),
                            module->declarations.end());
  root->statements.insert(root->statements.begin(), module->statements.begin(),
                          module->statements.end());
}

bool ModuleResolver::resolveAndMerge(ast::ProgramNode* root,
                                     const std::string& entryFilePath) {
  if (!root) return false;

  // Snapshot imports to process; new imports from merged modules are processed
  // iteratively
  std::vector<std::pair<ast::ImportDecl*, std::string>>
      queue;  // (decl, currentFile)
  for (auto& imp : root->imports) {
    queue.emplace_back(imp.get(), entryFilePath);
  }

  while (!queue.empty()) {
    auto [decl, currentFile] = queue.back();
    queue.pop_back();

    ImportInfo info = toImportInfo(*decl);

    if (info.type == ImportType::LIBRARY) {
      // Map library identifiers to std package paths (placeholder mapping)
      // e.g., Math -> packages/tspp_std/math.tspp (if exists)
      // For now, we don't load any file for libraries; codegen/stdlib linkage
      // handles it.
      continue;
    }

    // FILE import
    if (!info.source) continue;
    std::string spec = *info.source;

    // Try to resolve to an existing file; consider common variants
    auto tryCandidates =
        [&](const fs::path& baseDir,
            const std::string& s) -> std::optional<std::string> {
      std::vector<fs::path> cands;
      fs::path raw = s;
      if (raw.is_relative()) cands.push_back(baseDir / raw);
      cands.push_back(raw);
      // If no extension, try adding .tspp
      for (size_t i = 0; i < cands.size(); ++i) {
        if (!cands[i].has_extension()) {
          cands.push_back(cands[i].string() + ".tspp");
        }
      }
      for (const auto& cand : cands) {
        std::error_code ec;
        fs::path norm = fs::weakly_canonical(cand, ec);
        if (ec) continue;
        if (fs::exists(norm)) return norm.string();
      }
      return std::nullopt;
    };

    auto absOpt = tryCandidates(fs::path(dirOf(currentFile)), spec);

    // If unresolved and looks like std/* or bare library path, treat as library
    // and continue
    if (!absOpt.has_value()) {
      if (!isRelative(spec)) {
        // Heuristic: library-like import path (e.g., std, std/utils) -> skip
        // file load
        continue;
      }
      // Relative but not found: warn and continue
      std::cerr << "Warning: import not found: '" << spec << "' from "
                << currentFile << "\n";
      continue;
    }

    const std::string absPath = *absOpt;

    if (checkCircular(absPath)) {
      return false;
    }
    enter(absPath);
    auto mod = parseModuleFile(absPath);
    exit(absPath);
    if (!mod) {
      std::cerr << "Warning: failed to parse imported module: " << absPath
                << "\n";
      continue;
    }

    // Merge module into root
    mergeInto(root, absPath, mod);

    // Enqueue nested imports from the module, with their current file context =
    // absPath
    for (auto& nested : mod->imports) {
      queue.emplace_back(nested.get(), absPath);
    }
  }

  return true;
}

}  // namespace parser::modules
