#!/bin/bash

# Test script to demonstrate client-server calculation

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

# Create test configs with specific parameters
mkdir -p "$ROOT_DIR/config"
cat > "$ROOT_DIR/config/server_test_config.json" << EOF
{
    "port": 14595,
    "maxClients": 4,
    "enableCalculation": true,
    "echoMode": true
}
EOF

cat > "$ROOT_DIR/config/client_test_config.json" << EOF
{
    "serverIp": "127.0.0.1",
    "serverPort": 14595,
    "autoCalculate": true,
    "calculationExpression": "1+2",
    "waitForResult": true
}
EOF

# Build the configurable server and client
echo "Building ConfigurableServer and ConfigurableClient..."
cmake --build . --target ConfigurableServer ConfigurableClient -- -j$(nproc)

# Check for successful build
if [ ! -f "$BUILD_DIR/Bin/ConfigurableServer" ] || [ ! -f "$BUILD_DIR/Bin/ConfigurableClient" ]; then
    echo "ERROR: Failed to build ConfigurableServer or ConfigurableClient"
    echo "Check CMake configuration and build errors"
    exit 1
fi

echo "Build successful!"

# Start the server process in the background
echo "Starting calculation server..."
"$BUILD_DIR/Bin/ConfigurableServer" "$ROOT_DIR/config/server_test_config.json" &
SERVER_PID=$!

# Give the server time to start up
sleep 2

# Verify server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "ERROR: Server process is not running"
    cat server_output.log
    exit 1
fi

echo "Server started with PID $SERVER_PID"

# Run the client with the test config
echo "Running client with calculation request '1+2'..."
"$BUILD_DIR/Bin/ConfigurableClient" "$ROOT_DIR/config/client_test_config.json"
CLIENT_EXIT=$?

# Kill the server process
echo "Stopping server..."
kill $SERVER_PID

# Check client exit code
if [ $CLIENT_EXIT -eq 0 ]; then
    echo "✅ TEST PASSED: Client received correct calculation result (3)"
else
    echo "❌ TEST FAILED: Client did not receive correct calculation result"
    exit 1
fi

# Clean up test config files
rm -f "$ROOT_DIR/config/server_test_config.json" "$ROOT_DIR/config/client_test_config.json"

echo "Test complete."