#!/usr/bin/env bash
# A failing build stage must make tspp exit non-zero and name the stage, and
# must never run a stale or planted executable.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"

work="$(mktemp -d "${TMPDIR:-/tmp}/tspp-linkfail.XXXXXX")"
trap 'rm -rf "$work"' EXIT
cd "$work"

cat >prog.tspp <<'TSPP'
function main(): int {
  console.log("should-not-run");
  return 0;
}
TSPP
# A planted binary at the pre-Phase-3 fixed output name must never run.
printf '#!/bin/sh\necho planted-binary-ran\n' >temp_exec
chmod +x temp_exec

fail() { echo "FAIL: $*"; exit 1; }

# expect_failure <stage text> <cmd...>
expect_failure() {
  local stage="$1"; shift
  local rc=0
  "$@" >out 2>err || rc=$?
  [[ $rc -ne 0 ]] || fail "'$*' exited 0 (stage: $stage)"
  grep -q "$stage" err || fail "stderr does not name the failing stage '$stage': $(grep -v Sanitizer err | head -n 3)"
  if grep -q -e should-not-run -e planted-binary-ran out; then
    fail "a program ran although the build failed"
  fi
}

# Link stage forced to fail.
expect_failure "link (clang) failed" env TSPP_CLANG=false "$TSPP" prog.tspp
expect_failure "link (clang) failed" env TSPP_CLANG=false "$TSPP" -o exe prog.tspp
[[ ! -e exe ]] || fail "-o output exists after a failed link"
# IR verification stage forced to fail.
expect_failure "IR verification (llvm-as) failed" env TSPP_LLVM_AS=false "$TSPP" prog.tspp
# Missing runtime library.
expect_failure "TSPP_RUNTIME_LIB" env TSPP_RUNTIME_LIB="$work/missing.a" "$TSPP" prog.tspp
# Missing clang.
expect_failure "not found" env TSPP_CLANG=no-such-clang-xyz "$TSPP" prog.tspp

echo "PASS: build failures exit non-zero and name the stage"
