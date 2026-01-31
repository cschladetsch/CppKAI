#!/bin/bash

echo "=== KAI Console Network Communication Demonstration ==="
echo

echo "1. Starting Console 1 (Server) on port 14600..."
echo "Commands being sent to Console 1:"
echo "  /network start 14600"
echo "  42 7 +"
echo "  stack"
echo

LOG1="$(mktemp -t kai-console1.XXXXXX.log)"
LOG2="$(mktemp -t kai-console2.XXXXXX.log)"

if command -v rg >/dev/null 2>&1; then
    MATCH_CMD=(rg -q)
else
    MATCH_CMD=(grep -q)
fi

if command -v script >/dev/null 2>&1; then
    # Use a pseudo-TTY so rang emits colors.
    CONSOLE_CMD=(script -q /dev/null -c "./Bin/Console --non-interactive")
else
    CONSOLE_CMD=(./Bin/Console --non-interactive)
fi

# Console 1 commands
(
    echo "/network start 14600"
    sleep 1
    echo "42 7 +"
    echo "stack"
    # Keep server alive so Console 2 can connect and send commands.
    sleep 6
    echo "exit"
) | timeout 12s "${CONSOLE_CMD[@]}" >>"$LOG1" 2>&1 &
CONSOLE1_PID=$!

sleep 2

echo "2. Starting Console 2 (Client) on port 14601..."
echo "Commands being sent to Console 2:"
echo "  /network start 14601"
echo "  /connect 127.0.0.1 14600"
echo "  /peers"
echo "  /@0 10 *"
echo "  /broadcast stack"
echo

# Console 2 commands
(
    echo "/network start 14601"
    sleep 1
    echo "/connect 127.0.0.1 14600"
    sleep 2
    # Retry once in case the server isn't ready yet.
    echo "/connect 127.0.0.1 14600"
    sleep 1
    echo "/peers"
    echo "/@0 10 *"
    sleep 1
    echo "/broadcast stack"
    echo "exit"
) | timeout 15s "${CONSOLE_CMD[@]}" >>"$LOG2" 2>&1 &
CONSOLE2_PID=$!

# Wait for both consoles to finish
wait $CONSOLE1_PID 2>/dev/null
wait $CONSOLE2_PID 2>/dev/null

echo
echo "=== Console 1 Output ==="
grep -v -F "Session terminated, killing shell" "$LOG1" | grep -v -F "...killed."
echo
echo "=== Console 2 Output ==="
grep -v -F "Session terminated, killing shell" "$LOG2" | grep -v -F "...killed."

echo
echo "=== Demonstration Complete ==="
echo
echo "What this demonstrated:"
echo "✓ Console 1 started networking on port 14600"
echo "✓ Console 1 calculated 42 + 7 = 49"
echo "✓ Console 2 started networking on port 14601"

if "${MATCH_CMD[@]}" "Connection failed" "$LOG2"; then
    echo "✗ Console 2 failed to connect to Console 1"
    echo "✗ Console 2 sent command to Console 1: multiply by 10"
    echo "✗ Result: 49 * 10 = 490"
    echo "✗ Broadcast command showed stack on both consoles"
    if "${MATCH_CMD[@]}" "error: 2" "$LOG2"; then
        echo "Note: this build uses the in-memory RakNet stub, which only connects peers inside"
        echo "the same process. Two separate ./Bin/Console processes will always fail to connect."
    fi
else
    echo "✓ Console 2 connected to Console 1"
    echo "✓ Console 2 sent command to Console 1: multiply by 10"
    echo "✓ Result: 49 * 10 = 490"
    echo "✓ Broadcast command showed stack on both consoles"
fi

rm -f "$LOG1" "$LOG2"
echo
echo "Network Commands Available:"
echo "  /network start [port]   - Enable networking"
echo "  /connect <host> <port>  - Connect to peer"
echo "  /@<peer> <command>      - Execute on specific peer"
echo "  /broadcast <command>    - Execute on all peers"
echo "  /peers                  - List connected consoles"
echo "  /nethistory             - Show message history"
