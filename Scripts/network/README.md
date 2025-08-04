# KAI Network Test Scripts

This directory contains scripts for testing and demonstrating the peer-to-peer network functionality in KAI.

## Purpose

These scripts serve multiple purposes:

1. **Demonstration**: Show how the peer-to-peer network functionality works
2. **Testing**: Verify that the network components are functioning correctly
3. **Example**: Provide examples of how to use the NetworkPeer application
4. **Documentation**: Illustrate the command execution flow between peers

## Available Scripts

- **run_peers.sh**: Provides instructions for running and testing peer-to-peer communication manually
- **automated_demo.sh**: Builds and verifies the NetworkPeer application readiness

## Script Details

### Run Peers Script (`run_peers.sh`)

This script provides step-by-step instructions for manually testing peer-to-peer communication:

```bash
./Scripts/network/run_peers.sh
```

The script will:
1. Explain how to open two terminal windows
2. Provide the commands to run peer1 and peer2
3. Give instructions for testing the connection
4. Show how to execute commands remotely

This approach allows you to:
- See the real-time communication between peers
- Experiment with different commands
- Observe the network behavior interactively

### Automated Demo Script (`automated_demo.sh`)

This script builds the NetworkPeer application and verifies it's ready for use:

```bash
./Scripts/network/automated_demo.sh
```

The script will:
1. Build the NetworkPeer application
2. Verify that the build was successful
3. Provide a summary of the peer-to-peer functionality
4. Show instructions for manual testing

This ensures that the network components are correctly built and available.

## Configuration

These scripts use the configuration files from the `config/` directory:

- **peer1_config.json**: Server-like peer configuration
  ```json
  {
      "listenPort": 14595,
      "autoListen": true,
      "interactive": true,
      "peers": [],
      "commands": {
          "add": "calc 1+2",
          "hello": "echo Hello from peer1"
      }
  }
  ```

- **peer2_config.json**: Client-like peer configuration
  ```json
  {
      "listenPort": 14596,
      "autoListen": true,
      "interactive": true,
      "peers": [
          {
              "host": "127.0.0.1",
              "port": 14595
          }
      ],
      "commands": {
          "test": "echo Testing from peer2"
      }
  }
  ```

If you modify these configurations, the scripts will automatically use the updated versions.

## Testing Workflow

The recommended testing workflow is:

1. Run `automated_demo.sh` to verify the build
2. Run `run_peers.sh` to get manual testing instructions
3. Follow the instructions to test peer-to-peer communication

## Example Remote Command Session

Here's an example of what a typical remote command session looks like:

```
# In peer2's terminal:
peers
# Output: Connected peers: 1
#  Peer: 127.0.0.1:14595

@0 add
# Output: Result from peer 0: 3

@0 hello
# Output: Result from peer 0: Hello from peer1
```

## Output and Logs

The `automated_demo.sh` script creates a `network_test_output/` directory containing:
- `peer1_output.log`: Output from the first peer
- `peer2_output.log`: Output from the second peer

These logs are useful for debugging and understanding the network communication flow.

## Extending

You can extend these scripts and tests by:

1. Adding more commands to the configuration files
2. Creating new configuration files for different scenarios
3. Adding more complex command sequences to the test scripts
4. Implementing multi-peer networks for more complex testing

## Related Documentation

- See the main [Networking documentation](../../Doc/Networking.md) for details on the network architecture
- Check the [config README](../../config/Readme.md) for configuration file format details
- Review the [NetworkTest application](../../Source/App/NetworkTest/Readme.md) for implementation details