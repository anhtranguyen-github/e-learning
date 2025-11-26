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

function build_qt() {
  echo "[BUILD QT] Compiling Qt client..."
  mkdir -p src/client/gui/build
  cd src/client/gui/build
  cmake ..
  make
}

function run_qt() {
  echo "[RUN QT] Starting Qt client..."
  ./src/client/gui/build/socker_client "$@"
}

function help() {
  echo "Usage: ./run.sh <command> [args]"
  echo
  echo "Commands:"
  echo "  build           Build the project (make)"
  echo "  server [port]   Run the server on optional port (default 8080)"
  echo "  client [host] [port]  Run a client (default 127.0.0.1 8080)"
  echo "  build_qt        Build the Qt client"
  echo "  run_qt          Run the Qt client"
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
  build_qt)
    shift; build_qt "$@" ;;
  run_qt)
    shift; run_qt "$@" ;;
  clean)
    shift; clean ;;
  help | --help | -h | "")
    help ;;
  *)
    echo "Unknown command: $1" >&2
    help
    exit 1 ;;
esac
