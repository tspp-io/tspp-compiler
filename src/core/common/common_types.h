/*****************************************************************************
 * File: common_types.h
 * Description: Core type definitions and utilities used throughout the compiler
 *
 * Contains:
 * - Basic type aliases
 * - Source location tracking
 * - Value storage and manipulation
 * - Error handling and result types
 *****************************************************************************/

#pragma once
#include <sstream>
#include <string>

namespace core {

/*****************************************************************************
 * Basic Type Aliases
 *****************************************************************************/
using Int = int;
using Float = float;
using Bool = bool;
using String = std::string;

/*****************************************************************************
 * Source Location
 *
 * A plain (file, line, column) triple. Lines and columns are 1-based; 0 means
 * "unknown". Cheap to copy: every token carries one. The source line shown
 * under a diagnostic is looked up by core::ErrorReporter, not stored here.
 *****************************************************************************/
class Location {
 public:
  Location(unsigned int ln = 0, unsigned int col = 0, std::string file = "")
      : line_(ln), column_(col), filename_(std::move(file)) {}

  unsigned int getLine() const {
    return line_;
  }
  unsigned int getColumn() const {
    return column_;
  }
  const std::string &getFilename() const {
    return filename_;
  }
  bool isValid() const {
    return line_ > 0;
  }

  bool operator==(const Location &other) const {
    return filename_ == other.filename_ && line_ == other.line_ &&
           column_ == other.column_;
  }

  // "file:line:col" (parts that are unknown are left out).
  std::string toString() const {
    std::stringstream output;
    output << filename_;
    if (line_ > 0) {
      if (!filename_.empty()) output << ":";
      output << line_;
      if (column_ > 0) output << ":" << column_;
    }
    return output.str();
  }

 private:
  unsigned int line_;     // Line number (1-based, 0 = unknown)
  unsigned int column_;   // Column number (1-based, 0 = unknown)
  std::string filename_;  // Source file name
};

/*****************************************************************************
 * Value Storage
 *****************************************************************************/
union LiteralValue {
  Int int_value;
  Float float_value;
  Bool bool_value;

  // Value constructors
  LiteralValue() : int_value(0) {}
  explicit LiteralValue(Int v) : int_value(v) {}
  explicit LiteralValue(Float v) : float_value(v) {}
  explicit LiteralValue(Bool v) : bool_value(v) {}
};

/*****************************************************************************
 * Error Handling
 *****************************************************************************/
struct Error {
  String message;
  Location location;

  Error(String msg, Location loc)
      : message(std::move(msg)), location(std::move(loc)) {}

  String toString() const {
    return location.toString() + ": " + message;
  }
};

/*****************************************************************************
 * Result Type
 *****************************************************************************/
template <typename T>
class Result {
 private:
  T value_;
  Error error_;
  Bool has_error_;

 public:
  // Constructors for success and failure cases
  Result(T value) : value_(std::move(value)), has_error_(false) {}
  Result(Error error) : error_(std::move(error)), has_error_(true) {}

  // Status checks
  Bool hasError() const {
    return has_error_;
  }
  const T &getValue() const {
    return value_;
  }
  const Error &getError() const {
    return error_;
  }

  // Implicit bool conversion for quick error checks
  operator bool() const {
    return !has_error_;
  }
};

}  // namespace core