#include "memory.h"

#include <iostream>

namespace tspp_std {

void MemoryStd::registerMakeShared() {
  // Register make_shared<T> function
  std::cout << "Registering make_shared function" << std::endl;
}

void MemoryStd::registerMakeUnique() {
  // Register make_unique<T> function
  std::cout << "Registering make_unique function" << std::endl;
}

void MemoryStd::registerMakeWeak() {
  // Register make_weak<T> function
  std::cout << "Registering make_weak function" << std::endl;
}

void MemoryStd::registerSmartPtrValid() {
  // Register valid() method for smart pointers
  std::cout << "Registering smart pointer valid() method" << std::endl;
}

void MemoryStd::registerSmartPtrReset() {
  // Register reset() method for smart pointers
  std::cout << "Registering smart pointer reset() method" << std::endl;
}

void MemoryStd::registerSmartPtrGet() {
  // Register get() method for smart pointers
  std::cout << "Registering smart pointer get() method" << std::endl;
}

void MemoryStd::registerAlloc() {
  // Register alloc function for manual memory management
  std::cout << "Registering alloc function" << std::endl;
}

void MemoryStd::registerFree() {
  // Register free function for manual memory management
  std::cout << "Registering free function" << std::endl;
}

void MemoryStd::registerDestroy() {
  // Register destroy function
  std::cout << "Registering destroy function" << std::endl;
}

void MemoryStd::registerAll() {
  registerMakeShared();
  registerMakeUnique();
  registerMakeWeak();
  registerSmartPtrValid();
  registerSmartPtrReset();
  registerSmartPtrGet();
  registerAlloc();
  registerFree();
  registerDestroy();
}

}  // namespace tspp_std
