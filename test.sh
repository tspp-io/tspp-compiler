#!/usr/bin/env bash
# Thin wrapper kept for old docs and habits: compile and run a .tspp file with
# the driver. All IR handling (void main wrapper, IR verification, linking)
# lives in the driver; see `build/src/tspp --help`.
#
# Usage: ./test.sh file.tspp [tspp options...] [-- program-args...]
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TSPP="${TSPP:-$ROOT/build/src/tspp}"
if [[ $# -eq 0 ]]; then
  echo "usage: $0 file.tspp [tspp options...]" >&2
  exit 2
fi
export LSAN_OPTIONS="${LSAN_OPTIONS:-suppressions=$ROOT/lsan.supp:report_objects=0:exitcode=0}"
exec "$TSPP" "$@"
