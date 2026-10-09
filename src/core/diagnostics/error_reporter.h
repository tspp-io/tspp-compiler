/*****************************************************************************
 * File: error_reporter.h
 * Description: The compiler's single diagnostics sink.
 *
 * Every stage (lexer, parser, import resolution, semantic analysis, codegen)
 * reports through core::diagnostics(). Diagnostics are printed to stderr as
 * they are reported, GCC style:
 *
 *   file:line:col: error: message
 *       source line
 *       ^
 *
 * The driver resets the sink before each compilation and stops after any
 * stage that reported an error.
 *****************************************************************************/

#pragma once
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "../common/common_types.h"

namespace core {

struct Diagnostic {
  enum class Severity {
    Error,    // Prevents compilation
    Warning,  // Potential issue, compilation continues
    Note,     // Extra information attached to the previous diagnostic
    Fatal     // Internal compiler error
  };

  Diagnostic(Severity sev, const Location &loc, String msg)
      : severity(sev), location(loc), message(std::move(msg)) {}

  Severity severity;
  Location location;
  String message;
};

class ErrorReporter {
 public:
  ErrorReporter();

  void error(const Location &location, const String &message);
  void warning(const Location &location, const String &message);
  void note(const Location &location, const String &message);
  // A bug in the compiler itself (e.g. invalid IR): always an error.
  void internalError(const String &message);

  // Source text used to print the offending line under a diagnostic. Files
  // that are not registered are read from disk on demand.
  void registerSource(const String &filename, const String &content);

  const std::vector<Diagnostic> &getDiagnostics() const {
    return diagnostics_;
  }
  bool hasErrors() const {
    return errorCount_ > 0;
  }
  int errorCount() const {
    return errorCount_;
  }

  // Forget all diagnostics and registered sources (one compilation each).
  void clear();

  // Where diagnostics are printed (default: std::cerr; nullptr: nowhere).
  void setOutput(std::ostream *out);
  void setColor(bool enabled) {
    color_ = enabled;
  }

  // After this many errors, further errors are counted but not printed.
  static constexpr int kMaxPrintedErrors = 50;

 private:
  std::vector<Diagnostic> diagnostics_;
  std::map<String, String> sources_;
  std::ostream *out_;
  bool color_;
  int errorCount_ = 0;

  void report(Diagnostic::Severity severity, const Location &location,
              const String &message);
  void print(const Diagnostic &diag);
  bool sourceLine(const String &filename, unsigned line, String &out);
};

// The process-wide diagnostics sink.
ErrorReporter &diagnostics();

}  // namespace core
