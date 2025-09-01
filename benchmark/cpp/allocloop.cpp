#include <vector>
int main() {
  int n = 100000;
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    std::vector<int> arr(10);
    arr[0] = i;
    sum += arr[0];
  }
  return sum;
}
