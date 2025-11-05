#!/usr/bin/env bash

set -e

function build() {
  echo "[BUILD] Compiling project..."
  make
}

function server() {
  echo "[SERVER] Starting server..."
  ./bin/server "$@"
}

function client() {
  echo "[CLIENT] Starting client..."
  ./bin/client "$@"
}

function clean() {
  echo "[CLEAN] Removing build artifacts..."
  make clean
}

function help() {
  echo "Usage: ./run.sh <command> [args]"
  echo
  echo "Commands:"
  echo "  build           Build the project (make)"
  echo "  server [port]   Run the server on optional port (default 8080)"
  echo "  client [host] [port]  Run a client (default 127.0.0.1 8080)"
  echo "  clean           Clean build artifacts (make clean)"
  echo "  help            Show this help message"
}

case "$1" in
  build)
    shift; build "$@" ;;
  server)
    shift; server "$@" ;;
  client)
    shift; client "$@" ;;
  clean)
    shift; clean ;;
  help | --help | -h | "")
    help ;;
  *)
    echo "Unknown command: $1" >&2
    help
    exit 1 ;;
esac
