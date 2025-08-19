#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/common/common_types.h"
#include "parser/nodes/meta_nodes.h"

namespace parser::modules {

enum class ImportType { LIBRARY, FILE };

struct ImportInfo {
  std::string identifier;             // import Identifier
  std::optional<std::string> source;  // from "..." (raw, without quotes)
  ImportType type{ImportType::LIBRARY};
  std::optional<std::string> resolvedPath;  // absolute path for FILE
};

class ModuleResolver {
 public:
  ModuleResolver();

  // Resolve and merge imports into root program. Returns false on hard error.
  bool resolveAndMerge(ast::ProgramNode* root,
                       const std::string& entryFilePath);

  // For diagnostics/testing
  const std::unordered_map<std::string, std::shared_ptr<ast::ProgramNode>>&
  getModuleCache() const {
    return moduleCache_;
  }

 private:
  std::unordered_map<std::string, std::shared_ptr<ast::ProgramNode>>
      moduleCache_;
  std::vector<std::string> dependencyStack_;
  std::unordered_set<std::string> mergedOnce_;  // to avoid duplicate merges

  static ImportInfo toImportInfo(const ast::ImportDecl& decl);
  static std::string stripQuotes(const std::string& s);
  static bool isRelative(const std::string& s);
  static std::string resolvePath(const std::string& spec,
                                 const std::string& currentFile);

  bool checkCircular(const std::string& path);
  void enter(const std::string& path);
  void exit(const std::string& path);

  // Parse a module file into a ProgramNode (cached)
  std::shared_ptr<ast::ProgramNode> parseModuleFile(const std::string& absPath);

  // Merge declarations/statements from module into root (once per absPath)
  void mergeInto(ast::ProgramNode* root, const std::string& modulePath,
                 const std::shared_ptr<ast::ProgramNode>& module);
};

}  // namespace parser::modules
