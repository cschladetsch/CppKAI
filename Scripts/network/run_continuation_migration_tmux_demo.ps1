#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
BIN="$PROJECT_ROOT/Bin/ContinuationMigrationDemo"
SESSION="${TMUX_SESSION_NAME:-kai-cont-migration}"
PORT="${1:-17100}"
HOST="${2:-127.0.0.1}"

WORK_DIR="$(mktemp -d "${TMPDIR:-/tmp}/kai-cont-migration-tmux.XXXXXX")"
SERVER_LOG="$WORK_DIR/server.log"
CLIENT_LOG="$WORK_DIR/client.log"
DRIVER_PID=""

cleanup() {
    if [ -n "${DRIVER_PID}" ] && kill -0 "$DRIVER_PID" 2>/dev/null; then
        kill "$DRIVER_PID" 2>/dev/null || true
    fi

    if tmux has-session -t "$SESSION" 2>/dev/null; then
        tmux kill-session -t "$SESSION" 2>/dev/null || true
    fi

    rm -rf "$WORK_DIR"
}

trap cleanup EXIT

if ! command -v tmux >/dev/null 2>&1; then
    echo "ERROR: tmux is required for this demo"
    exit 1
fi

echo "Building ContinuationMigrationDemo..."
cmake --build "$BUILD_DIR" --target ContinuationMigrationDemo

if [ ! -x "$BIN" ]; then
    echo "ERROR: missing demo binary at $BIN"
    exit 1
fi

if tmux has-session -t "$SESSION" 2>/dev/null; then
    tmux kill-session -t "$SESSION"
fi

echo "Creating tmux session '$SESSION'..."
tmux new-session -d -s "$SESSION" -n demo -c "$PROJECT_ROOT" bash
tmux split-window -h -t "$SESSION:demo" -c "$PROJECT_ROOT" bash
tmux select-layout -t "$SESSION:demo" even-horizontal

tmux send-keys -t "$SESSION:demo.0" "clear" C-m
tmux send-keys -t "$SESSION:demo.0" "printf 'Server pane ready.\\n'" C-m
tmux send-keys -t "$SESSION:demo.1" "clear" C-m
tmux send-keys -t "$SESSION:demo.1" "printf 'Client pane ready.\\n'" C-m

driver() {
    sleep 2
    tmux display-message -t "$SESSION" "Step 1: starting server"
    server_cmd="\"$BIN\" --server --host \"$HOST\" --port \"$PORT\" 2>&1 | tee \"$SERVER_LOG\""
    tmux send-keys -t "$SESSION:demo.0" "$server_cmd" C-m

    local deadline=$((SECONDS + 30))
    while ! grep -q "SERVER_READY" "$SERVER_LOG"; do
        if ! tmux has-session -t "$SESSION" 2>/dev/null; then
            return 1
        fi
        if [ "$SECONDS" -ge "$deadline" ]; then
            echo "ERROR: server did not become ready in time"
            return 1
        fi
        sleep 1
    done

    local handle
    handle="$(sed -n 's/.*handle=\([0-9][0-9]*\).*/\1/p' "$SERVER_LOG" | tail -n 1)"
    if [ -z "$handle" ]; then
        echo "ERROR: could not read agent handle from server log"
        return 1
    fi

    sleep 2
    tmux display-message -t "$SESSION" "Step 2: launching client with handle $handle"
    client_cmd="\"$BIN\" --client --host \"$HOST\" --port \"$PORT\" --handle \"$handle\" 2>&1 | tee \"$CLIENT_LOG\""
    tmux send-keys -t "$SESSION:demo.1" "$client_cmd" C-m

    deadline=$((SECONDS + 30))
    while ! grep -q "MIGRATION_OK result=42" "$CLIENT_LOG"; do
        if ! tmux has-session -t "$SESSION" 2>/dev/null; then
            return 1
        fi
        if [ "$SECONDS" -ge "$deadline" ]; then
            echo "ERROR: client did not report success in time"
            return 1
        fi
        sleep 1
    done

    sleep 1
    tmux display-message -t "$SESSION" "Step 3: continuation migration completed successfully"
}

driver &
DRIVER_PID=$!

if [ -n "${TMUX:-}" ]; then
    echo "Already inside tmux; leaving the demo session detached."
    wait "$DRIVER_PID"
elif [ -t 1 ]; then
    echo "Attach with tmux to watch the demo. Detach with Ctrl-b d."
    tmux attach-session -t "$SESSION"
else
    wait "$DRIVER_PID"
fi

wait "$DRIVER_PID" || true

if [ -f "$SERVER_LOG" ] && [ -f "$CLIENT_LOG" ]; then
    echo "----- SERVER LOG -----"
    cat "$SERVER_LOG"
    echo "----- CLIENT LOG -----"
    cat "$CLIENT_LOG"
fi

if ! grep -q "MIGRATION_OK result=42" "$CLIENT_LOG" 2>/dev/null; then
    echo "ERROR: migration did not complete successfully"
    exit 1
fi

echo "Continuation migration tmux demo completed successfully."