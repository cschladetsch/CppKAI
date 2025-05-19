#!/bin/bash

# Run a simple demonstration of the minimal connection example
# This script starts the minimal server and client, demonstrates connection,
# and exchanges a message for a calculation

# Navigate to project root directory
cd "$(dirname "$0")/.."
ROOT_DIR=$(pwd)
BUILD_DIR="$ROOT_DIR/build"

# Ensure build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Enter build directory
cd "$BUILD_DIR"

# Configure and build the minimal examples
echo "Building minimal server and client..."
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target MinimalServer MinimalClient

# Check if builds were successful
if [ ! -f "bin/MinimalServer" ] || [ ! -f "bin/MinimalClient" ]; then
    echo "ERROR: Failed to build MinimalServer or MinimalClient."
    echo "Check build output for errors."
    exit 1
fi

# Set up a port for testing
TEST_PORT=14595

# Start the server in the background
echo "Starting the minimal server on port $TEST_PORT..."
./bin/MinimalServer $TEST_PORT > server_output.log 2>&1 &
SERVER_PID=$!

# Wait for server to start
echo "Waiting for server to initialize..."
sleep 2

# Check if server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "ERROR: Server failed to start. Check server_output.log for details."
    cat server_output.log
    exit 1
fi

echo "Server started with PID $SERVER_PID"

# Start the client in the background
echo "Starting the minimal client connecting to localhost:$TEST_PORT..."
./bin/MinimalClient 127.0.0.1 $TEST_PORT > client_output.log 2>&1 &
CLIENT_PID=$!

# Wait for client to start and connect
echo "Waiting for client to connect..."
sleep 2

# Check if client is running
if ! kill -0 $CLIENT_PID 2>/dev/null; then
    echo "ERROR: Client failed to start. Check client_output.log for details."
    cat client_output.log
    kill $SERVER_PID
    exit 1
fi

echo "Client started with PID $CLIENT_PID"

# Send a test message to the client (will be echoed by server)
echo "Sending test calculation message '1+2' to client..."
# Use named pipe to send input to client's stdin
FIFO="/tmp/client_input_$$.fifo"
mkfifo $FIFO
cat $FIFO | tee /dev/tty | ./bin/MinimalClient 127.0.0.1 $TEST_PORT > client_output2.log 2>&1 &
NEW_CLIENT_PID=$!
echo "1+2" > $FIFO
sleep 2
rm $FIFO

# Display output from the server and client
echo "============= SERVER OUTPUT ============="
cat server_output.log
echo
echo "============= CLIENT OUTPUT ============="
cat client_output.log
echo
echo "========= CALCULATION OUTPUT ==========="
cat client_output2.log

# Clean up
echo "Cleaning up processes..."
kill $SERVER_PID $CLIENT_PID $NEW_CLIENT_PID 2>/dev/null

echo "Demo completed."