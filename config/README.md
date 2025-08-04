# KAI Network Configuration Files

This directory contains configuration files for the peer-to-peer network functionality in KAI's networking system.

## Overview

The NetworkPeer application uses JSON configuration files to specify:
- Network ports and listening behavior
- Connections to other peers
- Pre-defined commands for remote execution
- Interactive mode settings

Using these configuration files, you can quickly set up peers in different configurations for testing and development.

## Available Configuration Files

- **peer1_config.json**: Server-like peer that listens for connections and provides commands
- **peer2_config.json**: Client-like peer that connects to peer1 and can execute commands remotely
- **minimal_config.json**: Minimalist configuration for basic testing

## Configuration Format Reference

```json
{
    "listenPort": 14595,       // Port to listen on (integer)
    "autoListen": true,        // Whether to automatically start listening (boolean)
    "interactive": true,       // Whether to accept commands from stdin (boolean)
    "peers": [                 // Array of peers to connect to
        {
            "host": "127.0.0.1",  // Hostname or IP address (string)
            "port": 14595          // Port number (integer)
        }
    ],
    "commands": {              // Dictionary of commands available for remote execution
        "add": "calc 1+2",     // Command name: command to execute
        "hello": "echo Hello World",
        "list_files": "ls -la"
    }
}
```

### Configuration Options

| Option | Type | Description |
|--------|------|-------------|
| `listenPort` | Integer | The port number to listen on for incoming connections |
| `autoListen` | Boolean | If true, automatically starts listening on startup |
| `interactive` | Boolean | If true, accepts commands from standard input |
| `peers` | Array | List of peers to connect to on startup |
| `peers[].host` | String | Hostname or IP address of a peer |
| `peers[].port` | Integer | Port number of a peer |
| `commands` | Object | Dictionary mapping command names to their implementations |

## Usage Guide

### Basic Usage

To use these configuration files, run the NetworkPeer executable:

```bash
./Bin/NetworkPeer /path/to/config.json
```

For example:
```bash
cd /home/xian/local/KAI/build && ./Bin/NetworkPeer ../config/peer1_config.json
```

### Interactive Commands

When running in interactive mode, the NetworkPeer application supports these commands:

- `help`: Display available commands
- `peers`: List connected peers
- `exit`: Exit the application
- `@<peer_index> <command>`: Execute a command on a remote peer

### Remote Command Execution

To execute a command on a remote peer, use the syntax:

```
@peer_index command_name
```

For example, to execute the "add" command on peer 0:
```
@0 add
```

This will execute the "calc 1+2" command on the remote peer and return the result.

## Testing and Examples

The network functionality can be tested using the scripts in the `Scripts/network` directory:

- `run_peers.sh`: Instructions for running two peers in separate terminals for interactive testing
- `automated_demo.sh`: Automated test of the peer-to-peer functionality

## Advanced Configuration

### Custom Command Examples

```json
{
    "commands": {
        "system_info": "uname -a",
        "memory_usage": "free -h",
        "calculate": "calc",
        "update_time": "date > /tmp/last_update.txt && echo 'Time updated'"
    }
}
```

### Multiple Peer Connections

```json
{
    "peers": [
        {"host": "192.168.1.10", "port": 14595},
        {"host": "192.168.1.11", "port": 14595},
        {"host": "10.0.0.5", "port": 14600}
    ]
}
```

## Troubleshooting

- If connections fail, ensure the specified ports are open and not blocked by firewalls
- For connection issues on localhost, try using "127.0.0.1" explicitly rather than "localhost"
- If commands fail to execute remotely, check that they are properly defined in the server peer's config