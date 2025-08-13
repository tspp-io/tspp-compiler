#pragma once
#include <optional>
#include <string>

#include "core/common/common_types.h"
#include "token_type.h"

namespace tokens {

/**
 * Class representing a token with type, lexeme, and location information
 */
class Token {
 public:
  /**
   * Default constructor
   */
  Token() : type_(TokenType::END_OF_FILE) {}

  /**
   * Regular token constructor
   */
  Token(TokenType type, std::string lexeme, core::Location location)
      : type_(type),
        lexeme_(std::move(lexeme)),
        location_(std::move(location)) {}

  /**
   * Factory method for creating error tokens
   */
  static Token createError(std::string lexeme, core::Location location,
                           std::string errorMessage) {
    Token token(TokenType::ERROR_TOKEN, std::move(lexeme), std::move(location));
    token.errorMessage_ = std::move(errorMessage);
    return token;
  }

  /**
   * Token information access
   */
  TokenType getType() const {
    return type_;
  }
  const std::string &getLexeme() const {
    return lexeme_;
  }
  const core::Location &getLocation() const {
    return location_;
  }
  const std::optional<std::string> &getErrorMessage() const {
    return errorMessage_;
  }

  /**
   * Token classification methods
   */
  bool isError() const {
    return type_ == TokenType::ERROR_TOKEN;
  }
  bool isEOF() const {
    return type_ == TokenType::END_OF_FILE;
  }

  // Token category checks using token_type.h functions
  bool isDeclaration() const {
    return tokens::isDeclaration(type_);
  }
  bool isType() const {
    return tokens::isType(type_);
  }
  bool isControlFlow() const {
    return tokens::isControlFlow(type_);
  }
  bool isClassRelated() const {
    return tokens::isClassRelated(type_);
  }
  bool isOperator() const {
    return tokens::isOperator(type_);
  }
  bool isMethodAttribute() const {
    return tokens::isMethodAttribute(type_);
  }
  bool isFieldModifier() const {
    return tokens::isFieldModifier(type_);
  }
  bool isLiteral() const {
    return tokens::isLiteral(type_);
  }
  bool isDelimiter() const {
    return tokens::isDelimiter(type_);
  }
  bool isSpecial() const {
    return tokens::isSpecial(type_);
  }

  /**
   * @brief Returns a string representation of the token for debugging.
   */
  std::string toString() const;

 private:
  TokenType type_;                           // Type of token
  std::string lexeme_;                       // Actual text of token
  core::Location location_;                  // Position in source
  std::optional<std::string> errorMessage_;  // Error information if any
};

}  // namespace tokens