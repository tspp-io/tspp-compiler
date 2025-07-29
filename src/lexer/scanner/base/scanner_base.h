#pragma once
#include <string>

#include "../../../tokens/Token.h"
namespace lexer {
namespace scanner {
class ScannerBase {
 public:
  virtual ~ScannerBase() = default;
  virtual tokens::Token scan(const std::string& src, size_t& pos) = 0;
};
}  // namespace scanner
}  // namespace lexer
