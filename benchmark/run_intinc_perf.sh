#!/usr/bin/env bash
# run_intinc_perf.sh: Run perf+flamegraph for intinc.tspp
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BENCH_DIR="$REPO_ROOT/benchmark"
TSPP_BIN="$REPO_ROOT/src/tspp"
INTINC_SRC="$BENCH_DIR/intinc.tspp"
PERF_DATA="$REPO_ROOT/perf.data"
PERF_FOLDED="$REPO_ROOT/perf.folded"
PERF_SVG="$REPO_ROOT/perf.svg"

if [[ ! -x "$TSPP_BIN" ]]; then
  echo "Error: tspp binary not found at $TSPP_BIN"
  exit 1
fi
if [[ ! -f "$INTINC_SRC" ]]; then
  echo "Error: intinc.tspp not found at $INTINC_SRC"
  exit 1
fi

# 1. perf stat
perf stat -e instructions,cycles,cache-misses,branch-misses -- "$TSPP_BIN" "$INTINC_SRC"

# 2. perf record
perf record -F200 -g -e cycles:u -o "$PERF_DATA" -- "$TSPP_BIN" "$INTINC_SRC"

# 3. perf script + stackcollapse
perf script -i "$PERF_DATA" | "$REPO_ROOT/FlameGraph/stackcollapse-perf.pl" > "$PERF_FOLDED"

# 4. flamegraph
"$REPO_ROOT/FlameGraph/flamegraph.pl" "$PERF_FOLDED" > "$PERF_SVG"

echo "Flamegraph generated: $PERF_SVG"
