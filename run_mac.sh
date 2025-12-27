#!/usr/bin/env bash

function detect_os() {
  uname -s
}

function maybe_set_qt_prefix() {
  if [ -n "${CMAKE_PREFIX_PATH:-}" ]; then
    return 0
  fi

  if ! command -v brew >/dev/null 2>&1; then
    return 0
  fi

  local qt_prefix
  qt_prefix="$(brew --prefix qt@5 2>/dev/null)"
  if [ -z "$qt_prefix" ]; then
    qt_prefix="$(brew --prefix qt 2>/dev/null)"
  fi

  if [ -n "$qt_prefix" ]; then
    export CMAKE_PREFIX_PATH="$qt_prefix"
  fi
}

function build() {
  echo "[BUILD] Compiling Project..."

  echo "--- Building Server ---"
  make directories bin/server

  echo "--- Building Qt Client ---"
  mkdir -p src/client/gui/build
  if [ "$(detect_os)" = "Darwin" ]; then
    maybe_set_qt_prefix
  fi
  cmake -S src/client/gui -B src/client/gui/build
  cmake --build src/client/gui/build
}

function build_console() {
  echo "[BUILD_CONSOLE] Compiling Terminal Client..."
  make directories bin/client
}

function server() {
  echo "[SERVER] Starting server..."
  ./bin/server "$@"
}

function client() {
  if [ ! -f "./src/client/gui/build/socker_client" ]; then
    echo "[CLIENT] Binary not found. Building..."
    build
  fi
  echo "[CLIENT] Starting Qt client..."
  ./src/client/gui/build/socker_client "$@"
}

function client_console() {
  echo "[CLIENT_CONSOLE] Starting terminal client..."
  ./bin/client "$@"
}

function clean() {
  echo "[CLEAN] Removing build artifacts..."
  make clean
  rm -rf src/client/gui/build
}

function help() {
  echo "Usage: ./run.sh <command> [args]"
  echo
  echo "Commands:"
  echo "  build           Build Server and Qt Client"
  echo "  build_console   Build Terminal Client"
  echo "  server [port]   Run the server on optional port (default 8080)"
  echo "  client          Run the Qt client"
  echo "  client_console  Run the Terminal client"
  echo "  clean           Clean all build artifacts"
  echo "  help            Show this help message"
}

case "$1" in
  build)
    shift; build "$@" ;;
  build_console)
    shift; build_console "$@" ;;
  server)
    shift; server "$@" ;;
  client)
    shift; client "$@" ;;
  client_console)
    shift; client_console "$@" ;;
  clean)
    shift; clean ;;
  help | --help | -h | "")
    help ;;
  *)
    echo "Unknown command: $1" >&2
    help
    exit 1 ;;
esac
