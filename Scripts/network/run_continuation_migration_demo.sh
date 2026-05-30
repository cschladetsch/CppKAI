#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
BIN="$PROJECT_ROOT/Bin/ContinuationMigrationDemo"
PORT="${1:-17100}"
HOST="${2:-127.0.0.1}"

WORK_DIR="$(mktemp -d "${TMPDIR:-/tmp}/kai-cont-migration.XXXXXX")"
SERVER_LOG="$WORK_DIR/server.log"
CLIENT_LOG="$WORK_DIR/client.log"
TAIL_PID=""
SERVER_PID=""

cleanup() {
    if [ -n "${TAIL_PID}" ] && kill -0 "$TAIL_PID" 2>/dev/null; then
        kill "$TAIL_PID" 2>/dev/null || true
    fi
    if [ -n "${SERVER_PID}" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID" 2>/dev/null || true
    fi
    rm -rf "$WORK_DIR"
}

trap cleanup EXIT

echo "Building ContinuationMigrationDemo..."
cmake --build "$BUILD_DIR" --target ContinuationMigrationDemo

if [ ! -x "$BIN" ]; then
    echo "ERROR: missing demo binary at $BIN"
    exit 1
fi

echo "Starting server on $HOST:$PORT..."
echo "SERVER: will thaw and resume the migrated Pi continuation"
"$BIN" --server --host "$HOST" --port "$PORT" >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!

tail -n +1 -f "$SERVER_LOG" &
TAIL_PID=$!

ready_deadline=$((SECONDS + 20))
while ! grep -q "SERVER_READY" "$SERVER_LOG"; do
    if ! kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "ERROR: server exited before becoming ready"
        cat "$SERVER_LOG"
        exit 1
    fi
    if [ "$SECONDS" -ge "$ready_deadline" ]; then
        echo "ERROR: server did not become ready in time"
        cat "$SERVER_LOG"
        exit 1
    fi
    sleep 1
done

handle="$(sed -n 's/.*handle=\([0-9][0-9]*\).*/\1/p' "$SERVER_LOG" | tail -n 1)"
if [ -z "$handle" ]; then
    echo "ERROR: could not read agent handle from server log"
    cat "$SERVER_LOG"
    exit 1
fi

echo "Starting client against handle $handle..."
echo "CLIENT: compiling Pi continuation: { 2 * } 'double # 5 double &"
echo "CLIENT: freezing continuation and sending it to the server process"
"$BIN" --client --host "$HOST" --port "$PORT" --handle "$handle" 2>&1 | tee "$CLIENT_LOG"

wait "$SERVER_PID"

if [ -n "${TAIL_PID}" ] && kill -0 "$TAIL_PID" 2>/dev/null; then
    kill "$TAIL_PID" 2>/dev/null || true
fi

echo "----- SERVER LOG -----"
cat "$SERVER_LOG"
echo "----- CLIENT LOG -----"
cat "$CLIENT_LOG"

if ! grep -q "MIGRATION_OK result=10" "$CLIENT_LOG"; then
    echo "ERROR: migration did not complete successfully"
    exit 1
fi

echo "PROOF: the continuation moved processes, resumed remotely, and returned result=10."
echo "Continuation migration demo completed successfully."
