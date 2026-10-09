#!/usr/bin/env bash
# REPL (`tspp` with no arguments, H2): each chunk is compiled, linked and run
# as a separate program through the normal driver path. A chunk is a line, or
# several lines until braces balance. Errors are reported and the REPL goes
# on; state does not carry over; nothing is written to the CWD.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"

work="$(mktemp -d "${TMPDIR:-/tmp}/tspp-repl.XXXXXX")"
trap 'rm -rf "$work"' EXIT
cd "$work"
mkdir cwd

fail() { echo "FAIL: $*"; echo "--- stdout"; cat out; echo "--- stderr"; cat err; exit 1; }

cat >input <<'TSPP'
console.log("hello from repl")

function main(): int {
  if (1 < 2) {
    console.log("block " + (2 + 3 * 4))
  }
  return 3
}
let x: int = 5;
console.log("x is " + x)
console.log("brace in a string {")
console.log("still running")
TSPP

rc=0
(cd cwd && "$TSPP" <../input >../out 2>../err) || rc=$?
[ "$rc" -eq 0 ] || fail "REPL exited with $rc"

grep -q 'each input is compiled and run as a separate program' out ||
  fail "banner does not say how the REPL works"
grep -q 'hello from repl' out || fail "single-line chunk did not run"
grep -q 'block 14' out || fail "multi-line chunk did not run as one program"
grep -q '\[exit status 3\]' out || fail "non-zero exit status not shown"
# `x` from an earlier chunk is not visible: a located error, then the REPL
# continues (a brace inside a string does not open a block).
grep -q "<repl>:1:23: error: use of undeclared identifier 'x'" err ||
  fail "expected an undeclared-identifier error for x"
grep -q 'brace in a string {' out || fail "brace in a string started a block"
grep -q 'still running' out || fail "REPL stopped after an error"
[ -z "$(ls -A cwd)" ] || fail "REPL wrote files into the CWD: $(ls -A cwd)"
echo "ok"
