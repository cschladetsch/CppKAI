#pragma once

#include <functional>
#include <map>
#include <string>
#include <unordered_map>

#include "KAI/Network/ConnectionEvent.h"
#include "KAI/Network/Network.h"
#include "KAI/Network/RakNetStub.h"  // Include the RakNetStub.h file directly

KAI_NET_BEGIN

// Connection states for peer connections
enum class ConnectionState { Disconnected, Connecting, Connected, Failed };

// Connection info for a remote peer
struct ConnectionInfo {
    RakNet::SystemAddress address;
    ConnectionState state;
    int64_t lastActivity;
    int ping;

    ConnectionInfo()
        : state(ConnectionState::Disconnected), lastActivity(0), ping(0) {}
};

// Connection manager to track peer connections
class ConnectionManager {
   public:
    using ConnectionId = int;
    using ConnectionCallback =
        std::function<void(ConnectionId, ConnectionEvent)>;

    ConnectionManager(RakNet::RakPeerInterface* peer);
    ~ConnectionManager();

    // Add a connection (after successful connection or incoming connection)
    ConnectionId AddConnection(const RakNet::SystemAddress& address);

    // Remove a connection
    void RemoveConnection(ConnectionId id);

    // Remove a connection by address
    void RemoveConnection(const RakNet::SystemAddress& address);

    // Update connection states and detect timeouts
    void Update();

    // Get a connection by ID
    ConnectionInfo* GetConnection(ConnectionId id);

    // Get a connection by system address
    ConnectionInfo* GetConnectionByAddress(
        const RakNet::SystemAddress& address);

    // Get all connections
    const std::unordered_map<ConnectionId, ConnectionInfo>& GetAllConnections()
        const;

    // Set callback for connection events
    void SetConnectionCallback(ConnectionCallback callback);

    // Get the number of active connections
    size_t GetConnectionCount() const;

    // Check if a specific peer is connected
    bool IsConnected(ConnectionId id) const;

    // Check if a specific peer is connected by address
    bool IsConnected(const RakNet::SystemAddress& address) const;

    // Handle connection events for a system address
    void OnConnectionEvent(const RakNet::SystemAddress& address,
                           ConnectionEvent event);

   private:
    RakNet::RakPeerInterface* _peer;
    std::unordered_map<ConnectionId, ConnectionInfo> _connections;
    // Custom hash and equal functions for the map since RakNet::SystemAddress
    // doesn't have a proper hash implementation
    std::map<std::string, ConnectionId>
        _addressToId;  // Use string representation as key
    ConnectionCallback _callback;
    ConnectionId _nextId;
    int64_t _connectionTimeout;  // Timeout in milliseconds
};

KAI_NET_END