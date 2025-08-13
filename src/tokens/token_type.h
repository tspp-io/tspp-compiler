#pragma once

namespace tokens {

/**
 * Enumeration of all possible token types in the language.
 * Each section represents a different category of tokens.
 */
enum class TokenType {
  /*****************************************************************************
   * Declaration Keywords
   *****************************************************************************/
  DECL_BEGIN,
  LET = DECL_BEGIN,  // 'let' keyword for variable declaration
  CONST,             // 'const' keyword for constant values
  FUNCTION,          // 'function' keyword for function declarations
  CLASS,             // 'class' keyword for class definitions
  INTERFACE,         // 'interface' keyword for interface definitions
  ENUM,              // 'enum' keyword for enumeration types
  CONSTRUCTOR,       // 'CONSTRUCTOR' method to init class
  TYPEDEF,           // 'typedef' keyword for type aliases
  NAMESPACE,         // 'namespace' keyword for namespace declarations
  TEMPLATE,          // 'template' keyword for template declarations
  IMPORT,            // 'import' keyword for module imports
  FROM,              // 'from' keyword for module imports
  NEW,               // 'new' keyword to create new class instance
  GET,
  SET,
  CAST,
  DECL_END = NEW,

  /*****************************************************************************
   * Generic and Template Related
   *****************************************************************************/
  GENERIC_BEGIN,
  WHERE = GENERIC_BEGIN,  // 'where' keyword for generic constraints
  THROWS,                 // 'throws' keyword for exception specification
  GENERIC_END = THROWS,

  /*****************************************************************************
   * Access Modifiers
   *****************************************************************************/
  ACCESS_BEGIN,
  PUBLIC = ACCESS_BEGIN,  // 'public' access modifier
  PRIVATE,                // 'private' access modifier
  PROTECTED,              // 'protected' access modifier
  ACCESS_END = PROTECTED,

  /*****************************************************************************
   * Control Flow Keywords
   *****************************************************************************/
  CONTROL_BEGIN,
  IF = CONTROL_BEGIN,  // 'if' conditional statement
  ELSE,                // 'else' conditional statement
  SWITCH,              // 'switch' statement
  CASE,                // 'case' label
  DEFAULT,             // 'default' label
  WHILE,               // 'while' loop
  DO,                  // 'do-while' loop
  FOR,                 // 'for' loop
  OF,                  // 'of' keyword for for-of loops
  BREAK,               // 'break' statement
  CONTINUE,            // 'continue' statement
  RETURN,              // 'return' statement
  TRY,                 // 'try' block
  CATCH,               // 'catch' block
  FINALLY,             // 'finally' block
  THROW,               // 'throw' statement
  CONTROL_END = THROW,

  /*****************************************************************************
   * Type Keywords
   *****************************************************************************/
  TYPE_BEGIN,
  VOID = TYPE_BEGIN,  // 'void' type
  INT,                // 'int' type
  FLOAT,              // 'float' type
  BOOLEAN,            // 'boolean' type
  STRING,             // 'string' type
  TYPE_END = STRING,

  /*****************************************************************************
   * Storage and Memory Qualifiers (used with variables)
   *****************************************************************************/
  STORAGE_BEGIN,
  STACK = STORAGE_BEGIN,  // '#stack' storage qualifier
  HEAP,                   // '#heap' storage qualifier
  STATIC,                 // '#static' storage qualifier
  STORAGE_END = STATIC,

  /*****************************************************************************
   * Smart Pointer Modifiers (used in type annotations)
   *****************************************************************************/
  SMARTPTR_BEGIN,
  SHARED = SMARTPTR_BEGIN,  // '#shared' smart pointer type
  UNIQUE,                   // '#unique' smart pointer type
  WEAK,                     // '#weak' smart pointer type
  SMARTPTR_END = WEAK,

  /*****************************************************************************
   * Pointer Type Modifiers (used with pointer types)
   *****************************************************************************/
  PTRMOD_BEGIN,
  UNSAFE = PTRMOD_BEGIN,  // '@unsafe' pointer type modifier
  ALIGNED,                // '@aligned' pointer type modifier
  PTRMOD_END = ALIGNED,

  /*****************************************************************************
   * Function and Method Modifiers (used with functions/classes)
   *****************************************************************************/
  FUNC_MOD_BEGIN,
  CONST_FUNCTION = FUNC_MOD_BEGIN,  // '#const' function modifier
  CONSTEXPR,                        // '#constexpr' function modifier
  ZEROCAST,                         // '#zerocast' function modifier
  SIMD,                             // '#simd' function modifier
  PREFETCH,                         // '#prefetch' function modifier
  ATOMIC,                           // '#atomic' function modifier
  PINNED,                           // '#pinned' function modifier
  FUNC_MOD_END = PINNED,

  /*****************************************************************************
   * Method attribute tokens (kept separate from FUNC_MOD range)
   *****************************************************************************/
  METHOD_ATTR_BEGIN,
  INLINE = METHOD_ATTR_BEGIN,  // '#inline' method attribute
  VIRTUAL,                     // '#virtual' method attribute
  OVERRIDE,                    // '#override' method attribute
  METHOD_ATTR_END = OVERRIDE,

  /*****************************************************************************
   * Field modifiers (attributes applied to fields)
   *****************************************************************************/
  FIELD_MOD_BEGIN,
  READONLY = FIELD_MOD_BEGIN,  // '#readonly' field modifier
  VOLATILE,                    // '#volatile' field modifier
  FIELD_MOD_END = VOLATILE,

  /*****************************************************************************
   * Attribute/Meta Tokens
   *****************************************************************************/
  ATTRIBUTE,  // '#' (generic attribute marker)

  /*****************************************************************************
   * Class Modifiers
   *****************************************************************************/
  CLASS_MOD_BEGIN,
  PACKED = CLASS_MOD_BEGIN,  // '#packed' memory layout
  ABSTRACT,                  // '#abstract' class modifier
  FINAL,                     // '#final' class modifier
  EXTENDS,                   // 'extends' inheritance
  IMPLEMENTS,                // 'implements' interface implementation
  CLASS_MOD_END = IMPLEMENTS,

  /*****************************************************************************
   * Compile-Time Keywords
   *****************************************************************************/
  COMPILE_BEGIN,
  CONST_EXPR = COMPILE_BEGIN,  // '#const' compile-time expression
  SIZEOF,                      // '#sizeof' operator
  ALIGNOF,                     // '#alignof' operator
  TYPEOF,                      // '#typeof' operator
  ASM,                         // '#asm' inline assembly
  COMPILE_END = ASM,

  /*****************************************************************************
   * Literals and Values
   *****************************************************************************/
  LITERAL_BEGIN,
  IDENTIFIER = LITERAL_BEGIN,  // Variable/function names
  NUMBER,                      // Numeric literals
  STRING_LITERAL,              // String literals
  CHAR_LITERAL,                // Character literals
  TRUE,                        // 'true' boolean literal
  FALSE,                       // 'false' boolean literal
  NULL_VALUE,                  // 'null' value
  UNDEFINED,                   // 'undefined' value
  THIS,                        // 'this' keyword
  LITERAL_END = THIS,

  /*****************************************************************************
   * Operators
   *****************************************************************************/
  OPERATOR_BEGIN,
  // Arithmetic
  PLUS = OPERATOR_BEGIN,  // '+'
  MINUS,                  // '-'
  STAR,                   // '*'
  SLASH,                  // '/'
  PERCENT,                // '%'

  // Bitwise
  AMPERSAND,    // '&'
  PIPE,         // '|'
  CARET,        // '^'
  TILDE,        // '~'
  RIGHT_SHIFT,  // '>>'
  LEFT_SHIFT,   // '<<'

  // Logical
  EXCLAIM,              // '!'
  AMPERSAND_AMPERSAND,  // '&&'
  PIPE_PIPE,            // '||'

  // Comparison
  EQUALS,          // '='
  EQUALS_EQUALS,   // '=='
  EXCLAIM_EQUALS,  // '!='
  LESS,            // '<'
  GREATER,         // '>'
  LESS_EQUALS,     // '<='
  GREATER_EQUALS,  // '>='

  // Compound Assignment
  PLUS_EQUALS,       // '+='
  MINUS_EQUALS,      // '-='
  STAR_EQUALS,       // '*='
  SLASH_EQUALS,      // '/='
  PERCENT_EQUALS,    // '%='
  AMPERSAND_EQUALS,  // '&='
  PIPE_EQUALS,       // '|='
  CARET_EQUALS,      // '^='

  // Increment/Decrement
  PLUS_PLUS,    // '++'
  MINUS_MINUS,  // '--'

  // Other
  QUESTION,  // '?'
  COLON,     // ':'
  ARROW,     // '->'
  DOT,       // '.'
  AT,        // '@'
  OPERATOR_END = AT,

  /*****************************************************************************
   * Delimiters
   *****************************************************************************/
  DELIMITER_BEGIN,
  LEFT_PAREN = DELIMITER_BEGIN,  // '('
  RIGHT_PAREN,                   // ')'
  LEFT_BRACE,                    // '{'
  RIGHT_BRACE,                   // '}'
  LEFT_BRACKET,                  // '['
  RIGHT_BRACKET,                 // ']'
  SEMICOLON,                     // ';'
  COMMA,                         // ','
  DELIMITER_END = COMMA,

  /*****************************************************************************
   * Additional Token Types for Enhanced Language Features
   *****************************************************************************/
  FEATURE_BEGIN,
  YIELD = FEATURE_BEGIN,  // 'yield' keyword for generators
  ASYNC,                  // 'async' keyword for async functions
  AWAIT,                  // 'await' keyword for async/await
  FEATURE_END = AWAIT,

  // Token for error
  ERROR_TOKEN,

  // Special token for end of file
  END_OF_FILE
};

/**
 * Token category check functions implementation
 */
inline bool isDeclaration(TokenType type) {
  return type >= TokenType::DECL_BEGIN && type <= TokenType::DECL_END;
}

inline bool isType(TokenType type) {
  return type >= TokenType::TYPE_BEGIN && type <= TokenType::TYPE_END;
}

inline bool isControlFlow(TokenType type) {
  return type >= TokenType::CONTROL_BEGIN && type <= TokenType::CONTROL_END;
}

inline bool isClassRelated(TokenType type) {
  return type >= TokenType::CLASS_MOD_BEGIN && type <= TokenType::CLASS_MOD_END;
}

inline bool isMemoryManagement(TokenType type) {
  return type >= TokenType::STORAGE_BEGIN && type <= TokenType::STORAGE_END;
}

inline bool isOperator(TokenType type) {
  return type >= TokenType::OPERATOR_BEGIN && type <= TokenType::OPERATOR_END;
}

inline bool isAssignmentOperator(TokenType type) {
  static const TokenType assignmentOps[] = {
      TokenType::EQUALS,           TokenType::PLUS_EQUALS,
      TokenType::MINUS_EQUALS,     TokenType::STAR_EQUALS,
      TokenType::SLASH_EQUALS,     TokenType::PERCENT_EQUALS,
      TokenType::AMPERSAND_EQUALS, TokenType::PIPE_EQUALS,
      TokenType::CARET_EQUALS};

  for (const auto &op : assignmentOps) {
    if (type == op) return true;
  }
  return false;
}

inline bool isArithmeticOperator(TokenType type) {
  static const TokenType arithmeticOps[] = {
      TokenType::PLUS,       TokenType::MINUS,   TokenType::STAR,
      TokenType::SLASH,      TokenType::PERCENT, TokenType::PLUS_PLUS,
      TokenType::MINUS_MINUS};

  for (const auto &op : arithmeticOps) {
    if (type == op) return true;
  }
  return false;
}

inline bool isBitwiseOperator(TokenType type) {
  static const TokenType bitwiseOps[] = {TokenType::AMPERSAND, TokenType::PIPE,
                                         TokenType::CARET, TokenType::TILDE};

  for (const auto &op : bitwiseOps) {
    if (type == op) return true;
  }
  return false;
}

inline bool isLogicalOperator(TokenType type) {
  static const TokenType logicalOps[] = {
      TokenType::EXCLAIM, TokenType::AMPERSAND_AMPERSAND, TokenType::PIPE_PIPE};

  for (const auto &op : logicalOps) {
    if (type == op) return true;
  }
  return false;
}

inline bool isComparisonOperator(TokenType type) {
  static const TokenType comparisonOps[] = {
      TokenType::EQUALS_EQUALS, TokenType::EXCLAIM_EQUALS,
      TokenType::LESS,          TokenType::GREATER,
      TokenType::LESS_EQUALS,   TokenType::GREATER_EQUALS};

  for (const auto &op : comparisonOps) {
    if (type == op) return true;
  }
  return false;
}

/**
 * Additional checker functions for enhanced language features
 */
inline bool isGenericConstraint(TokenType type) {
  static const TokenType constraintOps[] = {
      TokenType::WHERE, TokenType::EXTENDS, TokenType::IMPLEMENTS};

  for (const auto &op : constraintOps) {
    if (type == op) return true;
  }
  return false;
}

inline bool isTemplateKeyword(TokenType type) {
  return type == TokenType::TEMPLATE;
}

/**
 * Enhanced operator classification that considers operator context
 */
inline bool isUnaryOperator(TokenType type, bool prefixPosition = true) {
  // Operators that can only be prefix unary
  static const TokenType prefixOnlyOps[] = {TokenType::EXCLAIM,
                                            TokenType::TILDE};

  // Operators that can only be postfix unary
  static const TokenType postfixOnlyOps[] = {TokenType::PLUS_PLUS,
                                             TokenType::MINUS_MINUS};

  // Operators that can be both unary and binary
  static const TokenType dualOps[] = {TokenType::PLUS, TokenType::MINUS};

  if (prefixPosition) {
    for (const auto &op : prefixOnlyOps) {
      if (type == op) return true;
    }
  }

  // Check postfix operators if not in prefix position
  if (!prefixPosition) {
    for (const auto &op : postfixOnlyOps) {
      if (type == op) return true;
    }
  }

  // Check dual-purpose operators in either position
  for (const auto &op : dualOps) {
    if (type == op) return true;
  }

  return false;
}

/**
 * Helper function to determine if a token can be both unary and binary
 */
inline bool isDualPurposeOperator(TokenType type) {
  static const TokenType dualOps[] = {TokenType::PLUS, TokenType::MINUS};

  for (const auto &op : dualOps) {
    if (type == op) return true;
  }
  return false;
}

/**
 * Enhanced function to check if a token is part of modern language features
 */
inline bool isModernFeature(TokenType type) {
  return type >= TokenType::FEATURE_BEGIN && type <= TokenType::FEATURE_END;
}

inline bool isLiteral(TokenType type) {
  return type >= TokenType::LITERAL_BEGIN && type <= TokenType::LITERAL_END;
}

inline bool isDelimiter(TokenType type) {
  return type >= TokenType::DELIMITER_BEGIN && type <= TokenType::DELIMITER_END;
}

inline bool isSpecial(TokenType type) {
  return type >= TokenType::COMPILE_BEGIN && type <= TokenType::COMPILE_END;
}

/**
 * Additional utility functions for token classification
 */
inline bool isAccessModifier(TokenType type) {
  return type >= TokenType::ACCESS_BEGIN && type <= TokenType::ACCESS_END;
}

inline bool isFunctionModifier(TokenType type) {
  return type >= TokenType::FUNC_MOD_BEGIN && type <= TokenType::FUNC_MOD_END;
}

inline bool isMethodAttribute(TokenType type) {
  return type >= TokenType::METHOD_ATTR_BEGIN &&
         type <= TokenType::METHOD_ATTR_END;
}

inline bool isFieldModifier(TokenType type) {
  return type >= TokenType::FIELD_MOD_BEGIN && type <= TokenType::FIELD_MOD_END;
}

inline bool isClassModifier(TokenType type) {
  return type >= TokenType::CLASS_MOD_BEGIN && type <= TokenType::CLASS_MOD_END;
}

}  // namespace tokens
