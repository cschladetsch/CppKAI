#!/bin/bash

# Test script to demonstrate peer-to-peer calculation

# Navigate to project root directory
cd "$(dirname "$0")/.."
ROOT_DIR=$(pwd)
BUILD_DIR="$ROOT_DIR/build"

# Make sure the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake ..
else
    cd "$BUILD_DIR"
fi

# Create temporary test configs with specific parameters for two peers
# Use the config directory instead of the root directory
cat > "$ROOT_DIR/config/temp_peer1_config.json" << 'EOFMARKER'
{
    "listenPort": 14595,
    "autoListen": true,
    "interactive": false,
    "peers": [],
    "commands": {
        "add": "calc 1+2"
    }
}
EOFMARKER

cat > "$ROOT_DIR/config/temp_peer2_config.json" << 'EOFMARKER'
{
    "listenPort": 14596,
    "autoListen": true,
    "interactive": false,
    "peers": [
        {
            "host": "127.0.0.1",
            "port": 14595
        }
    ],
    "commands": {}
}
EOFMARKER

# Build the network peer application
echo "Building NetworkPeer..."
cmake --build . --target NetworkPeer -- -j$(nproc)

# Check for successful build
if [ ! -f "$BUILD_DIR/Bin/NetworkPeer" ]; then
    echo "ERROR: Failed to build NetworkPeer"
    echo "Check CMake configuration and build errors"
    exit 1
fi

echo "Build successful!"

# Start the first peer process in the background
echo "Starting first peer (listening)..."
"$BUILD_DIR/Bin/NetworkPeer" "$ROOT_DIR/config/temp_peer1_config.json" > peer1_output.log 2>&1 &
PEER1_PID=$!

# Give the first peer time to start up
echo "Waiting for first peer to start up..."
sleep 10  # Increased sleep time further

# Verify first peer is running
if ! kill -0 $PEER1_PID 2>/dev/null; then
    echo "ERROR: First peer process is not running"
    cat peer1_output.log
    exit 1
fi

echo "First peer started with PID $PEER1_PID"

# Create a named pipe for feeding commands to the second peer
FIFO="/tmp/peer2_input_$$.fifo"
mkfifo $FIFO

# Start the second peer with the fifo connected to stdin
echo "Starting second peer (connecting)..."
cat $FIFO | "$BUILD_DIR/Bin/NetworkPeer" "$ROOT_DIR/config/temp_peer2_config.json" > peer2_output.log 2>&1 &
PEER2_PID=$!

# Give the second peer time to start and connect
echo "Waiting for second peer to connect to first peer..."
sleep 15  # Increased sleep time for connection even more

# Verify second peer is running
if ! kill -0 $PEER2_PID 2>/dev/null; then
    echo "ERROR: Second peer process is not running"
    cat peer2_output.log
    kill $PEER1_PID
    rm $FIFO
    exit 1
fi

echo "Second peer started with PID $PEER2_PID"

# Verify connection status by checking logs
echo "Checking connection status in logs..."
grep "Connected to peer" peer2_output.log || echo "No connection confirmation found in peer2 logs"

# Show peer list (if any)
echo "Checking for peer list in second peer log..."
grep -A5 "Connected peers" peer2_output.log || echo "No peer list found in logs"

# Send commands to the second peer to execute on the first peer
echo "Sending remote calculation command..."
echo "peers" > $FIFO
sleep 2
echo "@0 add" > $FIFO

# Give time for command to process
sleep 20  # Increased sleep time to give even more time for processing

# Show the contents of the log files for debugging
echo "Contents of peer1_output.log:"
cat peer1_output.log
echo "----------------------------------------"
echo "Contents of peer2_output.log:"
cat peer2_output.log
echo "----------------------------------------"

# Check the output for the expected result
if grep -q "Result from.*add.*3" peer2_output.log; then
    echo "✅ TEST PASSED: Successfully executed remote calculation (1+2=3)"
else
    echo "❌ TEST FAILED: Remote calculation did not produce expected result"
    
    # Run another grep to see what we actually got
    echo "Searching for 'Result from' in peer2_output.log:"
    grep "Result from" peer2_output.log || echo "No 'Result from' found in the output"
    
    echo "Searching for occurrences of '3' in peer2_output.log:"
    grep "3" peer2_output.log || echo "No '3' found in the output"
fi

# Clean up processes
echo "Stopping peers..."
kill $PEER1_PID $PEER2_PID

# Clean up test files
rm -f "$ROOT_DIR/config/temp_peer1_config.json" "$ROOT_DIR/config/temp_peer2_config.json" $FIFO

echo "Test complete."