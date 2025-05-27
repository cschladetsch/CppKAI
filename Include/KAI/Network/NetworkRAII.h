#pragma once

// RAII wrappers and smart pointer usage for KAI Network components

#include <KAI/Network/Agent.h>
#include <KAI/Network/Connection.h>
#include <KAI/Network/Node.h>
#include <KAI/Network/Proxy.h>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

KAI_BEGIN

namespace Network {

// Forward declarations
class ConnectionManager_SmartPtr;
class Node_SmartPtr;

// RAII wrapper for network connections
class ManagedConnection {
   public:
    using ConnectionPtr = std::shared_ptr<Connection>;

   private:
    ConnectionPtr connection_;
    std::weak_ptr<ConnectionManager_SmartPtr> manager_;
    bool is_active_ = true;

   public:
    ManagedConnection(ConnectionPtr conn,
                      std::weak_ptr<ConnectionManager_SmartPtr> mgr)
        : connection_(conn), manager_(mgr) {}

    ~ManagedConnection() { Disconnect(); }

    // Prevent copying
    ManagedConnection(const ManagedConnection&) = delete;
    ManagedConnection& operator=(const ManagedConnection&) = delete;

    // Allow moving
    ManagedConnection(ManagedConnection&& other) noexcept
        : connection_(std::move(other.connection_)),
          manager_(std::move(other.manager_)),
          is_active_(other.is_active_) {
        other.is_active_ = false;
    }

    void Disconnect() {
        if (is_active_ && connection_) {
            connection_->Close();
            if (auto mgr = manager_.lock()) {
                // Notify manager of disconnection
            }
            is_active_ = false;
        }
    }

    ConnectionPtr Get() const { return connection_; }
    Connection* operator->() const { return connection_.get(); }
    Connection& operator*() const { return *connection_; }

    bool IsActive() const {
        return is_active_ && connection_ && connection_->IsConnected();
    }
};

// Thread-safe connection manager using smart pointers
class ConnectionManager_SmartPtr
    : public std::enable_shared_from_this<ConnectionManager_SmartPtr> {
   public:
    using ConnectionPtr = std::shared_ptr<Connection>;
    using ManagedConnectionPtr = std::unique_ptr<ManagedConnection>;

   private:
    std::unordered_map<Handle, ConnectionPtr> connections_;
    mutable std::mutex mutex_;
    Handle next_handle_{1};

   public:
    // Factory method to create managed connections
    ManagedConnectionPtr CreateConnection(const Address& addr) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto handle = next_handle_++;
        auto conn = std::make_shared<Connection>(handle, addr);
        connections_[handle] = conn;

        return std::make_unique<ManagedConnection>(conn, shared_from_this());
    }

    // Get existing connection
    ConnectionPtr GetConnection(Handle handle) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = connections_.find(handle);
        return (it != connections_.end()) ? it->second : nullptr;
    }

    // Remove connection
    void RemoveConnection(Handle handle) {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.erase(handle);
    }

    // Get all active connections
    std::vector<ConnectionPtr> GetActiveConnections() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<ConnectionPtr> active;

        for (const auto& [handle, conn] : connections_) {
            if (conn && conn->IsConnected()) {
                active.push_back(conn);
            }
        }

        return active;
    }

    // Cleanup dead connections
    void CleanupConnections() {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = connections_.begin();
        while (it != connections_.end()) {
            if (!it->second || !it->second->IsConnected()) {
                it = connections_.erase(it);
            } else {
                ++it;
            }
        }
    }
};

// Smart pointer-based Node implementation
class Node_SmartPtr : public std::enable_shared_from_this<Node_SmartPtr> {
   public:
    using AgentPtr = std::shared_ptr<AgentBase>;
    using ProxyPtr = std::shared_ptr<ProxyBase>;
    using ConnectionManagerPtr = std::shared_ptr<ConnectionManager_SmartPtr>;

   private:
    std::unordered_map<Type::Number, AgentPtr> agents_;
    std::unordered_map<Type::Number, ProxyPtr> proxies_;
    ConnectionManagerPtr connection_manager_;
    std::shared_ptr<Registry> registry_;
    mutable std::mutex mutex_;

   public:
    Node_SmartPtr(std::shared_ptr<Registry> reg)
        : registry_(reg),
          connection_manager_(std::make_shared<ConnectionManager_SmartPtr>()) {}

    // Register an agent with automatic lifetime management
    template <class T>
    void RegisterAgent() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto agent = std::make_shared<Agent<T>>(shared_from_this());
        agents_[Type::Traits<T>::Number] = agent;
    }

    // Register a proxy with automatic lifetime management
    template <class T>
    void RegisterProxy() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto proxy = std::make_shared<Proxy<T>>(shared_from_this());
        proxies_[Type::Traits<T>::Number] = proxy;
    }

    // Get agent by type
    template <class T>
    std::shared_ptr<Agent<T>> GetAgent() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = agents_.find(Type::Traits<T>::Number);
        if (it != agents_.end()) {
            return std::dynamic_pointer_cast<Agent<T>>(it->second);
        }
        return nullptr;
    }

    // Get proxy by type
    template <class T>
    std::shared_ptr<Proxy<T>> GetProxy() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = proxies_.find(Type::Traits<T>::Number);
        if (it != proxies_.end()) {
            return std::dynamic_pointer_cast<Proxy<T>>(it->second);
        }
        return nullptr;
    }

    // Create connection with RAII
    ManagedConnection::ManagedConnectionPtr Connect(const Address& addr) {
        return connection_manager_->CreateConnection(addr);
    }

    ConnectionManagerPtr GetConnectionManager() const {
        return connection_manager_;
    }
};

// RAII wrapper for peer discovery
class PeerDiscoverySession {
   private:
    std::shared_ptr<PeerDiscovery> discovery_;
    bool is_active_ = true;

   public:
    explicit PeerDiscoverySession(std::shared_ptr<Node_SmartPtr> node)
        : discovery_(std::make_shared<PeerDiscovery>(node)) {
        discovery_->Start();
    }

    ~PeerDiscoverySession() { Stop(); }

    // Prevent copying
    PeerDiscoverySession(const PeerDiscoverySession&) = delete;
    PeerDiscoverySession& operator=(const PeerDiscoverySession&) = delete;

    void Stop() {
        if (is_active_ && discovery_) {
            discovery_->Stop();
            is_active_ = false;
        }
    }

    std::vector<Address> GetDiscoveredPeers() const {
        return discovery_ ? discovery_->GetPeers() : std::vector<Address>{};
    }
};

// Scoped network operation for exception safety
template <class Operation>
class ScopedNetworkOperation {
   private:
    Operation cleanup_;
    bool dismissed_ = false;

   public:
    explicit ScopedNetworkOperation(Operation op) : cleanup_(std::move(op)) {}

    ~ScopedNetworkOperation() {
        if (!dismissed_) {
            try {
                cleanup_();
            } catch (...) {
                // Log error but don't throw from destructor
            }
        }
    }

    void Dismiss() { dismissed_ = true; }
};

// Helper factory function
template <class Operation>
auto MakeScopedNetworkOperation(Operation op) {
    return ScopedNetworkOperation<Operation>(std::move(op));
}

}  // namespace Network

KAI_END

// Usage examples:
/*
// Old way with manual management:
auto node = new Network::Node(registry);
auto conn = node->Connect(address);
// ... use connection ...
delete conn;  // Manual cleanup
delete node;  // Manual cleanup

// New way with RAII and smart pointers:
auto node = std::make_shared<Network::Node_SmartPtr>(registry);
{
    auto conn = node->Connect(address);  // RAII connection
    // ... use connection ...
} // Connection automatically closed and cleaned up

// Peer discovery with RAII:
{
    Network::PeerDiscoverySession discovery(node);
    auto peers = discovery.GetDiscoveredPeers();
    // ... use peers ...
} // Discovery automatically stopped

// Scoped operations for exception safety:
auto cleanup = Network::MakeScopedNetworkOperation([&]() {
    connection_manager->CleanupConnections();
});
// ... perform network operations ...
if (success) {
    cleanup.Dismiss();  // Don't run cleanup
}
// Cleanup runs automatically if not dismissed
*/