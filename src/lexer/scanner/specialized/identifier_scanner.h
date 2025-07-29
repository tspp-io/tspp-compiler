#pragma once
#include "../base/scanner_base.h"
namespace lexer {
namespace scanner {
class IdentifierScanner : public ScannerBase {
 public:
  tokens::Token scan(const std::string& src, size_t& pos) override;
};
}  // namespace scanner
}  // namespace lexer
