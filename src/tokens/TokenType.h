#pragma once
/**
 * Enumeration of all possible token types in the language.
 * Each section represents a different category of tokens.
 */
namespace tokens {
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
   * Storage and Memory Modifiers
   *****************************************************************************/
  STORAGE_BEGIN,
  STACK = STORAGE_BEGIN,  // '#stack' storage modifier
  HEAP,                   // '#heap' storage modifier
  STATIC,                 // '#static' storage modifier
  SHARED,                 // '#shared' smart pointer type
  UNIQUE,                 // '#unique' smart pointer type
  WEAK,                   // '#weak' smart pointer type
  ATTRIBUTE,              // '#'
  STORAGE_END = ATTRIBUTE,

  /*****************************************************************************
   * Function and Method Modifiers
   *****************************************************************************/
  FUNC_MOD_BEGIN,
  INLINE = FUNC_MOD_BEGIN,  // '#inline' function modifier
  VIRTUAL,                  // '#virtual' function modifier - moved here
  UNSAFE,                   // '#unsafe' function modifier
  SIMD,                     // '#simd' function modifier
  TARGET,                   // '#target' platform specific code
  REF,                      // 'ref' parameter modifier
  FUNC_MOD_END = REF,

  /*****************************************************************************
   * Class Modifiers
   *****************************************************************************/
  CLASS_MOD_BEGIN,
  ALIGNED = CLASS_MOD_BEGIN,  // '#aligned' memory alignment
  PACKED,                     // '#packed' memory layout
  ABSTRACT,                   // '#abstract' class modifier
  ZEROCAST,                   // '#zerocast' interface modifier
  EXTENDS,                    // 'extends' inheritance
  IMPLEMENTS,                 // 'implements' interface implementation
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
}
