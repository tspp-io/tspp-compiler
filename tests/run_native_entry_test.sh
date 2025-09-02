#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
BIN="$ROOT_DIR/build/src/tspp"
TEST="$ROOT_DIR/tests/native_entry_test.tspp"

if [ ! -x "$BIN" ]; then
  echo "Compiler not built at $BIN. Run ./build.sh first."
  exit 1
fi

"$BIN" --jit "$TEST" >/dev/null
CODE=$?
echo "Exit code: $CODE"
if [ "$CODE" != "42" ]; then
  echo "Expected exit 42 from native JIT main(), got $CODE"
  exit 1
fi
echo "✅ Native JIT entry test passed"
