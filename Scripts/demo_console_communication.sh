#!/bin/bash

# Demo script showing two KAI consoles communicating using tmux
# This script demonstrates the console-to-console communication features

echo "========================================"
echo "KAI Console-to-Console Communication Demo"
echo "========================================"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}This demo uses tmux to show two KAI consoles communicating over a network.${NC}"
echo ""
echo -e "${YELLOW}Features demonstrated:${NC}"
echo "• Network console setup and connection"
echo "• Sending commands between consoles"
echo "• Broadcasting commands to all peers"
echo "• Cross-language communication (Pi ↔ Rho)"
echo "• Real-time command execution and results"
echo "• Message history tracking"
echo ""

# Check if tmux is available
if ! command -v tmux &> /dev/null; then
    echo -e "${RED}Error: tmux is required for this demo but not installed.${NC}"
    echo "Please install tmux:"
    echo "  Ubuntu/Debian: sudo apt-get install tmux"
    echo "  CentOS/RHEL: sudo yum install tmux"
    echo "  macOS: brew install tmux"
    exit 1
fi

# Check if console binary exists
CONSOLE_BIN="./Bin/Console"
if [ ! -f "$CONSOLE_BIN" ]; then
    CONSOLE_BIN="../build/Bin/Console"
    if [ ! -f "$CONSOLE_BIN" ]; then
        echo -e "${RED}Error: Console binary not found. Please build the project first.${NC}"
        echo "Expected locations:"
        echo "  ./Bin/Console"
        echo "  ../build/Bin/Console"
        exit 1
    fi
fi

echo -e "${GREEN}Found console binary: $CONSOLE_BIN${NC}"
echo ""

# Session name for tmux
SESSION_NAME="kai_console_demo"

# Kill existing session if it exists
tmux kill-session -t $SESSION_NAME 2>/dev/null

echo -e "${BLUE}Setting up tmux session with two console panes...${NC}"

# Create new tmux session with first console (Server)
tmux new-session -d -s $SESSION_NAME -x 120 -y 40

# Rename the first window
tmux rename-window -t $SESSION_NAME:0 "Console-Demo"

# Split window vertically to create two panes
tmux split-window -t $SESSION_NAME:0 -h

# Set pane titles
tmux select-pane -t $SESSION_NAME:0.0 -T "Console 1 (Server)"
tmux select-pane -t $SESSION_NAME:0.1 -T "Console 2 (Client)"

# Start console in left pane (Console 1 - Server)
tmux send-keys -t $SESSION_NAME:0.0 "echo 'Console 1 (Server) - Port 14600'" C-m
tmux send-keys -t $SESSION_NAME:0.0 "echo 'Starting KAI Console...'" C-m
tmux send-keys -t $SESSION_NAME:0.0 "$CONSOLE_BIN" C-m

# Wait a moment for console to start
sleep 2

# Start console in right pane (Console 2 - Client)
tmux send-keys -t $SESSION_NAME:0.1 "echo 'Console 2 (Client) - Port 14601'" C-m
tmux send-keys -t $SESSION_NAME:0.1 "echo 'Starting KAI Console...'" C-m
tmux send-keys -t $SESSION_NAME:0.1 "$CONSOLE_BIN" C-m

# Wait for both consoles to start
sleep 3

echo -e "${YELLOW}Setting up network communication...${NC}"

# Setup Console 1 (Server)
tmux send-keys -t $SESSION_NAME:0.0 "/network start 14600" C-m
sleep 1
tmux send-keys -t $SESSION_NAME:0.0 "2 3 +" C-m
tmux send-keys -t $SESSION_NAME:0.0 "5 7 *" C-m
tmux send-keys -t $SESSION_NAME:0.0 "stack" C-m

# Setup Console 2 (Client)
sleep 2
tmux send-keys -t $SESSION_NAME:0.1 "/network start 14601" C-m
sleep 1
tmux send-keys -t $SESSION_NAME:0.1 "/connect localhost 14600" C-m
sleep 2

echo -e "${GREEN}Demonstrating console-to-console communication...${NC}"

# Demo sequence
sleep 1
tmux send-keys -t $SESSION_NAME:0.1 "/@0 10 +" C-m  # Add 10 to Console 1's stack
sleep 2
tmux send-keys -t $SESSION_NAME:0.1 "/broadcast stack" C-m  # Broadcast stack command
sleep 2  
tmux send-keys -t $SESSION_NAME:0.1 "/@0 \"Hello from Console 2!\" print" C-m
sleep 2
tmux send-keys -t $SESSION_NAME:0.1 "/peers" C-m
sleep 1

# Show some Pi operations on Console 1
tmux send-keys -t $SESSION_NAME:0.0 "100 200 +" C-m
sleep 1

# Console 2 manipulates Console 1's result
tmux send-keys -t $SESSION_NAME:0.1 "/@0 2 /" C-m  # Divide by 2
sleep 2

# Show network history
tmux send-keys -t $SESSION_NAME:0.1 "/nethistory" C-m
sleep 2

# Advanced demo - Cross language
echo -e "${BLUE}Demonstrating cross-language communication...${NC}"
sleep 1

# Switch Console 2 to Rho mode
tmux send-keys -t $SESSION_NAME:0.1 "rho" C-m
sleep 1

# Send Rho-style command from Console 2 to Console 1 (which will execute in Pi)
tmux send-keys -t $SESSION_NAME:0.1 "/@0 42 dup *" C-m  # 42^2 = 1764
sleep 2

# Show final results
tmux send-keys -t $SESSION_NAME:0.0 "stack" C-m
tmux send-keys -t $SESSION_NAME:0.1 "pi" C-m  # Switch back to Pi mode
tmux send-keys -t $SESSION_NAME:0.1 "/network status" C-m

# Add instructions pane at the bottom
tmux split-window -t $SESSION_NAME:0 -v -p 30
tmux select-pane -t $SESSION_NAME:0.2 -T "Instructions & Controls"

# Display instructions
tmux send-keys -t $SESSION_NAME:0.2 "echo ''" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo 'KAI Console-to-Console Demo Instructions:'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo '========================================'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo ''" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo 'Left Pane:  Console 1 (Server) - Port 14600'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo 'Right Pane: Console 2 (Client) - Port 14601'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo ''" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo 'Try these commands in Console 2 (right pane):'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo '  /@0 <command>      - Send command to Console 1'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo '  /broadcast <cmd>   - Send to all peers'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo '  /peers             - List connected peers'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo '  /nethistory        - Show message history'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo '  rho / pi           - Switch languages'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo ''" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo 'Navigation:'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo '  Ctrl+b, arrow keys - Switch between panes'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo '  Ctrl+b, d         - Detach from session'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo '  exit (in console) - Exit console'" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo ''" C-m
tmux send-keys -t $SESSION_NAME:0.2 "echo 'To exit demo: type \"exit\" in both consoles'" C-m

# Focus on Console 2 (client) for user interaction
tmux select-pane -t $SESSION_NAME:0.1

echo ""
echo -e "${GREEN}Demo is now running in tmux!${NC}"
echo ""
echo -e "${YELLOW}Tmux Session: $SESSION_NAME${NC}"
echo ""
echo -e "${CYAN}Controls:${NC}"
echo "• Ctrl+b, arrow keys - Navigate between panes"
echo "• Ctrl+b, d - Detach from session (keeps running)"
echo "• tmux attach -t $SESSION_NAME - Reattach to session"
echo "• Type 'exit' in both consoles to end demo"
echo ""
echo -e "${BLUE}Attaching to tmux session...${NC}"
echo ""

# Cleanup function
cleanup() {
    echo ""
    echo -e "${YELLOW}Cleaning up demo session...${NC}"
    tmux kill-session -t $SESSION_NAME 2>/dev/null
    echo -e "${GREEN}Demo session terminated.${NC}"
}

# Set trap for cleanup on script exit
trap cleanup EXIT

# Attach to the session
tmux attach-session -t $SESSION_NAME

# After user exits tmux session
echo ""
echo -e "${GREEN}Demo completed!${NC}"
echo ""
echo -e "${BLUE}Summary of what was demonstrated:${NC}"
echo "✓ Two KAI consoles communicating over network"
echo "✓ Remote command execution with real-time results"
echo "✓ Broadcasting commands to all connected peers"
echo "✓ Cross-language communication (Pi ↔ Rho)"
echo "✓ Network message history and peer management"
echo ""
echo -e "${YELLOW}Test Case Information:${NC}"
echo "Comprehensive test suite available at:"
echo "  Test/Console/TestConsoleNetworking.cpp"
echo ""
echo "Run tests with:"
echo "  make test"
echo "  # or specific test:"
echo "  ctest -R ConsoleNetworking"
echo ""
echo -e "${CYAN}Network Commands Reference:${NC}"
echo "  /network start [port]   - Start networking"
echo "  /network stop           - Stop networking"  
echo "  /network status         - Show network status"
echo "  /connect <host> <port>  - Connect to peer"
echo "  /peers                  - List connected peers"
echo "  /broadcast <command>    - Send to all peers"
echo "  /@<peer> <command>      - Send to specific peer"
echo "  /nethistory             - Show message history"
echo "  help network            - Network help"
echo ""
echo "========================================"