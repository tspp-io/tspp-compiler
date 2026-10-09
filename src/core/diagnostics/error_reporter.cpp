/*****************************************************************************
 * File: error_reporter.cpp
 * Description: The compiler's single diagnostics sink (see error_reporter.h).
 *****************************************************************************/

#include "error_reporter.h"

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace core {

namespace {
const char *const RED = "\033[1;31m";
const char *const MAGENTA = "\033[1;35m";
const char *const CYAN = "\033[1;36m";
const char *const GREEN = "\033[1;32m";
const char *const BOLD = "\033[1m";
const char *const RESET = "\033[0m";
}  // namespace

ErrorReporter &diagnostics() {
  static ErrorReporter reporter;
  return reporter;
}

ErrorReporter::ErrorReporter()
    : out_(&std::cerr), color_(isatty(STDERR_FILENO) != 0) {}

void ErrorReporter::error(const Location &location, const String &message) {
  report(Diagnostic::Severity::Error, location, message);
}

void ErrorReporter::warning(const Location &location, const String &message) {
  report(Diagnostic::Severity::Warning, location, message);
}

void ErrorReporter::note(const Location &location, const String &message) {
  report(Diagnostic::Severity::Note, location, message);
}

void ErrorReporter::internalError(const String &message) {
  report(Diagnostic::Severity::Fatal, Location(), message);
}

void ErrorReporter::registerSource(const String &filename,
                                   const String &content) {
  sources_[filename] = content;
}

void ErrorReporter::clear() {
  diagnostics_.clear();
  sources_.clear();
  errorCount_ = 0;
}

void ErrorReporter::setOutput(std::ostream *out) {
  out_ = out;
}

void ErrorReporter::report(Diagnostic::Severity severity,
                           const Location &location, const String &message) {
  diagnostics_.emplace_back(severity, location, message);
  bool isError = severity == Diagnostic::Severity::Error ||
                 severity == Diagnostic::Severity::Fatal;
  if (isError) ++errorCount_;
  if (!out_) return;
  if (isError && errorCount_ > kMaxPrintedErrors) return;
  print(diagnostics_.back());
  if (isError && errorCount_ == kMaxPrintedErrors) {
    *out_ << "tspp: too many errors emitted, stopping now\n";
  }
}

bool ErrorReporter::sourceLine(const String &filename, unsigned line,
                               String &out) {
  if (filename.empty() || line == 0) return false;
  auto it = sources_.find(filename);
  if (it == sources_.end()) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    std::stringstream ss;
    ss << file.rdbuf();
    it = sources_.emplace(filename, ss.str()).first;
  }
  std::istringstream in(it->second);
  String current;
  for (unsigned n = 1; std::getline(in, current); ++n) {
    if (n == line) {
      if (!current.empty() && current.back() == '\r') current.pop_back();
      out = current;
      return true;
    }
  }
  return false;
}

void ErrorReporter::print(const Diagnostic &diag) {
  std::ostream &os = *out_;
  const char *color = RED;
  const char *label = "error";
  switch (diag.severity) {
    case Diagnostic::Severity::Error:
      break;
    case Diagnostic::Severity::Warning:
      color = MAGENTA;
      label = "warning";
      break;
    case Diagnostic::Severity::Note:
      color = CYAN;
      label = "note";
      break;
    case Diagnostic::Severity::Fatal:
      label = "internal compiler error";
      break;
  }

  String where = diag.location.toString();
  if (where.empty()) where = "tspp";
  if (color_) os << BOLD;
  os << where << ": ";
  if (color_) os << color;
  os << label << ": ";
  if (color_) os << RESET << BOLD;
  os << diag.message;
  if (color_) os << RESET;
  os << "\n";

  // Source line with a caret under the column.
  String text;
  if (diag.location.getColumn() > 0 &&
      sourceLine(diag.location.getFilename(), diag.location.getLine(), text)) {
    String caret;
    unsigned col = 1;
    for (char c : text) {
      if (col >= diag.location.getColumn()) break;
      // Columns count a tab as 4 (LexerState::advance)
      if (c == '\t') {
        caret += '\t';
        col += 4;
      } else {
        caret += ' ';
        ++col;
      }
    }
    os << "    " << text << "\n    " << caret;
    if (color_) os << GREEN;
    os << "^";
    if (color_) os << RESET;
    os << "\n";
  }
}

}  // namespace core
