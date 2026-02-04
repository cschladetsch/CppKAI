#include "KAI/Network/ConnectionManager.h"

#include <chrono>
#include <iostream>
#include <vector>

#include "KAI/Network/ConnectionEvent.h"
#include "KAI/Network/NetworkLogger.h"

KAI_NET_BEGIN

namespace {
int64_t NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}
}  // namespace

ConnectionManager::ConnectionManager(NetPeer* peer)
    : peer_(peer),
      nextId_(1),
      connectionTimeout_(30000)  // 30 seconds timeout
{}

ConnectionManager::~ConnectionManager() {
    // Clear all connections
    connections_.clear();
    addressToId_.clear();
}

ConnectionManager::ConnectionId ConnectionManager::AddConnection(
    const NetAddress& address) {
    if (!peer_) return 0;

    std::string addrStr = address.ToString();

    // Check if we already have this connection
    auto it = addressToId_.find(addrStr);
    if (it != addressToId_.end()) {
        return it->second;
    }

    // Create a new connection
    ConnectionId id = nextId_++;
    ConnectionInfo info;
    info.address = address;
    info.state = ConnectionState::Connected;
    info.lastActivity = NowMs();
    info.ping = peer_->GetAveragePing(address);

    // Add to our maps
    connections_[id] = info;
    addressToId_[addrStr] = id;

    // Create log message and log it
    std::string logMessage =
        "Added connection " + std::to_string(id) + " for " + addrStr;
    std::cout << logMessage << std::endl;
    NetworkLogger::LogConnection(logMessage);

    return id;
}

void ConnectionManager::RemoveConnection(ConnectionId id) {
    auto it = connections_.find(id);
    if (it != connections_.end()) {
        // Remove from address map
        std::string addrStr = it->second.address.ToString();
        addressToId_.erase(addrStr);

        // Remove from connections
        connections_.erase(it);

        // Log the removal
        std::string logMessage = "Removed connection " + std::to_string(id);
        std::cout << logMessage << std::endl;
        NetworkLogger::LogConnection(logMessage);
    }
}

void ConnectionManager::RemoveConnection(const NetAddress& address) {
    std::string addrStr = address.ToString();
    auto it = addressToId_.find(addrStr);
    if (it != addressToId_.end()) {
        ConnectionId id = it->second;

        // Remove from maps
        addressToId_.erase(it);
        connections_.erase(id);

        // Log the removal
        std::string logMessage = "Removed connection for " + addrStr;
        std::cout << logMessage << std::endl;
        NetworkLogger::LogConnection(logMessage);
    }
}

void ConnectionManager::Update() {
    if (!peer_) return;

    // Update ping times and check for timeouts
    int64_t currentTime = NowMs();

    std::vector<ConnectionId> timedOutConnections;

    for (auto& pair : connections_) {
        ConnectionId id = pair.first;
        ConnectionInfo& info = pair.second;

        // Update ping time
        info.ping = peer_->GetAveragePing(info.address);

        // Check for timeout
        if (currentTime - info.lastActivity > connectionTimeout_) {
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
    auto it = connections_.find(id);
    if (it != connections_.end()) {
        it->second.lastActivity = NowMs();
    }
}

void ConnectionManager::UpdateActivity(const NetAddress& address) {
    std::string addrStr = address.ToString();
    auto it = addressToId_.find(addrStr);
    if (it != addressToId_.end()) {
        UpdateActivity(it->second);
    }
}

ConnectionState ConnectionManager::GetConnectionState(ConnectionId id) const {
    auto it = connections_.find(id);
    if (it != connections_.end()) {
        return it->second.state;
    }
    return ConnectionState::Disconnected;
}

void ConnectionManager::SetConnectionState(ConnectionId id,
                                           ConnectionState state) {
    auto it = connections_.find(id);
    if (it != connections_.end()) {
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
    auto it = connections_.find(id);
    if (it != connections_.end()) {
        return it->second.ping;
    }
    return -1;
}

NetAddress ConnectionManager::GetSystemAddress(ConnectionId id) const {
    auto it = connections_.find(id);
    if (it != connections_.end()) {
        return it->second.address;
    }
    return NetAddress();
}

ConnectionManager::ConnectionId ConnectionManager::GetConnectionId(
    const NetAddress& address) const {
    std::string addrStr = address.ToString();
    auto it = addressToId_.find(addrStr);
    if (it != addressToId_.end()) {
        return it->second;
    }
    return 0;  // 0 is invalid connection ID
}

std::vector<ConnectionManager::ConnectionId>
ConnectionManager::GetAllConnections() const {
    std::vector<ConnectionId> result;
    for (const auto& pair : connections_) {
        result.push_back(pair.first);
    }
    return result;
}

size_t ConnectionManager::GetConnectionCount() const {
    return connections_.size();
}

void ConnectionManager::SetConnectionTimeout(int64_t timeout) {
    connectionTimeout_ = timeout;
}

KAI_NET_END
