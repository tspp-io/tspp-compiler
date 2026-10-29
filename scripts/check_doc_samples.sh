#!/usr/bin/env bash
# Compile every tspp code sample in the docs with the current compiler.
#
# Usage: scripts/check_doc_samples.sh [file.md ...]
#   (default) README.md, ROADMAP.md, CONTRIBUTING.md and docs/*.md
#   TSPP=<path>   compiler to use (default: build/src/tspp)
#
# A sample is a fenced block opened with ```tspp. Each one is written to its
# own file and built with `tspp -o` (parse, check, codegen, IR verification and
# link; the program is not run). A sample without `main` is fine: codegen adds
# one.
#
# Samples that show planned, not-yet-working syntax must be fenced as
# ```tspp-planned instead; they are skipped. Blocks fenced as ```typescript or
# ```ts are rejected, so every tspp-looking sample is explicitly one or the
# other.
#
# Exit status: 0 if every sample builds, 1 otherwise.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
# Sanitizer leak reports from a Debug compiler are not what this checks.
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"

if [[ ! -x "$TSPP" ]]; then
  echo "check_doc_samples: compiler not found at $TSPP (run ./build.sh or set TSPP)" >&2
  exit 1
fi

if [[ $# -gt 0 ]]; then
  files=("$@")
else
  files=()
  for f in "$ROOT"/README.md "$ROOT"/ROADMAP.md "$ROOT"/CONTRIBUTING.md "$ROOT"/docs/*.md; do
    [[ -f "$f" ]] && files+=("$f")
  done
fi

work="$(mktemp -d "${TMPDIR:-/tmp}/tspp-docsamples.XXXXXX")"
trap 'rm -rf "$work"' EXIT

total=0
failed=0
skipped=0

for doc in "${files[@]}"; do
  rel="${doc#"$ROOT"/}"
  # Split the doc into one file per sample: <work>/<n>.tspp, and record
  # "<n> <line>" (line of the opening fence) in <work>/index. Rejected fences
  # are recorded as "BAD <line> <lang>"; planned ones as "PLANNED <line>".
  : >"$work/index"
  awk -v dir="$work" -v index_file="$work/index" -v start="$total" '
    BEGIN { n = start; inblock = 0; mode = "" }
    {
      line = $0
      if (!inblock && match(line, /^[ \t]*```[A-Za-z0-9_-]*[ \t]*$/)) {
        lang = line
        sub(/^[ \t]*```/, "", lang); sub(/[ \t]*$/, "", lang)
        indent = line; sub(/```.*/, "", indent)
        inblock = 1
        if (lang == "tspp") {
          n++; mode = "sample"; out = dir "/" n ".tspp"
          printf "" > out
          print n, NR >> index_file
        } else if (lang == "tspp-planned") {
          mode = "skip"; print "PLANNED", NR >> index_file
        } else if (lang == "typescript" || lang == "ts") {
          mode = "skip"; print "BAD", NR, lang >> index_file
        } else {
          mode = "skip"
        }
        next
      }
      if (inblock && match(line, /^[ \t]*```[ \t]*$/)) {
        if (mode == "sample") close(out)
        inblock = 0; mode = ""; next
      }
      if (inblock && mode == "sample") {
        if (indent != "" && index(line, indent) == 1) line = substr(line, length(indent) + 1)
        print line >> out
      }
    }
  ' "$doc"

  while read -r a b c; do
    case "$a" in
      PLANNED)
        skipped=$((skipped + 1))
        ;;
      BAD)
        failed=$((failed + 1))
        echo "FAIL $rel:$b: \`\`\`$c block; fence tspp samples as \`\`\`tspp (compiled) or \`\`\`tspp-planned (skipped)"
        ;;
      *)
        total=$((total + 1))
        sample="$work/$a.tspp"
        if (cd "$work" && "$TSPP" -o "$work/$a.exe" "$sample") >"$work/$a.out" 2>&1; then
          echo "ok   $rel:$b"
        else
          failed=$((failed + 1))
          echo "FAIL $rel:$b"
          grep -v -e '^==' -e 'Sanitizer' "$work/$a.out" | sed 's/^/     /' | head -n 8
        fi
        ;;
    esac
  done <"$work/index"
done

echo "Doc samples: $total compiled, $failed failed, $skipped planned (skipped)"
[[ $failed -eq 0 ]]
