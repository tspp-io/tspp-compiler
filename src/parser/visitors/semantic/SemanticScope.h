#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Symbol.h"

// Represents a single scope in the semantic analyzer (e.g., global, function,
// block)
class SemanticScope {
 public:
  SemanticScope(std::shared_ptr<SemanticScope> parent = nullptr);

  // Insert a symbol into the current scope. Returns false if already declared
  // in this scope.
  bool insert(const std::string& name, const Symbol& symbol);

  // Lookup a symbol in the current scope stack (innermost to outermost).
  // Returns nullptr if not found.
  const Symbol* lookup(const std::string& name) const;

  // Lookup a symbol only in the current scope (not parent scopes).
  const Symbol* lookupCurrent(const std::string& name) const;

  // Get the parent scope.
  std::shared_ptr<SemanticScope> getParent() const;

  // Returns true if this scope contains a class symbol
  bool containsClassSymbol() const;

 private:
  std::unordered_map<std::string, Symbol> symbols;
  std::shared_ptr<SemanticScope> parent;
};
