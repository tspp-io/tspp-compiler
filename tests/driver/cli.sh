#!/usr/bin/env bash
# Driver command line: --version, --help, usage errors, -o, --emit-llvm,
# --keep-temps, --asan, program arguments, and exit status propagation in the
# default compile-and-run mode.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"

work="$(mktemp -d "${TMPDIR:-/tmp}/tspp-cli.XXXXXX")"
trap 'rm -rf "$work"' EXIT
cd "$work"

fail() { echo "FAIL: $*"; exit 1; }
status() { local rc=0; "$@" >/dev/null 2>&1 || rc=$?; echo "$rc"; }

cat >exit7.tspp <<'TSPP'
function main(): int {
  console.log("exit7");
  return 7;
}
TSPP
cat >voidmain.tspp <<'TSPP'
function main(): void {
  console.log("void main");
}
TSPP

# --version / --help
version="$("$TSPP" --version)"
[[ "$version" =~ ^tspp\ [0-9]+\.[0-9]+\.[0-9]+$ ]] || fail "--version output: '$version'"
help="$("$TSPP" --help)"
[[ "$help" == *--emit-llvm* ]] || fail "--help does not list --emit-llvm"

# Usage errors exit 2; a missing file exits 1.
[[ $(status "$TSPP" --no-such-flag exit7.tspp) -eq 2 ]] || fail "unknown option should exit 2"
[[ $(status "$TSPP" -o) -eq 2 ]] || fail "-o without argument should exit 2"
[[ $(status "$TSPP" a.tspp b.tspp) -eq 2 ]] || fail "two inputs should exit 2"
[[ $(status "$TSPP" missing.tspp) -eq 1 ]] || fail "missing input should exit 1"

# Default mode runs the program and propagates its exit status; stdout is
# exactly the program's output (no banner).
rc=0; out="$("$TSPP" exit7.tspp 2>/dev/null)" || rc=$?
[[ $rc -eq 7 ]] || fail "default mode exit status $rc, expected 7"
[[ "$out" == "exit7" ]] || fail "default mode stdout: '$out'"

# main(): void is wrapped by codegen and exits 0.
rc=0; out="$("$TSPP" voidmain.tspp 2>/dev/null)" || rc=$?
[[ $rc -eq 0 && "$out" == "void main" ]] || fail "void main: rc=$rc out='$out'"

# -o builds without running.
rc=0; out="$("$TSPP" -o prog exit7.tspp 2>/dev/null)" || rc=$?
[[ $rc -eq 0 ]] || fail "-o exited $rc"
[[ -z "$out" ]] || fail "-o ran the program: '$out'"
[[ -x prog ]] || fail "-o did not write an executable"
rc=0; ./prog >/dev/null || rc=$?
[[ $rc -eq 7 ]] || fail "built program exit status $rc, expected 7"

# --emit-llvm writes <file>.ll (or -o) and nothing else.
"$TSPP" --emit-llvm exit7.tspp 2>/dev/null || fail "--emit-llvm exited $?"
[[ -s exit7.tspp.ll ]] || fail "--emit-llvm did not write exit7.tspp.ll"
"$TSPP" --emit-llvm -o custom.ll exit7.tspp 2>/dev/null || fail "--emit-llvm -o exited $?"
grep -q 'define i32 @main' custom.ll || fail "custom.ll has no main"

# --keep-temps keeps (and reports) the temp dir; without it nothing is kept.
"$TSPP" --keep-temps exit7.tspp >/dev/null 2>err || true
kept="$(sed -n 's/^tspp: kept temporary files in //p' err)"
[[ -n "$kept" && -f "$kept/program.ll" && -x "$kept/program" ]] || fail "--keep-temps: '$kept'"
rm -rf "$kept"

# --asan links the AddressSanitizer runtime; the default does not.
"$TSPP" -o plain exit7.tspp 2>/dev/null
"$TSPP" --asan -o asan exit7.tspp 2>/dev/null || fail "--asan build exited $?"
# (grep -c, not grep -q: an early-exiting grep would SIGPIPE nm under pipefail)
[[ $(nm plain 2>/dev/null | grep -c __asan_init) -eq 0 ]] || fail "default build is ASan-instrumented"
[[ $(nm asan 2>/dev/null | grep -c __asan_init) -gt 0 ]] || fail "--asan build lacks the ASan runtime"
rc=0; ./asan >/dev/null 2>&1 || rc=$?
[[ $rc -eq 7 ]] || fail "--asan program exit status $rc, expected 7"

echo "PASS: driver CLI"
