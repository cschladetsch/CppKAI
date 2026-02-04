#pragma once

#include "KAI/Network/FwdDeclarations.h"
#include "KAI/Network/Future.h"
#include "KAI/Network/NetHandle.h"
#include "KAI/Network/Transport.h"
#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/BinaryStream.h"
#include "KAI/Core/Object.h"
#include "KAI/Core/Value.h"

#include <any>
#include <atomic>
#include <functional>
#include <utility>
#include <mutex>
#include <memory>
#include <optional>
#include <stdexcept>
#include <chrono>
#include <thread>
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
    std::unique_ptr<NetPeer> peer_;
    bool isRunning_;
    Registry *registry_;
    std::unique_ptr<ConnectionManager> connectionManager_;
    std::unique_ptr<PeerDiscovery> peerDiscovery_;
    std::function<void()> updatePump_;

   public:
    static int constexpr DefaultPort = 14589;

    Node();
    ~Node();

    void SetRegistry(Registry *registry) { registry_ = registry; }
    Registry *GetRegistry() const { return registry_; }

    void SetUpdatePump(std::function<void()> pump) { updatePump_ = std::move(pump); }

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
    std::vector<NetAddress> GetDiscoveredPeers() const;
    void SetPeerDiscoveryCallback(
        std::function<void(const NetAddress &)> callback);

    // Get all active connections
    std::vector<NetAddress> GetConnections() const;

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

    template <typename T>
    T WaitFor(Future<T> &future,
              std::chrono::milliseconds timeout =
                  std::chrono::milliseconds(5000));

    void WaitFor(Future<void> &future,
                 std::chrono::milliseconds timeout =
                     std::chrono::milliseconds(5000));

    // Compatibility helpers for generated agents
    void SendResponse(const NetAddress &peer, BinaryStream &response);
    void BroadcastEvent(const std::string &name,
                        BinaryStream &eventData);
    void BroadcastEvent(const std::string &name);

    template <typename P>
    Future<P> FetchProperty(NetHandle handle, const std::string &name);

    template <typename P>
    Future<void> StoreProperty(NetHandle handle, const std::string &name,
                               P &&value);

   private:
    void ProcessPacket(const NetPacket &packet);
    void ProcessObjectMessage(const NetPacket &packet);
    void ProcessFunctionCall(const NetPacket &packet);
    void ProcessEventNotification(const NetPacket &packet);
    void ProcessFunctionResponse(const NetPacket &packet);
    void OnConnectionEvent(int connectionId, ConnectionEvent event);
    void SendFunctionCall(NetHandle handle, const std::string &name,
                          const Object &args, int futureId);

    // Helper method to get the packet identifier
    unsigned char GetPacketIdentifier(const NetPacket &packet);

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

    struct PendingResponse {
        std::function<void(const Object &, ResponseType,
                           const std::string &)>
            complete;
    };

    std::unordered_map<int, PendingResponse> pendingResponses_;
    std::mutex pendingMutex_;
};

// -----------------------------------------------------------------------------
// Inline implementation details

namespace detail {
    struct MethodInvokerBase {
        virtual ~MethodInvokerBase() = default;
        virtual Object Invoke(const std::vector<Object> &args) = 0;
    };

    template <typename R, typename... Args>
    struct MethodInvoker : MethodInvokerBase {
        MethodInvoker(Registry *registry, std::function<R(Args...)> fn)
            : registry_(registry), fn_(std::move(fn)) {}

        Object Invoke(const std::vector<Object> &args) override {
            if (args.size() != sizeof...(Args)) {
                throw std::invalid_argument("Incorrect number of arguments");
            }
            return InvokeImpl(args, std::index_sequence_for<Args...>{});
        }

       private:
        template <typename T>
        static std::decay_t<T> ExtractArg(const Object &obj) {
            if constexpr (std::is_same_v<std::decay_t<T>, Object>) {
                return obj;
            } else {
                return ConstDeref<std::decay_t<T>>(obj);
            }
        }

        template <std::size_t... Indices>
        Object InvokeImpl(const std::vector<Object> &args,
                          std::index_sequence<Indices...>) {
            if constexpr (std::is_void_v<R>) {
                fn_(ExtractArg<Args>(args[Indices])...);
                return Object();
            } else {
                if (!registry_) {
                    throw std::runtime_error("Null registry for return value");
                }
                R result = fn_(ExtractArg<Args>(args[Indices])...);
                return registry_->New(result);
            }
        }

        Registry *registry_;
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
    entry.methods[name] = std::make_shared<detail::MethodInvoker<R, Args...>>(
        registry_, std::move(fn));
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
    future.SetId(nextFutureId_++);
    std::shared_ptr<detail::MethodInvokerBase> invoker;

    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        auto entry = agentEntries_.find(handle.value);
        if (entry != agentEntries_.end()) {
            auto it = entry->second.methods.find(name);
            if (it != entry->second.methods.end()) {
                invoker = it->second;
            }
        }
    }

    if (invoker) {
        try {
            std::vector<Object> packedArgs;
            packedArgs.reserve(sizeof...(Args));
            if (!registry_) {
                throw std::runtime_error("Null registry for local invoke");
            }
            if constexpr (sizeof...(Args) > 0) {
                (packedArgs.emplace_back(
                     registry_->New(std::forward<Args>(args))),
                 ...);
            }

            Object result = invoker->Invoke(packedArgs);
            future.SetResponse(ResponseType::Returned);
            future.SetComplete(true);
            if constexpr (!std::is_void_v<R>) {
                if constexpr (std::is_same_v<std::decay_t<R>, Object>) {
                    future.SetValue(result);
                } else {
                    future.SetValue(ConstDeref<std::decay_t<R>>(result));
                }
            }
        } catch (const std::exception &e) {
            future.SetResponse(ResponseType::BadRequest);
            future.SetComplete(true);
            future.SetErrorMessage(e.what());
        }
        return future;
    }

    // Remote path - serialize args and send a function call request.
    if (!registry_) {
        future.SetResponse(ResponseType::BadRequest);
        future.SetComplete(true);
        future.SetErrorMessage("Null registry for remote invoke");
        return future;
    }

    Value<Array> argsArray = registry_->New<Array>();
    if constexpr (sizeof...(Args) > 0) {
        (argsArray->Append(registry_->New(std::forward<Args>(args))), ...);
    }

    Object argsObject = argsArray.GetObject();

    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        auto state = future.GetState();
        pendingResponses_[future.GetId()].complete =
            [state](const Object &obj, ResponseType response,
                    const std::string &error) {
                state->Response = response;
                state->Complete = true;
                state->ErrorMessage = error;
                if constexpr (!std::is_void_v<R>) {
                    if (response == ResponseType::Returned) {
                        if constexpr (std::is_same_v<std::decay_t<R>, Object>) {
                            state->Value = obj;
                        } else {
                            state->Value = ConstDeref<std::decay_t<R>>(obj);
                        }
                    }
                }
            };
    }

    // Implemented in Node.cpp
    SendFunctionCall(handle, name, argsObject, future.GetId());

    return future;
}

template <typename P>
Future<P> Node::FetchProperty(NetHandle handle, const std::string &name) {
    Future<P> future;
    future.SetId(nextFutureId_++);
    std::shared_ptr<detail::PropertyAccessorBase> accessor;

    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        auto entry = agentEntries_.find(handle.value);
        if (entry == agentEntries_.end()) {
            future.SetResponse(ResponseType::UnkownAgent);
            future.SetComplete(true);
            future.SetErrorMessage("Unknown agent");
            return future;
        }
        auto it = entry->second.properties.find(name);
        if (it == entry->second.properties.end()) {
            future.SetResponse(ResponseType::BadRequest);
            future.SetComplete(true);
            future.SetErrorMessage("Unknown property");
            return future;
        }
        accessor = it->second;
    }

    try {
        std::any value = accessor->Get();
        future.SetResponse(ResponseType::Returned);
        future.SetComplete(true);
        future.SetValue(std::any_cast<P>(value));
    } catch (const std::bad_any_cast &e) {
        future.SetResponse(ResponseType::BadRequest);
        future.SetComplete(true);
        future.SetErrorMessage(e.what());
    }

    return future;
}

template <typename P>
Future<void> Node::StoreProperty(NetHandle handle, const std::string &name,
                                 P &&value) {
    Future<void> future;
    future.SetId(nextFutureId_++);
    std::shared_ptr<detail::PropertyAccessorBase> accessor;

    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        auto entry = agentEntries_.find(handle.value);
        if (entry == agentEntries_.end()) {
            future.SetResponse(ResponseType::UnkownAgent);
            future.SetComplete(true);
            future.SetErrorMessage("Unknown agent");
            return future;
        }
        auto it = entry->second.properties.find(name);
        if (it == entry->second.properties.end()) {
            future.SetResponse(ResponseType::BadRequest);
            future.SetComplete(true);
            future.SetErrorMessage("Unknown property");
            return future;
        }
        accessor = it->second;
    }

    try {
        accessor->Set(std::any(std::forward<P>(value)));
        future.SetResponse(ResponseType::Returned);
        future.SetComplete(true);
    } catch (const std::exception &e) {
        future.SetResponse(ResponseType::BadRequest);
        future.SetComplete(true);
        future.SetErrorMessage(e.what());
    }

    return future;
}

template <typename T>
T Node::WaitFor(Future<T> &future, std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();
    while (!future.IsComplete()) {
        Update();
        if (updatePump_) updatePump_();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (timeout.count() >= 0 &&
            std::chrono::steady_clock::now() - start > timeout) {
            future.SetResponse(ResponseType::TimedOut);
            future.SetComplete(true);
            break;
        }
    }
    return future.GetValue();
}

inline void Node::WaitFor(Future<void> &future,
                          std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();
    while (!future.IsComplete()) {
        Update();
        if (updatePump_) updatePump_();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (timeout.count() >= 0 &&
            std::chrono::steady_clock::now() - start > timeout) {
            future.SetResponse(ResponseType::TimedOut);
            future.SetComplete(true);
            break;
        }
    }
}
KAI_NET_END
