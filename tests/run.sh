#!/usr/bin/env bash
# Test runner for tspp.
#
# Usage: tests/run.sh [substring...]
#   With arguments, only tests whose path contains one of the substrings run.
#
# Golden tests: every tests/**/*.tspp (except tests/bench/). The compiler and
# the program run with a fresh temp working directory; the source is compiled
# in place, so relative imports resolve.
#   - kind "run" (default): `tspp -o <exe> <file>` exits 0, then <exe> runs;
#     its stdout must equal <name>.expected and its exit status must equal the
#     "// exit: <n>" header (default 0);
#   - "// test: parse-only" header: `tspp --emit-llvm` exits 0 and the IR
#     passes llvm-as (the program is not linked or run);
#   - "// test: compile-error" header: `tspp -o <exe> <file>` exits non-zero;
#     every "// error: <text>" header line must appear in stderr;
#   - <name>.xfail marks a known failure (first line = reason). A failing xfail
#     test is XFAIL; a passing one is XPASS and counts as a failure.
# Only stdout is compared: ASan/LSan noise on stderr is ignored.
#
# Driver tests: every tests/driver/*.sh is run with TSPP set; exit 0 = PASS.
#
# Environment overrides:
#   TSPP            compiler binary          (default: <repo>/build/src/tspp)
#   LLVM_AS         llvm-as binary           (default: llvm-as)
#   TSPP_TEST_TIMEOUT seconds per step       (default: 20)
#   KEEP_TMP=1      keep per-test temp dirs and print their paths
# The driver finds clang, llvm-as and libcore.a itself (see `tspp --help`).
set -euo pipefail

TESTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$TESTS_DIR")"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
LLVM_AS="${LLVM_AS:-llvm-as}"
TIMEOUT="${TSPP_TEST_TIMEOUT:-20}"
KEEP_TMP="${KEEP_TMP:-0}"
export TSPP TSPP_TEST_TIMEOUT="$TIMEOUT"

# The compiler is an ASan build. Keep leak reports from changing exit codes;
# stderr is not compared anyway.
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"
export LSAN_OPTIONS="${LSAN_OPTIONS:-suppressions=$ROOT/lsan.supp:report_objects=0:exitcode=0}"

if [[ ! -x "$TSPP" ]]; then
  echo "error: compiler not found at $TSPP (run ./build.sh first)" >&2
  exit 2
fi
command -v "$LLVM_AS" >/dev/null || { echo "error: $LLVM_AS not on PATH" >&2; exit 2; }
command -v timeout >/dev/null || { echo "error: timeout(1) not found" >&2; exit 2; }

TMP_ROOT="$(mktemp -d "${TMPDIR:-/tmp}/tspp-tests.XXXXXX")"
cleanup() { [[ "$KEEP_TMP" == 1 ]] || rm -rf "$TMP_ROOT"; }
trap cleanup EXIT

pass=0 fail=0 xfail=0 xpass=0
failed_tests=()

# header_value <file> <key>: values of "// <key>: <value>" lines in the first 20 lines
header_values() {
  head -n 20 "$1" | sed -n "s|^[[:space:]]*//[[:space:]]*$2:[[:space:]]*||p" | sed 's/[[:space:]]*$//'
}

# stderr_head <file>: first lines of a stderr capture, without sanitizer noise
stderr_head() { grep -v -E 'Sanitizer|^$' "$1" | head -n 5 || true; }

# run_one <test.tspp relative to tests/> ; sets RESULT_OK (0/1) and RESULT_MSG
run_one() {
  local rel="$1"
  local src="$TESTS_DIR/$rel"
  local stem work rc kind expected want_exit
  stem="${src%.tspp}"
  work="$TMP_ROOT/$(echo "$rel" | tr '/' '_')"
  mkdir -p "$work"
  [[ "$KEEP_TMP" == 1 ]] && echo "  [tmp] $work"

  kind="$(header_values "$src" test | head -n 1)"
  kind="${kind:-run}"
  RESULT_OK=0
  RESULT_MSG=""

  rc=0
  if [[ "$kind" == parse-only ]]; then
    (cd "$work" && timeout "$TIMEOUT" "$TSPP" --emit-llvm -o "$work/out.ll" "$src" \
      >"$work/.stdout" 2>"$work/.stderr") || rc=$?
  else
    (cd "$work" && timeout "$TIMEOUT" "$TSPP" -o "$work/exe" "$src" \
      >"$work/.stdout" 2>"$work/.stderr") || rc=$?
  fi
  if [[ $rc -eq 124 ]]; then
    RESULT_MSG="compiler timed out after ${TIMEOUT}s"
    return
  fi

  case "$kind" in
    parse-only)
      if [[ $rc -ne 0 ]]; then
        RESULT_MSG="compiler exited $rc; stderr: $(stderr_head "$work/.stderr")"
      elif [[ ! -s "$work/out.ll" ]]; then
        RESULT_MSG="no IR written"
      elif ! "$LLVM_AS" "$work/out.ll" -o /dev/null 2>"$work/.llvm-as"; then
        RESULT_MSG="invalid IR: $(head -n 3 "$work/.llvm-as")"
      else
        RESULT_OK=1
      fi
      ;;
    compile-error)
      if [[ $rc -eq 0 ]]; then
        RESULT_MSG="expected a compile error, but the compiler exited 0"
        return
      fi
      local want missing=""
      while IFS= read -r want; do
        [[ -z "$want" ]] && continue
        grep -qF -- "$want" "$work/.stderr" || missing+="'$want' "
      done < <(header_values "$src" error)
      if [[ -n "$missing" ]]; then
        RESULT_MSG="compiler exited $rc but stderr lacks: $missing"
      else
        RESULT_OK=1
      fi
      ;;
    run)
      expected="$stem.expected"
      if [[ ! -f "$expected" ]]; then
        RESULT_MSG="missing $(basename "$expected")"
        return
      fi
      if [[ $rc -ne 0 ]]; then
        RESULT_MSG="compiler exited $rc; stderr: $(stderr_head "$work/.stderr")"
        return
      fi
      rc=0
      (cd "$work" && timeout "$TIMEOUT" "$work/exe" >"$work/.actual" 2>"$work/.run-stderr") || rc=$?
      if [[ $rc -eq 124 ]]; then
        RESULT_MSG="program timed out after ${TIMEOUT}s"
        return
      fi
      if ! diff -u "$expected" "$work/.actual" >"$work/.diff"; then
        RESULT_MSG="stdout differs from $(basename "$expected"):"$'\n'"$(tail -n +3 "$work/.diff" | head -n 20)"
        return
      fi
      want_exit="$(header_values "$src" exit | head -n 1)"
      want_exit="${want_exit:-0}"
      if [[ $rc -ne $want_exit ]]; then
        RESULT_MSG="program exited $rc, expected $want_exit; stderr: $(stderr_head "$work/.run-stderr")"
        return
      fi
      RESULT_OK=1
      ;;
    *)
      RESULT_MSG="unknown test kind '$kind'"
      ;;
  esac
}

# run_script <driver test .sh relative to tests/> ; sets RESULT_OK and RESULT_MSG
run_script() {
  local rel="$1" work rc=0
  work="$TMP_ROOT/$(echo "$rel" | tr '/' '_')"
  mkdir -p "$work"
  TMPDIR="$work" bash "$TESTS_DIR/$rel" >"$work/.out" 2>&1 || rc=$?
  if [[ $rc -eq 0 ]]; then
    RESULT_OK=1; RESULT_MSG=""
  else
    RESULT_OK=0; RESULT_MSG="exited $rc:"$'\n'"$(grep -v -E 'Sanitizer|^$' "$work/.out" | tail -n 10)"
  fi
}

mapfile -t all_tests < <(cd "$TESTS_DIR" && {
  find . -name '*.tspp' -not -path './bench/*'
  find ./driver -name '*.sh'
} | sed 's|^\./||' | LC_ALL=C sort)

for rel in "${all_tests[@]}"; do
  if [[ $# -gt 0 ]]; then
    match=0
    for pat in "$@"; do [[ "$rel" == *"$pat"* ]] && match=1; done
    [[ $match -eq 1 ]] || continue
  fi
  case "$rel" in
    *.sh) run_script "$rel"; xfail_file="$TESTS_DIR/${rel%.sh}.xfail" ;;
    *)    run_one "$rel";    xfail_file="$TESTS_DIR/${rel%.tspp}.xfail" ;;
  esac
  if [[ -f "$xfail_file" ]]; then
    reason="$(head -n 1 "$xfail_file")"
    if [[ $RESULT_OK -eq 1 ]]; then
      echo "XPASS $rel  (marked xfail: $reason; remove the .xfail file)"
      xpass=$((xpass + 1)); failed_tests+=("$rel (XPASS)")
    else
      echo "XFAIL $rel  ($reason)"
      xfail=$((xfail + 1))
    fi
  elif [[ $RESULT_OK -eq 1 ]]; then
    echo "PASS  $rel"
    pass=$((pass + 1))
  else
    echo "FAIL  $rel: $RESULT_MSG"
    fail=$((fail + 1)); failed_tests+=("$rel")
  fi
done

echo
echo "Summary: $pass PASS, $fail FAIL, $xfail XFAIL, $xpass XPASS"
if [[ ${#failed_tests[@]} -gt 0 ]]; then
  printf '  failed: %s\n' "${failed_tests[@]}"
  exit 1
fi
exit 0
