#include "string_utils.h"
#include <sstream>

namespace core::utils {

std::vector<String> StringUtils::split(const String &str, char delimiter) {
  std::vector<String> tokens;
  std::istringstream stream(str);
  String token;

  // Extract tokens using getline with specified delimiter
  while (std::getline(stream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

String StringUtils::join(const std::vector<String> &parts,
                         const String &delimiter) {
  // Handle empty input
  if (parts.empty()) {
    return "";
  }

  // Build result string efficiently using ostringstream
  std::ostringstream result;
  result << parts[0];

  // Append remaining parts with delimiter
  for (size_t i = 1; i < parts.size(); ++i) {
    result << delimiter << parts[i];
  }
  return result.str();
}

String StringUtils::trim(const String &str) {
  // Find first non-whitespace character
  auto first = str.find_first_not_of(" \t\n\r");
  if (first == String::npos) {
    return "";
  }

  // Find last non-whitespace character
  auto last = str.find_last_not_of(" \t\n\r");

  // Return substring between first and last non-whitespace
  return str.substr(first, last - first + 1);
}

String StringUtils::replace(String str, const String &from, const String &to) {
  size_t pos = 0;

  // Replace all occurrences of the substring
  while ((pos = str.find(from, pos)) != String::npos) {
    str.replace(pos, from.length(), to);
    pos += to.length();
  }
  return str;
}

bool StringUtils::startsWith(const String &str, const String &prefix) {
  // Check if string is long enough and starts with prefix
  return str.length() >= prefix.length() &&
         str.compare(0, prefix.length(), prefix) == 0;
}

bool StringUtils::endsWith(const String &str, const String &suffix) {
  // Check if string is long enough and ends with suffix
  return str.length() >= suffix.length() &&
         str.compare(str.length() - suffix.length(), suffix.length(), suffix) ==
             0;
}

String StringUtils::unescapeStringLiteral(const String &lexeme) {
  String body = lexeme;
  if (body.size() >= 2 && body.front() == '"' && body.back() == '"') {
    body = body.substr(1, body.size() - 2);
  }

  auto hexValue = [](char c) -> int {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
  };
  // Reads exactly `count` hex digits at body[pos]; -1 if they are not there.
  auto readHex = [&](size_t pos, size_t count) -> long {
    if (pos + count > body.size()) return -1;
    long value = 0;
    for (size_t k = 0; k < count; ++k) {
      int d = hexValue(body[pos + k]);
      if (d < 0) return -1;
      value = value * 16 + d;
    }
    return value;
  };

  String out;
  out.reserve(body.size());
  for (size_t i = 0; i < body.size(); ++i) {
    if (body[i] != '\\' || i + 1 >= body.size()) {
      out += body[i];
      continue;
    }
    char c = body[++i];
    switch (c) {
      case 'n': out += '\n'; break;
      case 'r': out += '\r'; break;
      case 't': out += '\t'; break;
      case '0': out += '\0'; break;
      case 'x': {
        long v = readHex(i + 1, 2);
        if (v < 0) { out += "\\x"; break; }
        out += static_cast<char>(v);
        i += 2;
        break;
      }
      case 'u': {
        long v = readHex(i + 1, 4);
        if (v < 0) { out += "\\u"; break; }
        i += 4;
        if (v < 0x80) {
          out += static_cast<char>(v);
        } else if (v < 0x800) {
          out += static_cast<char>(0xC0 | (v >> 6));
          out += static_cast<char>(0x80 | (v & 0x3F));
        } else {
          out += static_cast<char>(0xE0 | (v >> 12));
          out += static_cast<char>(0x80 | ((v >> 6) & 0x3F));
          out += static_cast<char>(0x80 | (v & 0x3F));
        }
        break;
      }
      default: out += c; break;  // \\ \' \" and unknown escapes
    }
  }
  return out;
}

} // namespace core::utils