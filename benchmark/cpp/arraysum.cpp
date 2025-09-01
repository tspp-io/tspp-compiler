#include <vector>
int main() {
  int n = 5000000;
  std::vector<int> a(n);
  for (int i = 0; i < n; ++i) a[i] = i;
  int s = 0;
  for (int i = 0; i < n; ++i) s += a[i];
  return s;
}
