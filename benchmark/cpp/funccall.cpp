int inc(int v) {
  return v + 1;
}
int main() {
  int x = 0;
  for (int i = 0; i < 5000000; ++i) {
    x = inc(x);
  }
  return x;
}
