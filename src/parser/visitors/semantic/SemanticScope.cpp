#include "SemanticScope.h"

SemanticScope::SemanticScope(std::shared_ptr<SemanticScope> parent)
    : parent(std::move(parent)) {}

bool SemanticScope::insert(const std::string& name, const Symbol& symbol) {
  if (symbols.find(name) != symbols.end()) {
    return false;  // Already declared in this scope
  }
  symbols[name] = symbol;
  return true;
}

const Symbol* SemanticScope::lookup(const std::string& name) const {
  auto it = symbols.find(name);
  if (it != symbols.end()) {
    return &it->second;
  }
  if (parent) {
    return parent->lookup(name);
  }
  return nullptr;
}

const Symbol* SemanticScope::lookupCurrent(const std::string& name) const {
  auto it = symbols.find(name);
  if (it != symbols.end()) {
    return &it->second;
  }
  return nullptr;
}

std::shared_ptr<SemanticScope> SemanticScope::getParent() const {
  return parent;
}

bool SemanticScope::containsClassSymbol() const {
  for (const auto& entry : symbols) {
    if (entry.second.isClass) {
      return true;
    }
  }
  return false;
}
