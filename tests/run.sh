#!/usr/bin/env bash
# Golden-output test runner for tspp.
#
# Usage: tests/run.sh [substring...]
#   With arguments, only tests whose path contains one of the substrings run.
#
# For every tests/**/*.tspp (except tests/bench/):
#   - the compiler runs in a fresh temp working directory (the driver writes
#     <file>.ll, temp.bc and temp_exec into its CWD and links ./build/src/libcore.a,
#     so the temp dir gets a copy of the test's directory and a `build` symlink);
#   - kind "run" (default): compiler exits 0, temp_exec was linked, and stdout
#     (minus the driver's "Output: " banner) equals <name>.expected;
#   - "// test: parse-only" header: compiler exits 0 and the emitted IR passes
#     llvm-as (the program is still linked and run by the driver; output ignored);
#   - "// test: compile-error" header: compiler exits non-zero; every
#     "// error: <text>" header line must appear in stderr;
#   - <name>.xfail marks a known failure (first line = reason). A failing xfail
#     test is XFAIL; a passing one is XPASS and counts as a failure.
# Only stdout is compared: ASan/LSan noise on stderr is ignored.
#
# Environment overrides:
#   TSPP            compiler binary          (default: <repo>/build/src/tspp)
#   TSPP_BUILD_DIR  dir containing src/libcore.a (default: <repo>/build)
#   LLVM_AS         llvm-as binary           (default: llvm-as)
#   TSPP_TEST_TIMEOUT seconds per test       (default: 20)
#   KEEP_TMP=1      keep per-test temp dirs and print their paths
set -euo pipefail

TESTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$TESTS_DIR")"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
TSPP_BUILD_DIR="${TSPP_BUILD_DIR:-$ROOT/build}"
LLVM_AS="${LLVM_AS:-llvm-as}"
TIMEOUT="${TSPP_TEST_TIMEOUT:-20}"
KEEP_TMP="${KEEP_TMP:-0}"

# The compiler and the compiled programs are ASan builds. Keep leak reports
# from changing exit codes; their stderr is not compared anyway.
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"
export LSAN_OPTIONS="${LSAN_OPTIONS:-suppressions=$ROOT/lsan.supp:report_objects=0:exitcode=0}"

if [[ ! -x "$TSPP" ]]; then
  echo "error: compiler not found at $TSPP (run ./build.sh first)" >&2
  exit 2
fi
if [[ ! -f "$TSPP_BUILD_DIR/src/libcore.a" ]]; then
  echo "error: runtime not found at $TSPP_BUILD_DIR/src/libcore.a" >&2
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

# run_one <test.tspp relative to tests/> ; sets RESULT_OK (0/1) and RESULT_MSG
run_one() {
  local rel="$1"
  local src="$TESTS_DIR/$rel"
  local dir base stem work rc kind expected actual
  dir="$(dirname "$rel")"
  base="$(basename "$rel")"
  stem="${src%.tspp}"
  work="$TMP_ROOT/$(echo "$rel" | tr '/' '_')"
  mkdir -p "$work"
  # Copy the test's directory so relative imports resolve, and expose the
  # runtime at ./build/src/libcore.a where the current driver looks for it.
  cp -R "$TESTS_DIR/$dir/." "$work/"
  ln -s "$TSPP_BUILD_DIR" "$work/build"
  [[ "$KEEP_TMP" == 1 ]] && echo "  [tmp] $work"

  rc=0
  (cd "$work" && timeout "$TIMEOUT" "$TSPP" "$base" >"$work/.stdout" 2>"$work/.stderr") || rc=$?

  kind="$(header_values "$src" test | head -n 1)"
  kind="${kind:-run}"
  RESULT_OK=0
  RESULT_MSG=""

  if [[ $rc -eq 124 ]]; then
    RESULT_MSG="timed out after ${TIMEOUT}s"
    return
  fi

  case "$kind" in
    parse-only)
      if [[ $rc -ne 0 ]]; then
        RESULT_MSG="compiler exited $rc; stderr: $(grep -v -E 'Sanitizer|^$' "$work/.stderr" | head -n 5)"
      elif [[ ! -s "$work/$base.ll" ]]; then
        RESULT_MSG="no IR written ($base.ll)"
      elif ! "$LLVM_AS" "$work/$base.ll" -o /dev/null 2>"$work/.llvm-as"; then
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
        RESULT_MSG="compiler exited $rc; stderr: $(grep -v -E 'Sanitizer|^$' "$work/.stderr" | head -n 5)"
        return
      fi
      if [[ ! -x "$work/temp_exec" ]]; then
        RESULT_MSG="no executable linked; stderr: $(grep -v -E 'Sanitizer|^$' "$work/.stderr" | head -n 5)"
        return
      fi
      # Drop the driver's "Output: " banner (first line) before comparing.
      actual="$work/.actual"
      sed '1{/^Output: $/d;}' "$work/.stdout" >"$actual"
      if diff -u "$expected" "$actual" >"$work/.diff"; then
        RESULT_OK=1
      else
        RESULT_MSG="stdout differs from $(basename "$expected"):"$'\n'"$(tail -n +3 "$work/.diff" | head -n 20)"
      fi
      ;;
    *)
      RESULT_MSG="unknown test kind '$kind'"
      ;;
  esac
}

mapfile -t all_tests < <(cd "$TESTS_DIR" && find . -name '*.tspp' -not -path './bench/*' | sed 's|^\./||' | LC_ALL=C sort)

for rel in "${all_tests[@]}"; do
  if [[ $# -gt 0 ]]; then
    match=0
    for pat in "$@"; do [[ "$rel" == *"$pat"* ]] && match=1; done
    [[ $match -eq 1 ]] || continue
  fi
  run_one "$rel"
  xfail_file="$TESTS_DIR/${rel%.tspp}.xfail"
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
