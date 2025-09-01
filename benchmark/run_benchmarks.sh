#!/usr/bin/env bash
# generate_all_benchmarks.sh
# Run microbenchmarks for tspp / cpp / node / python and produce CSVs + merged CSV.
# Usage: ./benchmark/generate_all_benchmarks.sh
#
# Requirements:
#  - bash
#  - /usr/bin/time (for elapsed seconds)
#  - gcc/g++ (for building cpp benchmarks) - optional if you already have compiled bins
#  - node (optional)
#  - python3 (optional)
#  - perf (optional; if missing perf counters will be 0)
#
set -euo pipefail

# resolve script dir
SOURCE="${BASH_SOURCE[0]:-$0}"
SCRIPT_DIR="$(cd "$(dirname "$SOURCE")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BENCH_DIR="$SCRIPT_DIR"
OUT_DIR="$BENCH_DIR/output"
mkdir -p "$OUT_DIR"

# Configure paths
TSPP_BIN="$REPO_ROOT/build/src/tspp"   # adjust if your tspp binary lives elsewhere
CPP_SRC_DIR="$BENCH_DIR/cpp"
CPP_BUILD_DIR="$BENCH_DIR/cpp/build"
NODE_DIR="$BENCH_DIR/node"
PY_DIR="$BENCH_DIR/python"

# Benchmarks list and iteration counts (must match across languages)
BENCHS=(startup emptyloop intinc funccall arraysum allocloop stringcat)
declare -A NMAP
NMAP[startup]=1
NMAP[emptyloop]=30000000
NMAP[intinc]=30000000
NMAP[funccall]=5000000
NMAP[arraysum]=10000000
NMAP[allocloop]=100000
NMAP[stringcat]=100000

# Perf events (comma-separated for perf -e arg)
PERF_EVENTS="instructions,cycles,cache-misses,branch-misses"

# Output filenames
TSV_OUT="$OUT_DIR/results_tspp.csv"
CPP_OUT="$OUT_DIR/results_cpp.csv"
NODE_OUT="$OUT_DIR/results_node.csv"
PY_OUT="$OUT_DIR/results_py.csv"
MERGED_OUT="$OUT_DIR/merged_results.csv"

CSV_HEADER="Benchmark,Time(s),ns/op,Instructions,Cycles,CacheMisses,BranchMisses"

# Helper: compute ns/op
compute_nsop() {
  local t="$1"
  local n="$2"
  if [[ -z "$t" || "$t" == "0" || "$n" -le 0 ]]; then
    echo "0.00"
  else
    awk -v t="$t" -v n="$n" 'BEGIN{ printf("%.2f", (t/n)*1e9) }'
  fi
}

# Helper: run perf stat and parse counters (best-effort)
# args: cmd... (command to run)
run_perf_and_parse() {
  local perf_available=1
  command -v perf >/dev/null 2>&1 || perf_available=0
  local out_file
  out_file="$(mktemp)"
  local instr=0 cycles=0 cmiss=0 bmiss=0

  if [[ $perf_available -eq 1 ]]; then
    # perf prints CSV rows when -x, is used. We redirect stdout to /dev/null to avoid clutter.
    # perf prints to stderr
    perf stat -x, -e $PERF_EVENTS -- "${@}" >/dev/null 2> "$out_file" || true
    # parse CSV rows: <value>,<unit>,<event>,...
    get_val() {
      local key="$1"
      # exact match in field 3
      local v
      v=$(awk -F, -v ev="$key" '$3==ev {print $1; exit}' "$out_file" || true)
      if [[ -z "$v" ]]; then
        # fallback: substring match
        v=$(awk -F, -v ev="$key" 'index($0,ev){print $1; exit}' "$out_file" || true)
      fi
      # strip commas and non-digit/dot
      v=$(echo "$v" | tr -d ',' | sed 's/[^0-9.]//g')
      if [[ -z "$v" ]]; then
        echo "0"
      else
        echo "$v"
      fi
    }
    instr=$(get_val "instructions")
    cycles=$(get_val "cycles")
    cmiss=$(get_val "cache-misses")
    bmiss=$(get_val "branch-misses")
  else
    # perf missing => zero counters
    instr=0; cycles=0; cmiss=0; bmiss=0
  fi

  rm -f "$out_file"
  echo "$instr" "$cycles" "$cmiss" "$bmiss"
}

# Run bench for a generic command (cmd args...), returns elapsed seconds
# usage: run_and_time <cmd...>
run_and_time() {
  local timefile
  timefile="$(mktemp)"
  # Use /usr/bin/time to print elapsed seconds to file (-f "%e")
  /usr/bin/time -f "%e" -o "$timefile" -- "${@}" >/dev/null 2>&1 || true
  local t
  t=$(cat "$timefile" | tr -d ' \n\r')
  rm -f "$timefile"
  if [[ -z "$t" ]]; then
    t=0
  fi
  echo "$t"
}

# Create empty CSVs with header
echo "$CSV_HEADER" > "$TSV_OUT"
echo "$CSV_HEADER" > "$CPP_OUT"
echo "$CSV_HEADER" > "$NODE_OUT"
echo "$CSV_HEADER" > "$PY_OUT"

# --------- 1) TSPP benchmarks -----------
echo "== Running TSPP benchmarks =="
if [[ ! -x "$TSPP_BIN" ]]; then
  echo "Warning: TSPP binary not found or not executable at $TSPP_BIN"
  echo "Make sure to build tspp (cmake -DCMAKE_BUILD_TYPE=Release .. && make) and place binary at $TSPP_BIN"
fi

for b in "${BENCHS[@]}"; do
  src="$BENCH_DIR/$b.tspp"
  if [[ ! -f "$src" ]]; then
    echo "Skipping tspp/$b (missing $src)"
    continue
  fi
  echo " - tspp: $b"
  # time
  t=$(run_and_time "$TSPP_BIN" "$src")
  # perf
  read instr cycles cmiss bmiss < <(run_perf_and_parse "$TSPP_BIN" "$src")
  # ns/op
  N=${NMAP[$b]}
  nsop=$(compute_nsop "$t" "$N")
  printf "%s,%s,%s,%s,%s,%s,%s\n" "$b" "$t" "$nsop" "$instr" "$cycles" "$cmiss" "$bmiss" >> "$TSV_OUT"
done

# --------- 2) C++ benchmarks -----------
echo
echo "== Building C++ benchmarks (if sources exist) =="
if [[ -d "$CPP_SRC_DIR" ]]; then
  mkdir -p "$CPP_BUILD_DIR"
  for f in "$CPP_SRC_DIR"/*.cpp; do
    [[ -f "$f" ]] || continue
    bn=$(basename "$f" .cpp)
    out="$CPP_BUILD_DIR/$bn"
    echo "  compile $bn"
    # Try to use optimizations; if g++ not found this will fail and skip
    if command -v g++ >/dev/null 2>&1; then
      g++ -O3 -march=native -flto -pipe "$f" -o "$out" || g++ -O3 "$f" -o "$out"
    else
      echo "  g++ not found; skipping compile"
      break
    fi
  done

  echo "== Running C++ benchmarks =="
  for b in "${BENCHS[@]}"; do
    bin="$CPP_BUILD_DIR/$b"
    if [[ ! -x "$bin" ]]; then
      echo "Skipping cpp/$b (no binary $bin)"
      continue
    fi
    echo " - cpp: $b"
    t=$(run_and_time "$bin")
    read instr cycles cmiss bmiss < <(run_perf_and_parse "$bin")
    N=${NMAP[$b]}
    nsop=$(compute_nsop "$t" "$N")
    printf "%s,%s,%s,%s,%s,%s,%s\n" "$b" "$t" "$nsop" "$instr" "$cycles" "$cmiss" "$bmiss" >> "$CPP_OUT"
  done
else
  echo "No C++ sources at $CPP_SRC_DIR; skipping C++ bench"
fi

# --------- 3) Node benchmarks -----------
echo
echo "== Running Node benchmarks =="
if command -v node >/dev/null 2>&1 && [[ -d "$NODE_DIR" ]]; then
  for b in "${BENCHS[@]}"; do
    js="$NODE_DIR/$b.js"
    if [[ ! -f "$js" ]]; then
      echo "Skipping node/$b (file missing)"
      continue
    fi
    echo " - node: $b"
    t=$(run_and_time node "$js")
    read instr cycles cmiss bmiss < <(run_perf_and_parse node "$js")
    N=${NMAP[$b]}
    nsop=$(compute_nsop "$t" "$N")
    printf "%s,%s,%s,%s,%s,%s,%s\n" "$b" "$t" "$nsop" "$instr" "$cycles" "$cmiss" "$bmiss" >> "$NODE_OUT"
  done
else
  echo "Node not found or no $NODE_DIR dir; skipping node benchmarks"
fi

# --------- 4) Python benchmarks -----------
echo
echo "== Running Python benchmarks =="
if command -v python3 >/dev/null 2>&1 && [[ -d "$PY_DIR" ]]; then
  for b in "${BENCHS[@]}"; do
    py="$PY_DIR/$b.py"
    if [[ ! -f "$py" ]]; then
      echo "Skipping py/$b (file missing)"
      continue
    fi
    echo " - python: $b"
    t=$(run_and_time python3 "$py")
    read instr cycles cmiss bmiss < <(run_perf_and_parse python3 "$py")
    N=${NMAP[$b]}
    nsop=$(compute_nsop "$t" "$N")
    printf "%s,%s,%s,%s,%s,%s,%s\n" "$b" "$t" "$nsop" "$instr" "$cycles" "$cmiss" "$bmiss" >> "$PY_OUT"
  done
else
  echo "python3 not found or no $PY_DIR dir; skipping python benchmarks"
fi

# --------- Merge outputs -----------
echo
echo "== Merging CSVs =="
# Merge: language column based on filename
# Format: Benchmark,Time(s),ns/op,Instructions,Cycles,CacheMisses,BranchMisses,Language
merge_file="$MERGED_OUT"
echo "Benchmark,Time(s),ns/op,Instructions,Cycles,CacheMisses,BranchMisses,Language" > "$merge_file"

append_with_lang() {
  local file="$1"; local lang="$2"
  if [[ ! -f "$file" ]]; then return; fi
  tail -n +2 "$file" | awk -F, -v OFS=, -v L="$lang" '{print $1,$2,$3,$4,$5,$6,$7,L}' >> "$merge_file"
}

append_with_lang "$TSV_OUT" "tspp"
append_with_lang "$CPP_OUT" "cpp"
append_with_lang "$NODE_OUT" "node"
append_with_lang "$PY_OUT" "python"

echo "Done. Outputs in $OUT_DIR:"
ls -1 "$OUT_DIR"/*.csv || true
echo
echo "To share data, upload these CSV files (especially $merge_file)."
