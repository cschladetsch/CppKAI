#pragma once

#include "KAI/Network/RakNetStub.h"  // Include RakNetStub.h directly
#include "KAI/Network/FwdDeclarations.h"
#include "KAI/Network/Future.h"
#include "KAI/Network/NetHandle.h"

#include <any>
#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <utility>

KAI_NET_BEGIN

// Forward declarations for KAI network classes
enum class ConnectionEvent;
struct ProxyBase;
struct AgentBase;

namespace detail {
    struct MethodInvokerBase;
    struct PropertyAccessorBase;
}

// A peer on the network. Nominally, there are no servers or clients. Just
// a collection of nodes that connect and communicate with each other.
// Forward declarations
class ConnectionManager;
class PeerDiscovery;

struct Node {
   private:
    RakNet::RakPeerInterface *peer_;
    bool isRunning_;
    std::unique_ptr<ConnectionManager> connectionManager_;
    std::unique_ptr<PeerDiscovery> peerDiscovery_;

   public:
    static int constexpr DefaultPort = 14589;

    Node();
    ~Node();

    void Listen(int port);
    void Listen(IpAddress const &address, int port);
    void Connect(IpAddress const &, int port);
    void Disconnect();
    void Shutdown();

    bool IsRunning() const { return isRunning_; }
    bool Update();  // Process incoming messages, returns true if messages were
                    // processed

    // Peer discovery methods
    void StartDiscovery(int discoveryPort = DefaultPort);
    void StopDiscovery();
    bool IsDiscovering() const;
    std::vector<RakNet::SystemAddress> GetDiscoveredPeers() const;
    void SetPeerDiscoveryCallback(
        std::function<void(const RakNet::SystemAddress &)> callback);

    // Get all active connections
    std::vector<RakNet::SystemAddress> GetConnections() const;

    // Check if connected to a specific address
    bool IsConnectedTo(const IpAddress &address, int port) const;

    // Get connection count
    size_t GetConnectionCount() const;

    // Get ping to a specific address
    int GetPing(const IpAddress &address, int port) const;

   template <class T = void>
    Future<T> Send(NetHandle handle, const Object &obj);

    template <class T = void>
    Future<T> Receive(NetHandle handle, Object obj);

    // Broadcast an object to all connected peers
    template <class T = void>
    void Broadcast(const Object &obj);

    // Agent/Proxy support -------------------------------------------------
    NetHandle AttachAgent(AgentBase *agent);
    void DetachAgent(NetHandle handle, AgentBase *agent);

    template <typename R, typename... Args>
    void RegisterMethod(NetHandle handle, const std::string &name,
                        std::function<R(Args...)> fn);

    template <typename Value>
    void RegisterProperty(NetHandle handle, const std::string &name,
                          std::function<Value()> getter,
                          std::function<void(Value)> setter);

    template <typename Value>
    void RegisterProperty(NetHandle handle, const std::string &name,
                          std::function<Value()> getter);

    template <typename R, typename... Args>
    Future<R> Invoke(NetHandle handle, const std::string &name,
                     Args &&...args);

    template <typename P>
    Future<P> FetchProperty(NetHandle handle, const std::string &name);

    template <typename P>
    Future<void> StoreProperty(NetHandle handle, const std::string &name,
                               P &&value);

   private:
    void ProcessPacket(RakNet::Packet *packet);
    void ProcessObjectMessage(RakNet::Packet *packet);
    void ProcessFunctionCall(RakNet::Packet *packet);
    void ProcessEventNotification(RakNet::Packet *packet);
    void OnConnectionEvent(int connectionId, ConnectionEvent event);

    // Helper method to get the packet identifier
    unsigned char GetPacketIdentifier(RakNet::Packet *packet);

   private:
    // Use raw pointer for Registry to avoid build issues
    // std::shared_ptr<Registry> _reg;

   private:
    typedef std::unordered_map<NetHandle, std::shared_ptr<ProxyBase>,
                               HashNetHandle>
        Proxies;
    typedef std::unordered_map<NetHandle, std::shared_ptr<AgentBase>,
                               HashNetHandle>
        Agents;

    Agents agents_;
    Proxies proxies_;

    struct AgentEntry {
        AgentBase *agent = nullptr;
        std::unordered_map<std::string, std::shared_ptr<detail::MethodInvokerBase>>
            methods;
        std::unordered_map<std::string, std::shared_ptr<detail::PropertyAccessorBase>>
            properties;
    };

    std::unordered_map<int, AgentEntry> agentEntries_;
    std::mutex agentMutex_;
    std::atomic<int> nextHandle_{1};
    std::atomic<int> nextFutureId_{1};
};

// -----------------------------------------------------------------------------
// Inline implementation details

namespace detail {
    struct MethodInvokerBase {
        virtual ~MethodInvokerBase() = default;
        virtual std::any Invoke(const std::vector<std::any> &args) = 0;
    };

    template <typename R, typename... Args>
    struct MethodInvoker : MethodInvokerBase {
        explicit MethodInvoker(std::function<R(Args...)> fn)
            : fn_(std::move(fn)) {}

        std::any Invoke(const std::vector<std::any> &args) override {
            if (args.size() != sizeof...(Args)) {
                throw std::invalid_argument("Incorrect number of arguments");
            }
            return InvokeImpl(args, std::index_sequence_for<Args...>{});
        }

       private:
        template <std::size_t... Indices>
        std::any InvokeImpl(const std::vector<std::any> &args,
                            std::index_sequence<Indices...>) {
            if constexpr (std::is_void_v<R>) {
                fn_(std::any_cast<std::decay_t<Args>>(args[Indices])...);
                return std::any();
            } else {
                R result = fn_(std::any_cast<std::decay_t<Args>>(args[Indices])...);
                return std::any(std::move(result));
            }
        }

        std::function<R(Args...)> fn_;
    };

    struct PropertyAccessorBase {
        virtual ~PropertyAccessorBase() = default;
        virtual std::any Get() = 0;
        virtual void Set(const std::any &value) {
            KAI_UNUSED_1(value);
            throw std::runtime_error("Property is read-only");
        }
        virtual bool CanWrite() const { return false; }
        std::type_index type{typeid(void)};
    };

    template <typename Value>
    struct PropertyAccessor : PropertyAccessorBase {
        PropertyAccessor(std::function<Value()> getter,
                         std::function<void(Value)> setter)
            : getter_(std::move(getter)), setter_(std::move(setter)) {
            this->type = typeid(Value);
        }

        std::any Get() override { return std::any(getter_()); }

        void Set(const std::any &value) override {
            setter_(std::any_cast<Value>(value));
        }

        bool CanWrite() const override { return static_cast<bool>(setter_); }

        std::function<Value()> getter_;
        std::function<void(Value)> setter_;
    };

    template <typename Value>
    struct ReadOnlyAccessor : PropertyAccessorBase {
        explicit ReadOnlyAccessor(std::function<Value()> getter)
            : getter_(std::move(getter)) {
            this->type = typeid(Value);
        }

        std::any Get() override { return std::any(getter_()); }

        std::function<Value()> getter_;
    };
}  // namespace detail

inline NetHandle Node::AttachAgent(AgentBase *agent) {
    std::lock_guard<std::mutex> lock(agentMutex_);
    NetHandle handle(nextHandle_++);
    agentEntries_[handle.value].agent = agent;
    return handle;
}

inline void Node::DetachAgent(NetHandle handle, AgentBase *agent) {
    std::lock_guard<std::mutex> lock(agentMutex_);
    auto it = agentEntries_.find(handle.value);
    if (it != agentEntries_.end() && it->second.agent == agent) {
        agentEntries_.erase(it);
    }
}

template <typename R, typename... Args>
void Node::RegisterMethod(NetHandle handle, const std::string &name,
                          std::function<R(Args...)> fn) {
    std::lock_guard<std::mutex> lock(agentMutex_);
    auto &entry = agentEntries_[handle.value];
    entry.methods[name] =
        std::make_shared<detail::MethodInvoker<R, Args...>>(std::move(fn));
}

template <typename Value>
void Node::RegisterProperty(NetHandle handle, const std::string &name,
                            std::function<Value()> getter,
                            std::function<void(Value)> setter) {
    std::lock_guard<std::mutex> lock(agentMutex_);
    auto accessor =
        std::make_shared<detail::PropertyAccessor<Value>>(std::move(getter),
                                                          std::move(setter));
    agentEntries_[handle.value].properties[name] = accessor;
}

template <typename Value>
void Node::RegisterProperty(NetHandle handle, const std::string &name,
                            std::function<Value()> getter) {
    std::lock_guard<std::mutex> lock(agentMutex_);
    auto accessor =
        std::make_shared<detail::ReadOnlyAccessor<Value>>(std::move(getter));
    agentEntries_[handle.value].properties[name] = accessor;
}

template <typename R, typename... Args>
Future<R> Node::Invoke(NetHandle handle, const std::string &name,
                       Args &&...args) {
    Future<R> future;
    future.Id = nextFutureId_++;
    std::shared_ptr<detail::MethodInvokerBase> invoker;

    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        auto entry = agentEntries_.find(handle.value);
        if (entry == agentEntries_.end()) {
            future.Response = ResponseType::UnkownAgent;
            future.Complete = true;
            future.ErrorMessage = "Unknown agent";
            return future;
        }
        auto it = entry->second.methods.find(name);
        if (it == entry->second.methods.end()) {
            future.Response = ResponseType::BadRequest;
            future.Complete = true;
            future.ErrorMessage = "Unknown method";
            return future;
        }
        invoker = it->second;
    }

    std::vector<std::any> packedArgs{std::forward<Args>(args)...};

    try {
        std::any result = invoker->Invoke(packedArgs);
        future.Response = ResponseType::Returned;
        future.Complete = true;
        if constexpr (!std::is_void_v<R>) {
            future.Value = std::any_cast<R>(result);
        }
    } catch (const std::bad_any_cast &e) {
        future.Response = ResponseType::BadRequest;
        future.Complete = true;
        future.ErrorMessage = e.what();
    } catch (const std::exception &e) {
        future.Response = ResponseType::BadRequest;
        future.Complete = true;
        future.ErrorMessage = e.what();
    }

    return future;
}

template <typename P>
Future<P> Node::FetchProperty(NetHandle handle, const std::string &name) {
    Future<P> future;
    future.Id = nextFutureId_++;
    std::shared_ptr<detail::PropertyAccessorBase> accessor;

    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        auto entry = agentEntries_.find(handle.value);
        if (entry == agentEntries_.end()) {
            future.Response = ResponseType::UnkownAgent;
            future.Complete = true;
            future.ErrorMessage = "Unknown agent";
            return future;
        }
        auto it = entry->second.properties.find(name);
        if (it == entry->second.properties.end()) {
            future.Response = ResponseType::BadRequest;
            future.Complete = true;
            future.ErrorMessage = "Unknown property";
            return future;
        }
        accessor = it->second;
    }

    try {
        std::any value = accessor->Get();
        future.Response = ResponseType::Returned;
        future.Complete = true;
        future.Value = std::any_cast<P>(value);
    } catch (const std::bad_any_cast &e) {
        future.Response = ResponseType::BadRequest;
        future.Complete = true;
        future.ErrorMessage = e.what();
    }

    return future;
}

template <typename P>
Future<void> Node::StoreProperty(NetHandle handle, const std::string &name,
                                 P &&value) {
    Future<void> future;
    future.Id = nextFutureId_++;
    std::shared_ptr<detail::PropertyAccessorBase> accessor;

    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        auto entry = agentEntries_.find(handle.value);
        if (entry == agentEntries_.end()) {
            future.Response = ResponseType::UnkownAgent;
            future.Complete = true;
            future.ErrorMessage = "Unknown agent";
            return future;
        }
        auto it = entry->second.properties.find(name);
        if (it == entry->second.properties.end()) {
            future.Response = ResponseType::BadRequest;
            future.Complete = true;
            future.ErrorMessage = "Unknown property";
            return future;
        }
        accessor = it->second;
    }

    try {
        accessor->Set(std::any(std::forward<P>(value)));
        future.Response = ResponseType::Returned;
        future.Complete = true;
    } catch (const std::exception &e) {
        future.Response = ResponseType::BadRequest;
        future.Complete = true;
        future.ErrorMessage = e.what();
    }

    return future;
}
KAI_NET_END
