#!/usr/bin/env bash
# Regression test for C3 (KNOWLEDGE_TRANSFER §9.3): the driver must not pass
# the input filename through a shell.
#
# Creates a source file whose name contains shell metacharacters and a
# harmless payload (`touch pwned`), compiles and runs it, and fails if the
# payload ran (the marker file exists in the working directory). The program
# must also still compile and print its output, i.e. the name was treated
# literally.
#
# Usage: tests/driver/shell_injection.sh   (TSPP=<compiler> to override)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"

work="$(mktemp -d "${TMPDIR:-/tmp}/tspp-inject.XXXXXX")"
trap 'rm -rf "$work"' EXIT
# Pre-Phase-3 drivers link ./build/src/libcore.a relative to the CWD.
ln -s "$(dirname "$(dirname "$TSPP")")" "$work/build"

name='inj;touch pwned;x.tspp'
cat >"$work/$name" <<'TSPP'
function main(): int {
  console.log("injection-test-ok");
  return 0;
}
TSPP

rc=0
(cd "$work" && "$TSPP" "$name" >"$work/.stdout" 2>"$work/.stderr") || rc=$?

status=0
if [[ -e "$work/pwned" ]]; then
  echo "FAIL: shell payload in the file name was executed (marker 'pwned' created)"
  status=1
fi
if [[ $rc -ne 0 ]]; then
  echo "FAIL: compiler exited $rc; stderr:"
  grep -v -E 'Sanitizer|^$' "$work/.stderr" | head -n 5 || true
  status=1
elif ! grep -qx 'injection-test-ok' "$work/.stdout"; then
  echo "FAIL: program output missing; stdout:"
  head -n 5 "$work/.stdout"
  status=1
fi
[[ $status -eq 0 ]] && echo "PASS: file name with shell metacharacters treated literally"
exit $status
