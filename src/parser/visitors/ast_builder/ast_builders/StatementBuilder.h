#pragma once
#include "core/common/macros.h"
#include "parser/nodes/statement_nodes.h"
#include "tokens/stream/token_stream.h"

namespace parser {

/**
 * @brief Builder for statement AST nodes
 * Handles parsing of if, while, do-while, for, for-of, switch, break,
 * continue, return, throw, try/catch/finally, and block statements
 */
class StatementBuilder {
 public:
  /**
   * @brief Build any statement from token stream
   * @param stream Token stream to parse from
   * @return Shared pointer to statement node, or nullptr on error
   */
  static Shared(ast::Stmt) build(tokens::TokenStream& stream);

  /**
   * @brief Build block statement specifically
   * @param stream Token stream to parse from
   * @return Shared pointer to block statement, or nullptr on error
   */
  static Shared(ast::BlockStmt) buildBlock(tokens::TokenStream& stream);

  /**
   * @brief Build if statement
   * @param stream Token stream to parse from
   * @return Shared pointer to if statement, or nullptr on error
   */
  static Shared(ast::IfStmt) buildIf(tokens::TokenStream& stream);

  /**
   * @brief Build while statement
   * @param stream Token stream to parse from
   * @return Shared pointer to while statement, or nullptr on error
   */
  static Shared(ast::WhileStmt) buildWhile(tokens::TokenStream& stream);

  /**
   * @brief Build for statement (C-style or for-of)
   * @param stream Token stream to parse from
   * @return Shared pointer to ForStmt or ForOfStmt, or nullptr on error
   */
  static Shared(ast::Stmt) buildFor(tokens::TokenStream& stream);

  /**
   * @brief Build do-while statement: do stmt while (expr);
   * @param stream Token stream to parse from
   * @return Shared pointer to do-while statement, or nullptr on error
   */
  static Shared(ast::DoWhileStmt) buildDoWhile(tokens::TokenStream& stream);

  /**
   * @brief Build switch statement
   * @param stream Token stream to parse from
   * @return Shared pointer to switch statement, or nullptr on error
   */
  static Shared(ast::SwitchStmt) buildSwitch(tokens::TokenStream& stream);

  /**
   * @brief Build break or continue statement
   * @param stream Token stream to parse from
   * @return Shared pointer to BreakStmt/ContinueStmt, or nullptr on error
   */
  static Shared(ast::Stmt) buildJump(tokens::TokenStream& stream);

  /**
   * @brief Build throw statement: throw expr;
   * @param stream Token stream to parse from
   * @return Shared pointer to throw statement, or nullptr on error
   */
  static Shared(ast::ThrowStmt) buildThrow(tokens::TokenStream& stream);

  /**
   * @brief Build try statement with optional catch and finally clauses
   * @param stream Token stream to parse from
   * @return Shared pointer to try statement, or nullptr on error
   */
  static Shared(ast::TryStmt) buildTry(tokens::TokenStream& stream);

  /**
   * @brief Build return statement
   * @param stream Token stream to parse from
   * @return Shared pointer to return statement, or nullptr on error
   */
  static Shared(ast::ReturnStmt) buildReturn(tokens::TokenStream& stream);

  /**
   * @brief Build asm statement
   * @param stream Token stream to parse from
   * @return Shared pointer to asm statement, or nullptr on error
   */
  static Shared(ast::AsmStmt) buildAsm(tokens::TokenStream& stream);

  /**
   * @brief Build expression statement
   * @param stream Token stream to parse from
   * @return Shared pointer to expression statement, or nullptr on error
   */
  static Shared(ast::ExprStmt)
      buildExpressionStatement(tokens::TokenStream& stream);
};

}  // namespace parser
