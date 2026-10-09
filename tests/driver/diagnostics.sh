#!/usr/bin/env bash
# Diagnostics format and driver behaviour (Phase 6):
#   - warnings are printed as file:line:col: warning: ... and do not fail;
#   - errors exit 1 and leave no output file behind (-o, --emit-llvm);
#   - no ANSI color codes when stderr is not a terminal;
#   - errors stop being printed after 50, with a notice.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"

work="$(mktemp -d "${TMPDIR:-/tmp}/tspp-diag.XXXXXX")"
trap 'rm -rf "$work"' EXIT
cd "$work"

fail() { echo "FAIL: $*"; exit 1; }

# A warning does not fail the build.
cat >warn.tspp <<'TSPP'
function f(x: int): int {
  if (x > 0) {
    return 1;
  }
}
function main(): int {
  console.log(f(0));
  return 0;
}
TSPP
rc=0
"$TSPP" warn.tspp >out 2>err || rc=$?
[[ $rc -eq 0 ]] || fail "a warning made tspp exit $rc"
grep -qF "warn.tspp:1:10: warning: control can reach the end of non-void function 'f'" err ||
  fail "warning missing or unlocated: $(grep -v Sanitizer err | head -n 3)"
[[ "$(cat out)" == "0" ]] || fail "program output: $(cat out)"

# An error exits 1 and writes nothing.
cat >bad.tspp <<'TSPP'
function main(): int {
  let x: int = ;
  return 0;
}
TSPP
for args in "-o bad.exe" "--emit-llvm -o bad.ll" "--emit-llvm"; do
  rc=0
  # shellcheck disable=SC2086
  "$TSPP" $args bad.tspp >out 2>err || rc=$?
  [[ $rc -eq 1 ]] || fail "'tspp $args' on a bad program exited $rc, expected 1"
  grep -qF "bad.tspp:2:16: error: expected an expression, found ';'" err ||
    fail "error missing or unlocated: $(grep -v Sanitizer err | head -n 3)"
done
[[ ! -e bad.exe && ! -e bad.ll && ! -e bad.tspp.ll ]] || fail "output written for a bad program"

# No color escapes when stderr is a file.
if grep -q $'\033' err; then fail "ANSI escapes in non-terminal stderr"; fi

# Error cap.
{
  echo "function main(): int {"
  for i in $(seq 1 60); do echo "  let v$i: int = ;"; done
  echo "  return 0;"
  echo "}"
} >many.tspp
rc=0
"$TSPP" -o many.exe many.tspp >out 2>err || rc=$?
[[ $rc -eq 1 ]] || fail "many errors: exit $rc"
n="$(grep -c ': error: ' err || true)"
[[ "$n" -eq 50 ]] || fail "printed $n errors, expected 50"
grep -q "too many errors emitted" err || fail "no 'too many errors' notice"

echo "PASS: diagnostics format and exit status"
