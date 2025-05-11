#include "KAI/Network/ConnectionManager.h"

#include <iostream>

#include "KAI/Network/ConnectionEvent.h"
#include "KAI/Network/NetworkLogger.h"
#include "KAI/Network/RakNetStub.h"

KAI_NET_BEGIN

ConnectionManager::ConnectionManager(RakNet::RakPeerInterface* peer)
    : _peer(peer),
      _nextId(1),
      _connectionTimeout(30000)  // 30 seconds timeout
{}

ConnectionManager::~ConnectionManager() {
    // Clear all connections
    _connections.clear();
    _addressToId.clear();
}

ConnectionManager::ConnectionId ConnectionManager::AddConnection(
    const RakNet::SystemAddress& address) {
    if (!_peer) return 0;

    // Convert SystemAddress to string for map lookup
    std::string addrStr = address.ToString();

    // Check if we already have this connection
    auto it = _addressToId.find(addrStr);
    if (it != _addressToId.end()) {
        return it->second;
    }

    // Create a new connection
    ConnectionId id = _nextId++;
    ConnectionInfo info;
    info.address = address;
    info.state = ConnectionState::Connected;
    info.lastActivity = RakNet::GetTimeMS();
    info.ping = _peer->GetAveragePing(address);

    // Add to our maps
    _connections[id] = info;
    _addressToId[addrStr] = id;

    // Create log message and log it
    std::string logMessage =
        "Added connection " + std::to_string(id) + " for " + addrStr;
    std::cout << logMessage << std::endl;
    NetworkLogger::LogConnection(logMessage);

    return id;
}

void ConnectionManager::RemoveConnection(ConnectionId id) {
    auto it = _connections.find(id);
    if (it != _connections.end()) {
        // Remove from address map
        std::string addrStr = it->second.address.ToString();
        _addressToId.erase(addrStr);

        // Remove from connections
        _connections.erase(it);

        // Log the removal
        std::string logMessage = "Removed connection " + std::to_string(id);
        std::cout << logMessage << std::endl;
        NetworkLogger::LogConnection(logMessage);
    }
}

void ConnectionManager::RemoveConnection(const RakNet::SystemAddress& address) {
    std::string addrStr = address.ToString();
    auto it = _addressToId.find(addrStr);
    if (it != _addressToId.end()) {
        ConnectionId id = it->second;

        // Remove from maps
        _addressToId.erase(it);
        _connections.erase(id);

        // Log the removal
        std::string logMessage = "Removed connection for " + addrStr;
        std::cout << logMessage << std::endl;
        NetworkLogger::LogConnection(logMessage);
    }
}

void ConnectionManager::Update() {
    if (!_peer) return;

    // Update ping times and check for timeouts
    RakNet::TimeMS currentTime = RakNet::GetTimeMS();

    std::vector<ConnectionId> timedOutConnections;

    for (auto& pair : _connections) {
        ConnectionId id = pair.first;
        ConnectionInfo& info = pair.second;

        // Update ping time
        info.ping = _peer->GetAveragePing(info.address);

        // Check for timeout
        if (currentTime - info.lastActivity > _connectionTimeout) {
            timedOutConnections.push_back(id);
        }
    }

    // Remove timed out connections
    for (auto id : timedOutConnections) {
        std::string logMessage =
            "Connection " + std::to_string(id) + " timed out";
        std::cout << logMessage << std::endl;
        NetworkLogger::LogConnection(logMessage);

        RemoveConnection(id);
    }
}

void ConnectionManager::UpdateActivity(ConnectionId id) {
    auto it = _connections.find(id);
    if (it != _connections.end()) {
        it->second.lastActivity = RakNet::GetTimeMS();
    }
}

void ConnectionManager::UpdateActivity(const RakNet::SystemAddress& address) {
    std::string addrStr = address.ToString();
    auto it = _addressToId.find(addrStr);
    if (it != _addressToId.end()) {
        UpdateActivity(it->second);
    }
}

ConnectionState ConnectionManager::GetConnectionState(ConnectionId id) const {
    auto it = _connections.find(id);
    if (it != _connections.end()) {
        return it->second.state;
    }
    return ConnectionState::Disconnected;
}

void ConnectionManager::SetConnectionState(ConnectionId id,
                                           ConnectionState state) {
    auto it = _connections.find(id);
    if (it != _connections.end()) {
        it->second.state = state;

        // Log the state change
        std::string stateStr;
        switch (state) {
            case ConnectionState::Connected:
                stateStr = "Connected";
                break;
            case ConnectionState::Connecting:
                stateStr = "Connecting";
                break;
            case ConnectionState::Disconnected:
                stateStr = "Disconnected";
                break;
            case ConnectionState::Failed:
                stateStr = "Failed";
                break;
            default:
                stateStr = "Unknown";
                break;
        }

        std::string logMessage = "Connection " + std::to_string(id) +
                                 " state changed to " + stateStr;
        NetworkLogger::LogConnection(logMessage);
    }
}

int ConnectionManager::GetPing(ConnectionId id) const {
    auto it = _connections.find(id);
    if (it != _connections.end()) {
        return it->second.ping;
    }
    return -1;
}

RakNet::SystemAddress ConnectionManager::GetSystemAddress(
    ConnectionId id) const {
    auto it = _connections.find(id);
    if (it != _connections.end()) {
        return it->second.address;
    }
    return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
}

ConnectionManager::ConnectionId ConnectionManager::GetConnectionId(
    const RakNet::SystemAddress& address) const {
    std::string addrStr = address.ToString();
    auto it = _addressToId.find(addrStr);
    if (it != _addressToId.end()) {
        return it->second;
    }
    return 0;  // 0 is invalid connection ID
}

std::vector<ConnectionManager::ConnectionId>
ConnectionManager::GetAllConnections() const {
    std::vector<ConnectionId> result;
    for (const auto& pair : _connections) {
        result.push_back(pair.first);
    }
    return result;
}

size_t ConnectionManager::GetConnectionCount() const {
    return _connections.size();
}

void ConnectionManager::SetConnectionTimeout(RakNet::TimeMS timeout) {
    _connectionTimeout = timeout;
}

KAI_NET_END