/*****************************************************************************
 * File: identifier_scanner.h
 *
 * Description:
 *   Scanner component for handling identifiers and keywords in the TSPP
 *language. Handles scanning of identifiers, keywords, and attribute names.
 *
 * Features:
 *   - Identifier validation
 *   - Keyword recognition
 *   - Attribute scanning
 *   - Error reporting for invalid identifiers
 *****************************************************************************/

#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

#include "lexer/scanner/base/scanner_base.h"
#include "tokens/token_type.h"
#include "tokens/tokens.h"

namespace lexer {

class IdentifierScanner : public ScannerBase {
 public:
  explicit IdentifierScanner(std::shared_ptr<LexerState> state);

  /**
   * @brief Scan an identifier or keyword token
   * @return Token representing identifier or keyword
   *
   * Recognizes:
   *   - Identifiers
   *   - Keywords (see TokenType::DECL_BEGIN, CONTROL_BEGIN, etc)
   *   - Storage/memory qualifiers (TokenType::STORAGE_BEGIN/STORAGE_END)
   *   - Smart pointer modifiers (TokenType::SMARTPTR_BEGIN/SMARTPTR_END)
   *   - Pointer type modifiers (TokenType::PTRMOD_BEGIN/PTRMOD_END)
   *   - Function/method modifiers (TokenType::FUNC_MOD_BEGIN/FUNC_MOD_END)
   *   - Class modifiers (TokenType::CLASS_MOD_BEGIN/CLASS_MOD_END)
   */
  tokens::Token scan();

  /**
   * @brief Scan an attribute token (starts with #)
   * @return Token representing the attribute
   *
   * Recognizes all attribute-style modifiers and maps to correct token type.
   */
  tokens::Token scanAttribute();

 private:
  static const std::unordered_map<std::string, tokens::TokenType>&
  getKeywordMap();
  bool validateIdentifier(std::string_view lexeme);
  tokens::TokenType identifierType(std::string_view lexeme);
};

}  // namespace lexer