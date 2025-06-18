# Network Calculation Test

This document describes the implementation of the network calculation test for KAI, which demonstrates how two nodes can interact by sending calculations and receiving results.

## Overview

The test consists of a client-server architecture where:
1. The server listens for calculation requests
2. The client connects and sends a calculation expression ("1+2")
3. The server parses the expression, calculates the result (3), and sends it back
4. The client verifies the result

## Components

### ConfigurableServer

The server component is implemented in `ConfigurableServer.cpp` and provides:
- JSON configuration via `server_config.json`
- Expression parsing for simple addition operations
- Response to calculation requests with results
- Support for multiple clients

### ConfigurableClient

The client component is implemented in `ConfigurableClient.cpp` and provides:
- JSON configuration via `client_config.json`
- Interactive or automatic operation modes
- Ability to send calculation requests
- Processing of calculation responses

### Configuration Files

Both components use JSON configuration files:

**server_config.json**:
```json
{
    "port": 14591,
    "maxClients": 32,
    "enableCalculation": true,
    "echoMode": true
}
```

**client_config.json**:
```json
{
    "serverIp": "127.0.0.1",
    "serverPort": 14591,
    "autoCalculate": false,
    "calculationExpression": "1+2",
    "waitForResult": true
}
```

## Testing

The `calc_test.sh` script demonstrates the calculation functionality:
1. It builds the configurable server and client
2. Creates test-specific configuration files
3. Starts the server in the background
4. Runs the client with automatic calculation of "1+2"
5. Verifies that the result (3) is received correctly

## Running the Test

```bash
./Scripts/calc_test.sh
```

A successful test will output:
```
TEST PASSED: Client received correct calculation result (3)
```

## Implementation Details

### Message Types

The implementation uses custom message types for different operations:
- `ID_CUSTOM_MESSAGE`: General text messages
- `ID_CALCULATION_REQUEST`: Client-to-server calculation requests
- `ID_CALCULATION_RESPONSE`: Server-to-client calculation results

### Expression Parsing

The server uses a simple regex-based parser to extract operands from the expression:
```cpp
int performCalculation(const std::string& expression) {
    // Very simple parser for "a+b" expressions
    std::regex pattern("(\\d+)\\s*\\+\\s*(\\d+)");
    std::smatch matches;
    
    if (std::regex_search(expression, matches, pattern) && matches.size() == 3) {
        int a = std::stoi(matches[1].str());
        int b = std::stoi(matches[2].str());
        return a + b;
    }
    
    // For unrecognized expressions, return -1
    return -1;
}
```

This implementation is intentionally simple and only handles addition of two integers, but it demonstrates the core concept of remote calculation.

## Future Improvements

1. Support for more complex expressions and operations
2. Improved error handling and validation
3. Secure communication with encryption
4. Support for asynchronous calculations
5. Load balancing across multiple calculation servers