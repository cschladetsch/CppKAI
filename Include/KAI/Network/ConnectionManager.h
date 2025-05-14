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
    std::vector<ConnectionId> GetAllConnections() const;

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

    // Update activity timestamp for a connection
    void UpdateActivity(ConnectionId id);

    // Update activity timestamp for a connection by address
    void UpdateActivity(const RakNet::SystemAddress& address);

    // Get connection state
    ConnectionState GetConnectionState(ConnectionId id) const;

    // Set connection state
    void SetConnectionState(ConnectionId id, ConnectionState state);

    // Get ping for a connection
    int GetPing(ConnectionId id) const;

    // Get system address for a connection ID
    RakNet::SystemAddress GetSystemAddress(ConnectionId id) const;

    // Get connection ID for a system address
    ConnectionId GetConnectionId(const RakNet::SystemAddress& address) const;

    // Set connection timeout
    void SetConnectionTimeout(RakNet::TimeMS timeout);

   private:
    RakNet::RakPeerInterface* peer_;
    std::unordered_map<ConnectionId, ConnectionInfo> connections_;
    // Custom hash and equal functions for the map since RakNet::SystemAddress
    // doesn't have a proper hash implementation
    std::map<std::string, ConnectionId>
        addressToId_;  // Use string representation as key
    ConnectionCallback callback_;
    ConnectionId nextId_;
    int64_t connectionTimeout_;  // Timeout in milliseconds
};

KAI_NET_END