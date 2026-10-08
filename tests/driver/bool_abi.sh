#!/usr/bin/env bash
# The runtime's tspp_bool_to_string takes a C int. The i1 the IR passes must
# be declared `zeroext`, or its upper bits are undefined (x86-64 SysV) and a
# false value can print "true".
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"

work="$(mktemp -d "${TMPDIR:-/tmp}/tspp-boolabi.XXXXXX")"
trap 'rm -rf "$work"' EXIT
cd "$work"

cat >prog.tspp <<'TSPP'
function main(): int {
  let b: boolean = false;
  console.log("b: " + b);
  return 0;
}
TSPP

"$TSPP" --emit-llvm -o prog.ll prog.tspp 2>/dev/null
if ! grep -q 'declare ptr @tspp_bool_to_string(i1 zeroext)' prog.ll; then
  echo "FAIL: tspp_bool_to_string is not declared with a zeroext i1 parameter"
  grep 'tspp_bool_to_string' prog.ll || true
  exit 1
fi
echo "ok"
