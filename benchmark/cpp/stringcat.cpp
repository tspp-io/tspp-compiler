#include <string>
int main() {
  std::string s = "";
  for (int i = 0; i < 100000; ++i) {
    s += "x";
  }
  return s.length();
}
