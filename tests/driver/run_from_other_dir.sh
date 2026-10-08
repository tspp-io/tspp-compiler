#!/usr/bin/env bash
# The driver must work from any working directory: it finds libcore.a next to
# its own executable (not ./build/src/), resolves imports relative to the
# source file, and leaves nothing behind in the CWD.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"

work="$(mktemp -d "${TMPDIR:-/tmp}/tspp-otherdir.XXXXXX")"
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/cwd" "$work/src/lib"

cat >"$work/src/lib/helper.tspp" <<'TSPP'
function helperValue(): int {
  return 41;
}
TSPP
cat >"$work/src/main.tspp" <<'TSPP'
import Helper from "./lib/helper";
function main(): int {
  console.log("value: ", helperValue() + 1);
  return 0;
}
TSPP

fail() { echo "FAIL: $*"; exit 1; }

# 1. Relative source path from an unrelated CWD (no ./build there).
out="$(cd "$work/cwd" && "$TSPP" ../src/main.tspp 2>"$work/err")" ||
  fail "compile+run from another directory exited $?: $(grep -v Sanitizer "$work/err" | head -n 5)"
[[ "$out" == "value: 42" ]] || fail "unexpected output: '$out'"

# 2. -o with an absolute source path, run the executable from elsewhere.
(cd "$work/cwd" && "$TSPP" -o prog "$work/src/main.tspp" 2>"$work/err") ||
  fail "-o build exited $?: $(grep -v Sanitizer "$work/err" | head -n 5)"
out="$(cd / && "$work/cwd/prog")" || fail "built program exited $?"
[[ "$out" == "value: 42" ]] || fail "unexpected output from -o binary: '$out'"

# 3. Nothing but the requested output was written into the CWD.
leftover="$(cd "$work/cwd" && ls -A | grep -v '^prog$' || true)"
[[ -z "$leftover" ]] || fail "driver left files in the CWD: $leftover"
leftover="$(cd "$work/src" && ls -A | grep -v -E '^(main\.tspp|lib)$' || true)"
[[ -z "$leftover" ]] || fail "driver left files next to the source: $leftover"

echo "PASS: tspp works from another directory"
