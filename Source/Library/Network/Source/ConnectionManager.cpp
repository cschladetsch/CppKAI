#include "KAI/Network/ConnectionManager.h"

#include <iostream>

#include "KAI/Network/ConnectionEvent.h"
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

    std::cout << "Added connection " << id << " for " << addrStr << std::endl;

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

        std::cout << "Removed connection " << id << std::endl;
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

        std::cout << "Removed connection for " << addrStr << std::endl;
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

        // Update ping
        info.ping = _peer->GetAveragePing(info.address);

        // Check if peer is still connected
        if (_peer->GetConnectionState(info.address) != RakNet::IS_CONNECTED) {
            if (info.state == ConnectionState::Connected) {
                info.state = ConnectionState::Disconnected;

                // Trigger disconnect callback
                if (_callback) {
                    _callback(id, ConnectionEvent::ConnectionLost);
                }

                timedOutConnections.push_back(id);
            }
        } else {
            // Connection is active, update last activity time
            info.lastActivity = currentTime;
        }

        // Check for timeout - if last activity is too old
        if (info.state == ConnectionState::Connected &&
            currentTime - info.lastActivity > _connectionTimeout) {
            // Connection timed out
            info.state = ConnectionState::Disconnected;

            if (_callback) {
                _callback(id, ConnectionEvent::Timeout);
            }

            timedOutConnections.push_back(id);
        }
    }

    // Remove timed out connections
    for (ConnectionId id : timedOutConnections) {
        RemoveConnection(id);
    }
}

ConnectionInfo* ConnectionManager::GetConnection(ConnectionId id) {
    auto it = _connections.find(id);
    if (it != _connections.end()) {
        return &it->second;
    }

    return nullptr;
}

ConnectionInfo* ConnectionManager::GetConnectionByAddress(
    const RakNet::SystemAddress& address) {
    std::string addrStr = address.ToString();
    auto it = _addressToId.find(addrStr);
    if (it != _addressToId.end()) {
        return GetConnection(it->second);
    }

    return nullptr;
}

const std::unordered_map<ConnectionManager::ConnectionId, ConnectionInfo>&
ConnectionManager::GetAllConnections() const {
    return _connections;
}

void ConnectionManager::SetConnectionCallback(ConnectionCallback callback) {
    _callback = callback;
}

size_t ConnectionManager::GetConnectionCount() const {
    return _connections.size();
}

bool ConnectionManager::IsConnected(ConnectionId id) const {
    auto it = _connections.find(id);
    if (it != _connections.end()) {
        return it->second.state == ConnectionState::Connected;
    }

    return false;
}

bool ConnectionManager::IsConnected(
    const RakNet::SystemAddress& address) const {
    std::string addrStr = address.ToString();
    auto it = _addressToId.find(addrStr);
    if (it != _addressToId.end()) {
        return IsConnected(it->second);
    }

    return false;
}

void ConnectionManager::OnConnectionEvent(const RakNet::SystemAddress& address,
                                          ConnectionEvent event) {
    std::string addrStr = address.ToString();
    auto it = _addressToId.find(addrStr);
    if (it != _addressToId.end()) {
        // Update connection state
        ConnectionId id = it->second;
        ConnectionInfo& info = _connections[id];

        switch (event) {
            case ConnectionEvent::Connected:
                info.state = ConnectionState::Connected;
                info.lastActivity = RakNet::GetTimeMS();
                break;

            case ConnectionEvent::Disconnected:
            case ConnectionEvent::ConnectionLost:
                info.state = ConnectionState::Disconnected;
                break;

            case ConnectionEvent::ConnectionFailed:
                info.state = ConnectionState::Failed;
                break;

            default:
                break;
        }

        // Call callback
        if (_callback) {
            _callback(id, event);
        }
    }
}

KAI_NET_END