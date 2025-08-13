#pragma once
#include <string>

#include "parser/nodes/declaration_nodes.h"
#include "parser/nodes/expression_nodes.h"
#include "parser/nodes/misc_nodes.h"
#include "parser/nodes/statement_nodes.h"

namespace ast {

// ANSI color codes for pretty-printing
inline std::string colorKey(const std::string& key) {
  return "\033[1;36m" + key + "\033[0m";  // Cyan bold
}
inline std::string colorType(const std::string& type) {
  return "\033[1;33m" + type + "\033[0m";  // Yellow bold
}

// Utility: Print FunctionModifier as string
inline const char* functionModifierToString(FunctionModifier mod) {
  switch (mod) {
    case FunctionModifier::None:
      return "None";
    case FunctionModifier::Const:
      return "Const";
    case FunctionModifier::Constexpr:
      return "Constexpr";
    case FunctionModifier::Zerocast:
      return "Zerocast";
    case FunctionModifier::Simd:
      return "Simd";
    case FunctionModifier::Prefetch:
      return "Prefetch";
    case FunctionModifier::Atomic:
      return "Atomic";
    case FunctionModifier::Pinned:
      return "Pinned";
    default:
      return "None";
  }
}

// Utility: Print ClassModifier as string
inline const char* classModifierToString(ClassModifier mod) {
  switch (mod) {
    case ClassModifier::None:
      return "None";
    case ClassModifier::Abstract:
      return "Abstract";
    case ClassModifier::Packed:
      return "Packed";
    case ClassModifier::Pinned:
      return "Pinned";
    case ClassModifier::Final:
      return "Final";
    default:
      return "None";
  }
}

inline const char* methodAttributeToString(MethodAttribute a) {
  switch (a) {
    case MethodAttribute::None:
      return "None";
    case MethodAttribute::Inline:
      return "Inline";
    case MethodAttribute::Virtual:
      return "Virtual";
    case MethodAttribute::Override:
      return "Override";
    case MethodAttribute::Static:
      return "Static";
    case MethodAttribute::Abstract:
      return "Abstract";
    default:
      return "None";
  }
}

inline const char* fieldModifierToString(FieldModifier m) {
  switch (m) {
    case FieldModifier::None:
      return "None";
    case FieldModifier::Readonly:
      return "Readonly";
    case FieldModifier::Volatile:
      return "Volatile";
    case FieldModifier::Atomic:
      return "Atomic";
    case FieldModifier::Constexpr:
      return "Constexpr";
    default:
      return "None";
  }
}

// Utility: Print StorageQualifier as string
inline const char* storageQualifierToString(StorageQualifier q) {
  switch (q) {
    case StorageQualifier::None:
      return "None";
    case StorageQualifier::Stack:
      return "Stack";
    case StorageQualifier::Heap:
      return "Heap";
    case StorageQualifier::Static:
      return "Static";
    default:
      return "None";
  }
}

// Utility: Print expression type as string
inline const char* exprTypeToString(const Expr& expr) {
  if (dynamic_cast<const BinaryExpr*>(&expr)) return "BinaryExpr";
  if (dynamic_cast<const UnaryExpr*>(&expr)) return "UnaryExpr";
  if (dynamic_cast<const LiteralExpr*>(&expr)) return "LiteralExpr";
  if (dynamic_cast<const IdentifierExpr*>(&expr)) return "IdentifierExpr";
  if (dynamic_cast<const AssignmentExpr*>(&expr)) return "AssignmentExpr";
  if (dynamic_cast<const CallExpr*>(&expr)) return "CallExpr";
  if (dynamic_cast<const MemberAccessExpr*>(&expr)) return "MemberAccessExpr";
  if (dynamic_cast<const GroupingExpr*>(&expr)) return "GroupingExpr";
  if (dynamic_cast<const ThisExpr*>(&expr)) return "ThisExpr";
  if (dynamic_cast<const NullExpr*>(&expr)) return "NullExpr";
  return "UnknownExpr";
}

// Utility: Print statement type as string
inline const char* stmtTypeToString(const Stmt& stmt) {
  if (dynamic_cast<const BlockStmt*>(&stmt)) return "BlockStmt";
  if (dynamic_cast<const IfStmt*>(&stmt)) return "IfStmt";
  if (dynamic_cast<const ReturnStmt*>(&stmt)) return "ReturnStmt";
  if (dynamic_cast<const ExprStmt*>(&stmt)) return "ExprStmt";
  return "UnknownStmt";
}

}  // namespace ast
