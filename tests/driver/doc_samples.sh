#!/usr/bin/env bash
# Every ```tspp code sample in README.md, ROADMAP.md, CONTRIBUTING.md and
# docs/*.md must compile and link with the current compiler (samples of
# planned syntax are fenced ```tspp-planned and skipped). See
# scripts/check_doc_samples.sh.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export TSPP="${TSPP:-$ROOT/build/src/tspp}"
# run.sh may pass a relative TSPP; the checker changes directory.
[[ "$TSPP" == /* ]] || TSPP="$ROOT/$TSPP"

out="$("$ROOT/scripts/check_doc_samples.sh" 2>&1)" || {
  echo "FAIL: doc samples"
  grep -v '^ok ' <<<"$out"
  exit 1
}
