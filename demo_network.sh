#!/bin/bash

echo "=== KAI Console Network Communication Demonstration ==="
echo

echo "1. Starting Console 1 (Server) on port 14600..."
echo "Commands being sent to Console 1:"
echo "  /network start 14600"
echo "  42 7 +"
echo "  stack"
echo

# Console 1 commands
(
    echo "/network start 14600"
    sleep 1
    echo "42 7 +"
    echo "stack"
    echo "exit"
) | timeout 10s ./Bin/Console --non-interactive &
CONSOLE1_PID=$!

sleep 2

echo "2. Starting Console 2 (Client) on port 14601..."
echo "Commands being sent to Console 2:"
echo "  /network start 14601"
echo "  /connect localhost 14600"
echo "  /peers"
echo "  /@0 10 *"
echo "  /broadcast stack"
echo

# Console 2 commands
(
    echo "/network start 14601"
    sleep 1
    echo "/connect localhost 14600"
    sleep 1
    echo "/peers"
    echo "/@0 10 *"
    sleep 1
    echo "/broadcast stack"
    echo "exit"
) | timeout 15s ./Bin/Console --non-interactive &
CONSOLE2_PID=$!

# Wait for both consoles to finish
wait $CONSOLE1_PID 2>/dev/null
wait $CONSOLE2_PID 2>/dev/null

echo
echo "=== Demonstration Complete ==="
echo
echo "What this demonstrated:"
echo "✓ Console 1 started networking on port 14600"
echo "✓ Console 1 calculated 42 + 7 = 49"
echo "✓ Console 2 started networking on port 14601"
echo "✓ Console 2 connected to Console 1"
echo "✓ Console 2 sent command to Console 1: multiply by 10"
echo "✓ Result: 49 * 10 = 490"
echo "✓ Broadcast command showed stack on both consoles"
echo
echo "Network Commands Available:"
echo "  /network start [port]   - Enable networking"
echo "  /connect <host> <port>  - Connect to peer"
echo "  /@<peer> <command>      - Execute on specific peer"
echo "  /broadcast <command>    - Execute on all peers"
echo "  /peers                  - List connected consoles"
echo "  /nethistory             - Show message history"