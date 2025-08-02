
#pragma once
#include <memory>
#include <string>

namespace ast {
class BaseNode;
}

// Symbol information for variables, functions, classes, etc.
struct Symbol {
  std::string name;
  std::string typeName;  // e.g. "int", "bool", or user type
  bool isConst = false;
  bool isFunction = false;
  bool isClass = false;
  std::shared_ptr<ast::BaseNode> declNode;  // Pointer to declaration node
  void* llvmType = nullptr; // (void* to avoid direct LLVM dependency in headers)
  // Add more fields as needed (e.g. parameter types, etc.)
};
