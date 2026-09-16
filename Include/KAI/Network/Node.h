#pragma once

#include <any>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "KAI/Core/BinaryStream.h"
#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/Object.h"
#include "KAI/Core/Value.h"
#include "KAI/Network/Future.h"
#include "KAI/Network/FwdDeclarations.h"
#include "KAI/Network/NetHandle.h"
#include "KAI/Network/Transport.h"

KAI_NET_BEGIN

// Forward declarations for KAI network classes
enum class ConnectionEvent;
struct ProxyBase;
struct AgentBase;

namespace detail {
struct MethodInvokerBase;
struct PropertyAccessorBase;
}  // namespace detail

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

    void SetUpdatePump(std::function<void()> pump) {
        updatePump_ = std::move(pump);
    }

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
    Future<R> Invoke(NetHandle handle, const std::string &name, Args &&...args);

    template <typename T>
    T WaitFor(Future<T> &future, std::chrono::milliseconds timeout =
                                     std::chrono::milliseconds(5000));

    void WaitFor(Future<void> &future, std::chrono::milliseconds timeout =
                                           std::chrono::milliseconds(5000));

    // Compatibility helpers for generated agents
    void SendResponse(const NetAddress &peer, BinaryStream &response);
    void BroadcastEvent(const std::string &name, BinaryStream &eventData);
    void BroadcastEvent(const std::string &name);

    // Event subscription: handler receives a BinaryPacket positioned after the
    // event name so callers can deserialize the payload directly.
    void SubscribeEvent(const std::string &name,
                        std::function<void(BinaryPacket &)> handler);

    // Send a serialized KAI object to all connected peers.
    void SendObject(const Object &obj);

    // Subscribe to incoming object messages.
    void SubscribeObjectMessage(std::function<void(const Object &)> handler);

    // Register a callback for connection lifecycle events.
    void SetConnectionEventCallback(
        std::function<void(ConnectionEvent, const NetAddress &)> callback);

    // Associate a proxy handle with the remote peer address that hosts it.
    // Must be called before invoking methods on that proxy.
    void BindProxyAddress(NetHandle handle, const NetAddress &address);

    template <typename P>
    Future<P> FetchProperty(NetHandle handle, const std::string &name);

    template <typename P>
    Future<void> StoreProperty(NetHandle handle, const std::string &name,
                               P &&value);

    // Completes a Future<T> that was previously handed to this Node (as an
    // RPC argument) while still pending, identified by the id it was
    // registered under via RegisterPendingFutureImport. Called once the far
    // side (or, for a same-process call, this same Node) reports that the
    // future has resolved. Safe to call with an unknown id (no-op).
    void CompletePendingFutureImport(int futureId, const Object &value);

    // Registers a completer for a still-pending Future<T> that this Node
    // received as a method argument, so that a later resolution (local or
    // over the network, see ID_KAI_FUTURE_RESOLVE) can complete the exact
    // same shared Future state that was already handed to the method
    // implementation.
    template <typename T>
    void RegisterPendingFutureImport(int futureId, Future<T> future) {
        if (futureId == 0) return;
        std::lock_guard<std::mutex> lock(futureImportMutex_);
        pendingFutureImports_[futureId] = [future](const Object &value) mutable {
            if constexpr (!std::is_void_v<T>) {
                if (value.Exists()) {
                    future.SetValue(ConstDeref<std::decay_t<T>>(value));
                }
            }
            future.SetResponse(ResponseType::Returned);
            future.SetComplete(true);
        };
    }

   private:
    void ProcessPacket(const NetPacket &packet);
    void ProcessObjectMessage(const NetPacket &packet);
    void ProcessFunctionCall(const NetPacket &packet);
    void ProcessEventNotification(const NetPacket &packet);
    void ProcessFunctionResponse(const NetPacket &packet);
    void ProcessPropertyGet(const NetPacket &packet);
    void ProcessPropertySet(const NetPacket &packet);
    void OnConnectionEvent(int connectionId, ConnectionEvent event);
    void SendFunctionCall(NetHandle handle, const std::string &name,
                          const Object &args, int futureId);
    // Implemented in Node.cpp to avoid circular includes via Serialization.h.
    void SendPropertyGet(NetHandle handle, int futureId,
                         const std::string &name);
    void SendPropertySet(NetHandle handle, int futureId,
                         const std::string &name, const Object &value);
    // Sends a resolution notice for a future that was previously passed as
    // a still-pending method argument to `target` (see PackInvokeArg).
    void SendFutureResolution(const NetAddress &target, int futureId,
                              const Object &value);
    void ProcessFutureResolution(const NetPacket &packet);

    // Packs one Invoke() argument into an Object suitable for the args
    // Array. A plain value packs as-is (unchanged behavior). A Future<T>
    // packs as a 2-element Array: [true, value] if already resolved, or
    // [false, id] if still pending - in which case `resolveSink` is
    // attached via Future::OnResolved so that whenever the caller's future
    // eventually completes, the far side (or this same Node, for a local
    // call) gets told via `resolveSink(id, resolvedValueObject)`.
    template <typename T>
    Object PackInvokeArg(T &&arg,
                         std::function<void(int, const Object &)> resolveSink) {
        using D = std::decay_t<T>;
        if constexpr (is_future_v<D>) {
            using U = future_value_t<D>;
            Value<Array> slot = registry_->New<Array>();
            if (arg.IsComplete()) {
                slot->Append(registry_->New(true));
                if constexpr (!std::is_void_v<U>) {
                    slot->Append(registry_->New(arg.GetValue()));
                }
            } else {
                if (arg.GetId() == 0) arg.SetId(nextFutureId_++);
                int id = arg.GetId();
                slot->Append(registry_->New(false));
                slot->Append(registry_->New(id));

                Registry *reg = registry_;
                arg.OnResolved([reg, id, arg, resolveSink]() mutable {
                    (void)reg;  // unused when U is void - value-less resolve
                    Object valueObj;
                    if constexpr (!std::is_void_v<U>) {
                        if (auto opt = arg.GetOptionalValue()) {
                            valueObj = reg->New(*opt);
                        }
                    }
                    if (resolveSink) resolveSink(id, valueObj);
                });
            }
            return slot.GetObject();
        } else {
            return registry_->New(std::forward<T>(arg));
        }
    }

    // Returns the routed NetAddress for a handle: checks proxyAddresses_ first,
    // then falls back to the first active connection.
    NetAddress RouteAddress(NetHandle handle) const;

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
        std::unordered_map<std::string,
                           std::shared_ptr<detail::MethodInvokerBase>>
            methods;
        std::unordered_map<std::string,
                           std::shared_ptr<detail::PropertyAccessorBase>>
            properties;
    };

    std::unordered_map<int, AgentEntry> agentEntries_;
    std::mutex agentMutex_;
    std::atomic<int> nextHandle_{1};
    std::atomic<int> nextFutureId_{1};

    struct PendingResponse {
        std::function<void(const Object &, ResponseType, const std::string &)>
            complete;
    };

    std::unordered_map<int, PendingResponse> pendingResponses_;
    std::mutex pendingMutex_;

    // Completers for still-pending Future<T> arguments this Node has
    // received (locally or over the network), keyed by the future's id.
    // See RegisterPendingFutureImport / CompletePendingFutureImport.
    std::unordered_map<int, std::function<void(const Object &)>>
        pendingFutureImports_;
    std::mutex futureImportMutex_;

    std::unordered_map<std::string,
                       std::vector<std::function<void(BinaryPacket &)>>>
        eventSubscriptions_;
    std::mutex eventMutex_;

    std::vector<std::function<void(const Object &)>> objectMessageHandlers_;
    std::mutex objectMessageMutex_;

    std::function<void(ConnectionEvent, const NetAddress &)>
        connectionEventCallback_;

    // Maps proxy NetHandle value → the remote peer's address.
    std::unordered_map<int, NetAddress> proxyAddresses_;
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
    MethodInvoker(Registry *registry, Node *node, std::function<R(Args...)> fn)
        : registry_(registry), node_(node), fn_(std::move(fn)) {}

    Object Invoke(const std::vector<Object> &args) override {
        if (args.size() != sizeof...(Args)) {
            throw std::invalid_argument("Incorrect number of arguments");
        }
        return InvokeImpl(args, std::index_sequence_for<Args...>{});
    }

   private:
    template <typename T>
    std::decay_t<T> ExtractArg(const Object &obj) {
        using D = std::decay_t<T>;
        if constexpr (std::is_same_v<D, Object>) {
            return obj;
        } else if constexpr (is_future_v<D>) {
            // A Future<U> argument travels as a 2-element Array:
            // [true, value] if the sender had already resolved it, or
            // [false, id] if it was still pending - see
            // Node::PackInvokeArg. Either way we reconstruct a real
            // Future<U> here; a pending one gets registered with the Node
            // so a later resolution (local or networked) can complete it.
            using U = future_value_t<D>;
            D future;
            if (obj.Exists() && obj.IsType<Array>()) {
                const Array &arr = ConstDeref<Array>(obj);
                bool resolved = arr.Size() > 0 && ConstDeref<bool>(arr.At(0));
                if (resolved) {
                    if constexpr (!std::is_void_v<U>) {
                        if (arr.Size() > 1) {
                            future.SetValue(
                                ConstDeref<std::decay_t<U>>(arr.At(1)));
                        }
                    }
                    future.SetResponse(ResponseType::Returned);
                    future.SetComplete(true);
                } else if (arr.Size() > 1) {
                    int id = ConstDeref<int>(arr.At(1));
                    future.SetId(id);
                    if (node_) {
                        node_->RegisterPendingFutureImport<U>(id, future);
                    }
                }
            }
            return future;
        } else {
            return ConstDeref<D>(obj);
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
    Node *node_;
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
    // Object-level accessors used by the remote property get/set path.
    virtual Object GetAsObject(Registry *reg) = 0;
    virtual void SetFromObject(const Object &obj) {
        KAI_UNUSED_1(obj);
        throw std::runtime_error("Property is read-only");
    }
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

    Object GetAsObject(Registry *reg) override {
        if (!reg) throw std::runtime_error("Null registry in GetAsObject");
        return reg->New(getter_());
    }

    void SetFromObject(const Object &obj) override {
        setter_(ConstDeref<std::decay_t<Value>>(obj));
    }

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

    Object GetAsObject(Registry *reg) override {
        if (!reg) throw std::runtime_error("Null registry in GetAsObject");
        return reg->New(getter_());
    }

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
        registry_, this, std::move(fn));
}

template <typename Value>
void Node::RegisterProperty(NetHandle handle, const std::string &name,
                            std::function<Value()> getter,
                            std::function<void(Value)> setter) {
    std::lock_guard<std::mutex> lock(agentMutex_);
    auto accessor = std::make_shared<detail::PropertyAccessor<Value>>(
        std::move(getter), std::move(setter));
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
            // Same-process call: a pending Future<T> argument's resolution
            // is completed directly against this same Node's pending-import
            // table, no network round trip needed.
            std::function<void(int, const Object &)> localResolveSink =
                [this](int futureId, const Object &value) {
                    CompletePendingFutureImport(futureId, value);
                };
            if constexpr (sizeof...(Args) > 0) {
                (packedArgs.emplace_back(
                     PackInvokeArg(std::forward<Args>(args), localResolveSink)),
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
    // Remote call: a pending Future<T> argument's resolution has to cross
    // the network back to whichever peer hosts `handle`.
    std::function<void(int, const Object &)> remoteResolveSink =
        [this, handle](int futureId, const Object &value) {
            SendFutureResolution(RouteAddress(handle), futureId, value);
        };
    if constexpr (sizeof...(Args) > 0) {
        (argsArray->Append(PackInvokeArg(std::forward<Args>(args),
                                        remoteResolveSink)),
         ...);
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
        if (entry != agentEntries_.end()) {
            auto it = entry->second.properties.find(name);
            if (it != entry->second.properties.end()) {
                accessor = it->second;
            }
        }
    }

    if (accessor) {
        // Local path.
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

    // Remote path: serialize a property-get request and register the future.
    if (!registry_) {
        future.SetResponse(ResponseType::BadRequest);
        future.SetComplete(true);
        future.SetErrorMessage("Null registry for remote FetchProperty");
        return future;
    }

    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        auto state = future.GetState();
        pendingResponses_[future.GetId()].complete =
            [state](const Object &obj, ResponseType response,
                    const std::string &error) {
                state->Response = response;
                state->Complete = true;
                state->ErrorMessage = error;
                if (response == ResponseType::Returned) {
                    if constexpr (std::is_same_v<std::decay_t<P>, Object>) {
                        state->Value = obj;
                    } else {
                        state->Value = ConstDeref<std::decay_t<P>>(obj);
                    }
                }
            };
    }

    // Implemented in Node.cpp to avoid circular includes via Serialization.h.
    SendPropertyGet(handle, future.GetId(), name);
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
        if (entry != agentEntries_.end()) {
            auto it = entry->second.properties.find(name);
            if (it != entry->second.properties.end()) {
                accessor = it->second;
            }
        }
    }

    if (accessor) {
        // Local path.
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

    // Remote path: serialize a property-set request.
    if (!registry_) {
        future.SetResponse(ResponseType::BadRequest);
        future.SetComplete(true);
        future.SetErrorMessage("Null registry for remote StoreProperty");
        return future;
    }

    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        auto state = future.GetState();
        pendingResponses_[future.GetId()].complete =
            [state](const Object & /*obj*/, ResponseType response,
                    const std::string &error) {
                state->Response = response;
                state->Complete = true;
                state->ErrorMessage = error;
            };
    }

    Object valueObj = registry_->New(std::forward<P>(value));
    // Implemented in Node.cpp to avoid circular includes via Serialization.h.
    SendPropertySet(handle, future.GetId(), name, valueObj);
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
