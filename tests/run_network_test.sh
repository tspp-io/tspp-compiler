#!/usr/bin/env bash
# Wrapper to start local HTTP and TCP servers, run the network test, then stop servers.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
HTTP_PORT=8000
TCP_PORT=8001

HTTP_PID_FILE="/tmp/tspp_http_server.pid"
TCP_PID_FILE="/tmp/tspp_tcp_server.pid"

start_servers() {
  echo "Starting local HTTP server on 127.0.0.1:${HTTP_PORT}"
  # Run Python http.server bound to localhost
  (cd "$ROOT_DIR/tests" && python3 -m http.server ${HTTP_PORT} --bind 127.0.0.1 > /dev/null 2>&1) &
  echo $! > "$HTTP_PID_FILE"

  echo "Starting local TCP echo server on 127.0.0.1:${TCP_PORT}"
  (cd "$ROOT_DIR/tests" && python3 local_tcp_echo.py ${TCP_PORT} > /dev/null 2>&1) &
  echo $! > "$TCP_PID_FILE"

  # Give servers a moment to start
  sleep 0.5
}

stop_servers() {
  if [ -f "$HTTP_PID_FILE" ]; then
    kill "$(cat "$HTTP_PID_FILE")" 2>/dev/null || true
    rm -f "$HTTP_PID_FILE"
  fi
  if [ -f "$TCP_PID_FILE" ]; then
    kill "$(cat "$TCP_PID_FILE")" 2>/dev/null || true
    rm -f "$TCP_PID_FILE"
  fi
}

trap stop_servers EXIT

start_servers

echo "Running network test through test.sh"
"$ROOT_DIR/test.sh" "$ROOT_DIR/tests/sys_network.tspp"

echo "Network test completed"
