#!/usr/bin/env python3
"""Simple TCP echo server for local deterministic tests.

Usage: python3 local_tcp_echo.py [port]
"""
import sys
import socket


def run(port: int):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('127.0.0.1', port))
        s.listen(5)
        print(f"TCP echo server listening on 127.0.0.1:{port}")
        try:
            while True:
                conn, addr = s.accept()
                with conn:
                    # echo until client closes
                    while True:
                        data = conn.recv(4096)
                        if not data:
                            break
                        conn.sendall(data)
        except KeyboardInterrupt:
            print("Shutting down TCP echo server")


if __name__ == '__main__':
    port = 8001
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except Exception:
            pass
    run(port)
