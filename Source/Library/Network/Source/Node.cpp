#include "KAI/Network/Node.h"

#include <iostream>

#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Network/ConnectionManager.h"
#include "KAI/Network/NetworkLogger.h"
#include "KAI/Network/PeerDiscovery.h"
#include "KAI/Network/Serialization.h"

KAI_NET_BEGIN

Node::Node() : peer_(nullptr), isRunning_(false), registry_(nullptr) {
    peer_ = NetPeer::Create();
    if (!peer_) {
        std::cerr << "Failed to create network peer" << std::endl;
        return;
    }

    // Create the connection manager
    connectionManager_ = std::make_unique<ConnectionManager>(peer_.get());

    // Create the peer discovery component
    peerDiscovery_ = std::make_unique<PeerDiscovery>(peer_.get());

    // Initialize NetworkLogger
    NetworkLogger::Init();
}

Node::~Node() {
    Shutdown();
    peer_.reset();
}

void Node::Listen(int port) { Listen(IpAddress("0.0.0.0"), port); }

void Node::Listen(IpAddress const &address, int port) {
    if (!peer_) return;

    // Check if we're already running
    if (isRunning_) {
        std::string errorMsg = "Node is already running, cannot listen on " +
                               address.ToString() + ":" + std::to_string(port);
        std::cerr << errorMsg << std::endl;
        NetworkLogger::LogStatus(errorMsg);
        return;
    }

    NetAddress bindAddress(address.ToString(),
                           static_cast<unsigned short>(port));
    if (!peer_->Startup(32, bindAddress)) {
        const bool wildcardBind = address.ToString() == "0.0.0.0";
        const NetAddress loopback("127.0.0.1",
                                  static_cast<unsigned short>(port));
        if (!wildcardBind || !peer_->Startup(32, loopback)) {
            std::string errorMsg = "Failed to start network server on " +
                                   address.ToString() + ":" +
                                   std::to_string(port);
            std::cerr << errorMsg << std::endl;
            NetworkLogger::LogStatus(errorMsg);
            return;
        }
        bindAddress = loopback;
    }

    peer_->SetMaximumIncomingConnections(32);
    isRunning_ = true;

    // Log that we're listening
    std::string logMessage = "Network node listening on " + bindAddress.host +
                             ":" + std::to_string(port);
    std::cout << logMessage << std::endl;
    NetworkLogger::LogStatus(logMessage);
}

void Node::Connect(IpAddress const &ip, int port) {
    if (!peer_) return;

    // Check if we're already connected to this address
    if (IsConnectedTo(ip, port)) {
        std::string logMessage = "Already connected to " + ip.ToString() + ":" +
                                 std::to_string(port);
        std::cout << logMessage << std::endl;
        NetworkLogger::LogConnection(logMessage);
        return;
    }

    // If not started yet, start with any available port
    if (!isRunning_) {
        NetAddress bindAddress("0.0.0.0", 0);
        if (!peer_->Startup(32, bindAddress) &&
            !peer_->Startup(32, NetAddress("127.0.0.1", 0))) {
            std::string errorMsg =
                "Failed to start network client for " + ip.ToString();
            std::cerr << errorMsg << std::endl;
            NetworkLogger::LogStatus(errorMsg);
            return;
        }

        isRunning_ = true;

        // Log the port we're using
        int usedPort = peer_->GetInternalAddress().GetPort();
        std::string startupMsg =
            "Node started and bound to port " + std::to_string(usedPort);
        std::cout << startupMsg << std::endl;
        NetworkLogger::LogStatus(startupMsg);
    }

    // Connect to remote peer
    if (!peer_->Connect(
            NetAddress(ip.ToString(), static_cast<unsigned short>(port)))) {
        std::string errorMsg = "Failed to connect to " + ip.ToString() + ":" +
                               std::to_string(port);
        std::cerr << errorMsg << std::endl;
        NetworkLogger::LogStatus(errorMsg);
        return;
    }

    std::string logMessage =
        "Connecting to " + ip.ToString() + ":" + std::to_string(port);
    std::cout << logMessage << std::endl;
    NetworkLogger::LogConnection(logMessage);
}

void Node::Disconnect() {
    if (!peer_ || !isRunning_) return;

    // Disconnect from all peers gracefully
    peer_->Shutdown(300);  // Give 300ms to send disconnect packets

    // Reset connection manager
    if (connectionManager_) {
        auto connections = connectionManager_->GetAllConnections();
        for (auto id : connections) {
            connectionManager_->RemoveConnection(id);
        }
    }

    isRunning_ = false;

    NetworkLogger::LogStatus("Node disconnected from all peers");
}

void Node::Shutdown() {
    // Stop peer discovery if it's running
    StopDiscovery();

    // Disconnect from all peers
    Disconnect();

    NetworkLogger::LogStatus("Node shutdown complete");
}

bool Node::Update() {
    if (!peer_ || !isRunning_) return false;

    bool processedPackets = false;

    // Process incoming packets
    while (true) {
        auto packet = peer_->Receive();
        if (!packet) {
            break;
        }
        processedPackets = true;
        ProcessPacket(*packet);
    }

    // Update connection manager
    if (connectionManager_) {
        connectionManager_->Update();
    }

    // Update peer discovery
    if (peerDiscovery_ && peerDiscovery_->IsDiscovering()) {
        peerDiscovery_->Update();
    }

    return processedPackets;
}

void Node::StartDiscovery(int discoveryPort) {
    if (peerDiscovery_) {
        peerDiscovery_->Start(discoveryPort);
        NetworkLogger::LogDiscovery("Node started peer discovery on port " +
                                    std::to_string(discoveryPort));
    }
}

void Node::StopDiscovery() {
    if (peerDiscovery_ && peerDiscovery_->IsDiscovering()) {
        peerDiscovery_->Stop();
        NetworkLogger::LogDiscovery("Node stopped peer discovery");
    }
}

bool Node::IsDiscovering() const {
    return peerDiscovery_ ? peerDiscovery_->IsDiscovering() : false;
}

std::vector<NetAddress> Node::GetDiscoveredPeers() const {
    return peerDiscovery_ ? peerDiscovery_->GetDiscoveredPeers()
                          : std::vector<NetAddress>();
}

void Node::SetPeerDiscoveryCallback(
    std::function<void(const NetAddress &)> callback) {
    if (peerDiscovery_) {
        peerDiscovery_->SetDiscoveryCallback(callback);
    }
}

std::vector<NetAddress> Node::GetConnections() const {
    std::vector<NetAddress> result;
    if (connectionManager_) {
        auto connectionIds = connectionManager_->GetAllConnections();
        for (auto id : connectionIds) {
            result.push_back(connectionManager_->GetSystemAddress(id));
        }
    }
    return result;
}

bool Node::IsConnectedTo(const IpAddress &address, int port) const {
    if (!connectionManager_) return false;

    NetAddress targetAddr(address.ToString(),
                          static_cast<unsigned short>(port));

    // Get all connections and check each one
    for (auto conn : GetConnections()) {
        // Compare IPs and ports separately to handle different string
        // representations
        if (conn.GetPort() == targetAddr.GetPort() &&
            conn.host == targetAddr.host) {
            return true;
        }
    }
    return false;
}

size_t Node::GetConnectionCount() const {
    return connectionManager_ ? connectionManager_->GetConnectionCount() : 0;
}

int Node::GetPing(const IpAddress &address, int port) const {
    if (!peer_ || !connectionManager_) return -1;

    NetAddress systemAddr(address.ToString(),
                          static_cast<unsigned short>(port));
    return peer_->GetAveragePing(systemAddr);
}

unsigned char Node::GetPacketIdentifier(const NetPacket &packet) {
    if (packet.data.empty()) return 255;
    return packet.data[0];
}

void Node::ProcessPacket(const NetPacket &packet) {
    // Get the packet identifier
    unsigned char packetId = GetPacketIdentifier(packet);

    // Log the packet
    std::string packetType = "unknown";
    switch (packetId) {
        case static_cast<unsigned char>(
            SystemMessage::ConnectionRequestAccepted):
            packetType = "connection request accepted";
            break;
        case static_cast<unsigned char>(SystemMessage::ConnectionAttemptFailed):
            packetType = "connection attempt failed";
            break;
        case static_cast<unsigned char>(SystemMessage::AlreadyConnected):
            packetType = "already connected";
            break;
        case static_cast<unsigned char>(SystemMessage::NewIncomingConnection):
            packetType = "new incoming connection";
            break;
        case static_cast<unsigned char>(
            SystemMessage::NoFreeIncomingConnections):
            packetType = "no free incoming connections";
            break;
        case static_cast<unsigned char>(
            SystemMessage::DisconnectionNotification):
            packetType = "disconnection notification";
            break;
        case static_cast<unsigned char>(SystemMessage::ConnectionLost):
            packetType = "connection lost";
            break;
        default:
            packetType = "custom type: " + std::to_string(packetId);
            break;
    }

    std::string logMessage = "Received packet of type '" + packetType +
                             "' from " + packet.address.ToString();
    NetworkLogger::LogMessage(logMessage);

    // Update connection activity
    if (connectionManager_) {
        connectionManager_->UpdateActivity(packet.address);
    }

    // Handle standard connection events
    switch (packetId) {
        case static_cast<unsigned char>(
            SystemMessage::ConnectionRequestAccepted): {
            // We connected to another system
            OnConnectionEvent(connectionManager_->AddConnection(packet.address),
                              ConnectionEvent::Connected);
            break;
        }
        case static_cast<unsigned char>(
            SystemMessage::ConnectionAttemptFailed): {
            // Connection attempt failed
            OnConnectionEvent(0, ConnectionEvent::ConnectionFailed);
            break;
        }
        case static_cast<unsigned char>(SystemMessage::AlreadyConnected): {
            // We're already connected to this system
            // Re-use Connected event since AlreadyConnected is not defined
            OnConnectionEvent(
                connectionManager_->GetConnectionId(packet.address),
                ConnectionEvent::Connected);
            break;
        }
        case static_cast<unsigned char>(SystemMessage::NewIncomingConnection): {
            // A remote system connected to us
            OnConnectionEvent(connectionManager_->AddConnection(packet.address),
                              ConnectionEvent::Connected);
            break;
        }
        case static_cast<unsigned char>(
            SystemMessage::NoFreeIncomingConnections): {
            // Remote system has no free incoming connections
            OnConnectionEvent(0, ConnectionEvent::ConnectionFailed);
            break;
        }
        case static_cast<unsigned char>(
            SystemMessage::DisconnectionNotification): {
            // Remote system disconnected
            OnConnectionEvent(
                connectionManager_->GetConnectionId(packet.address),
                ConnectionEvent::Disconnected);
            connectionManager_->RemoveConnection(packet.address);
            break;
        }
        case static_cast<unsigned char>(SystemMessage::ConnectionLost): {
            // Connection lost
            OnConnectionEvent(
                connectionManager_->GetConnectionId(packet.address),
                ConnectionEvent::ConnectionLost);
            connectionManager_->RemoveConnection(packet.address);
            break;
        }
        default: {
            // Check if this is a custom message type
            if (packetId >= kUserPacketStart) {
                if (packetId == NetworkSerializer::ID_KAI_OBJECT_MESSAGE) {
                    // Process object message (serialized object)
                    ProcessObjectMessage(packet);
                } else if (packetId ==
                           NetworkSerializer::ID_KAI_FUNCTION_CALL) {
                    // Process function call
                    ProcessFunctionCall(packet);
                } else if (packetId ==
                           NetworkSerializer::ID_KAI_EVENT_NOTIFICATION) {
                    // Process event notification
                    ProcessEventNotification(packet);
                } else if (packetId ==
                           NetworkSerializer::ID_KAI_FUNCTION_RESPONSE) {
                    ProcessFunctionResponse(packet);
                } else if (packetId == NetworkSerializer::ID_KAI_PROPERTY_GET) {
                    ProcessPropertyGet(packet);
                } else if (packetId == NetworkSerializer::ID_KAI_PROPERTY_SET) {
                    ProcessPropertySet(packet);
                }
            }
            break;
        }
    }
}

void Node::ProcessObjectMessage(const NetPacket &packet) {
    if (!registry_) {
        NetworkLogger::LogMessage("ProcessObjectMessage: null registry");
        return;
    }

    BinaryPacket stream(
        reinterpret_cast<const char *>(packet.data.data()),
        reinterpret_cast<const char *>(packet.data.data() + packet.data.size()),
        registry_);

    unsigned char msgId = 0;
    if (!stream.Read(msgId)) {
        NetworkLogger::LogMessage(
            "ProcessObjectMessage: failed to read header");
        return;
    }

    Object obj = NetworkSerializer::DeserializeObject(stream, *registry_);
    if (!obj.Exists()) {
        NetworkLogger::LogMessage(
            "ProcessObjectMessage: failed to deserialize object");
        return;
    }

    std::vector<std::function<void(const Object &)>> handlers;
    {
        std::lock_guard<std::mutex> lock(objectMessageMutex_);
        handlers = objectMessageHandlers_;
    }

    for (auto &handler : handlers) {
        handler(obj);
    }
}

void Node::ProcessFunctionCall(const NetPacket &packet) {
    if (!registry_) {
        NetworkLogger::LogMessage(
            "Processing function call failed: null packet or registry");
        return;
    }

    BinaryPacket stream(
        reinterpret_cast<const char *>(packet.data.data()),
        reinterpret_cast<const char *>(packet.data.data() + packet.data.size()),
        registry_);

    unsigned char msgId = 0;
    if (!stream.Read(msgId)) {
        NetworkLogger::LogMessage("Failed to read function call header");
        return;
    }

    int handleValue = 0;
    int futureId = 0;
    std::string methodName;

    if (!stream.Read(handleValue) || !stream.Read(futureId) ||
        !NetworkSerializer::ReadString(stream, methodName)) {
        NetworkLogger::LogMessage("Failed to read function call metadata");
        return;
    }

    Object argsObj = NetworkSerializer::DeserializeObject(stream, *registry_);
    std::vector<Object> args;
    if (argsObj.Exists() && argsObj.IsType<Array>()) {
        const Array &arr = ConstDeref<Array>(argsObj);
        args.reserve(arr.Size());
        for (int i = 0; i < arr.Size(); ++i) {
            args.push_back(arr.At(i));
        }
    }

    std::shared_ptr<detail::MethodInvokerBase> invoker;
    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        auto entry = agentEntries_.find(handleValue);
        if (entry != agentEntries_.end()) {
            auto it = entry->second.methods.find(methodName);
            if (it != entry->second.methods.end()) {
                invoker = it->second;
            }
        }
    }

    ResponseType response = ResponseType::Returned;
    std::string errorMessage;
    Object result;

    if (!invoker) {
        response = ResponseType::BadRequest;
        errorMessage = "Unknown agent or method";
    } else {
        try {
            result = invoker->Invoke(args);
        } catch (const std::exception &e) {
            response = ResponseType::BadRequest;
            errorMessage = e.what();
        }
    }

    if (futureId != 0) {
        BinaryStream responseStream;
        responseStream.Write(static_cast<unsigned char>(
            NetworkSerializer::ID_KAI_FUNCTION_RESPONSE));
        responseStream.Write(futureId);
        responseStream.Write(static_cast<int>(response));
        NetworkSerializer::WriteString(responseStream, errorMessage);
        NetworkSerializer::SerializeObject(responseStream, result);

        peer_->Send(responseStream, SendReliability::Reliable, BufferOffset(0),
                    packet.address, SendRouting::Unicast);
        peer_->Flush();
    }
}

void Node::ProcessEventNotification(const NetPacket &packet) {
    BinaryPacket stream(
        reinterpret_cast<const char *>(packet.data.data()),
        reinterpret_cast<const char *>(packet.data.data() + packet.data.size()),
        registry_);

    unsigned char msgId = 0;
    std::string eventName;
    if (!stream.Read(msgId) ||
        !NetworkSerializer::ReadString(stream, eventName)) {
        NetworkLogger::LogMessage(
            "ProcessEventNotification: failed to read header");
        return;
    }

    std::vector<std::function<void(BinaryPacket &)>> handlers;
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        auto it = eventSubscriptions_.find(eventName);
        if (it != eventSubscriptions_.end()) {
            handlers = it->second;
        }
    }

    NetworkLogger::LogMessage(
        "ProcessEventNotification: dispatching event '" + eventName + "' to " +
        std::to_string(handlers.size()) + " subscriber(s)");

    for (auto &handler : handlers) {
        handler(stream);
    }
}

void Node::SendResponse(const NetAddress &peer, BinaryStream &response) {
    if (!peer_ || !isRunning_) return;

    BinaryStream bs;
    bs.Write(static_cast<unsigned char>(
        NetworkSerializer::ID_KAI_FUNCTION_RESPONSE));
    bs.Write(0);  // Unknown future id; compatibility-only
    bs.Write(static_cast<int>(ResponseType::Returned));
    NetworkSerializer::WriteString(bs, std::string());

    // Pass through raw response payload to keep generated code compiling.
    int size = response.Size();
    bs.Write(size);
    if (size > 0) {
        bs.Write(size, response.Begin());
    }

    peer_->Send(bs, SendReliability::Reliable, BufferOffset(0), peer,
                SendRouting::Unicast);
}

void Node::BroadcastEvent(const std::string &name, BinaryStream &eventData) {
    if (!peer_ || !isRunning_) return;

    BinaryStream bs;
    bs.Write(static_cast<unsigned char>(
        NetworkSerializer::ID_KAI_EVENT_NOTIFICATION));
    NetworkSerializer::WriteString(bs, name);

    // Always write raw payload bytes (zero bytes for empty events).
    if (eventData.Size() > 0) {
        bs.Write(eventData.Size(), eventData.Begin());
    }

    peer_->Send(bs, SendReliability::Reliable, BufferOffset(0), NetAddress(),
                SendRouting::Broadcast);
    peer_->Flush();
}

void Node::BroadcastEvent(const std::string &name) {
    BinaryStream empty;
    BroadcastEvent(name, empty);
}

void Node::ProcessFunctionResponse(const NetPacket &packet) {
    if (!registry_) {
        NetworkLogger::LogMessage(
            "Processing function response failed: null packet or registry");
        return;
    }

    BinaryPacket stream(
        reinterpret_cast<const char *>(packet.data.data()),
        reinterpret_cast<const char *>(packet.data.data() + packet.data.size()),
        registry_);

    unsigned char msgId = 0;
    int futureId = 0;
    int responseValue = 0;
    std::string errorMessage;

    if (!stream.Read(msgId) || !stream.Read(futureId) ||
        !stream.Read(responseValue) ||
        !NetworkSerializer::ReadString(stream, errorMessage)) {
        NetworkLogger::LogMessage("Failed to read function response");
        return;
    }

    Object result = NetworkSerializer::DeserializeObject(stream, *registry_);

    PendingResponse pending;
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        auto it = pendingResponses_.find(futureId);
        if (it == pendingResponses_.end()) {
            return;
        }
        pending = std::move(it->second);
        pendingResponses_.erase(it);
    }

    if (pending.complete) {
        pending.complete(result, static_cast<ResponseType>(responseValue),
                         errorMessage);
    }
}

void Node::SendFunctionCall(NetHandle handle, const std::string &name,
                            const Object &args, int futureId) {
    if (!peer_ || !isRunning_) return;

    NetAddress targetAddress;
    bool found = false;

    // Check if this proxy handle has a bound remote address.
    {
        auto it = proxyAddresses_.find(handle.value);
        if (it != proxyAddresses_.end()) {
            targetAddress = it->second;
            found = true;
        }
    }

    // Fall back to the first available connection (single-peer case).
    if (!found && connectionManager_) {
        auto connections = connectionManager_->GetAllConnections();
        if (!connections.empty()) {
            targetAddress =
                connectionManager_->GetSystemAddress(connections.front());
            found = true;
        }
    }

    if (!found) {
        NetworkLogger::LogMessage(
            "No active connection for SendFunctionCall to handle " +
            std::to_string(handle.value));
        return;
    }

    BinaryStream bs;
    bs.Write(
        static_cast<unsigned char>(NetworkSerializer::ID_KAI_FUNCTION_CALL));
    bs.Write(handle.value);
    bs.Write(futureId);
    NetworkSerializer::WriteString(bs, name);
    NetworkSerializer::SerializeObject(bs, args);

    peer_->Send(bs, SendReliability::Reliable, BufferOffset(0), targetAddress,
                SendRouting::Unicast);
    peer_->Flush();
}

void Node::OnConnectionEvent(int connectionId, ConnectionEvent event) {
    std::string eventType;
    switch (event) {
        case ConnectionEvent::Connected:
            eventType = "Connected";
            break;
        case ConnectionEvent::Disconnected:
            eventType = "Disconnected";
            break;
        case ConnectionEvent::ConnectionFailed:
            eventType = "ConnectionFailed";
            break;
        case ConnectionEvent::ConnectionLost:
            eventType = "ConnectionLost";
            break;
        case ConnectionEvent::Timeout:
            eventType = "Timeout";
            break;
        default:
            eventType = "Unknown";
            break;
    }

    NetworkLogger::LogConnection("Connection event: " + eventType +
                                 " for connection ID " +
                                 std::to_string(connectionId));

    if (connectionEventCallback_ && connectionManager_) {
        NetAddress address;
        auto connections = connectionManager_->GetAllConnections();
        for (auto id : connections) {
            if (id == connectionId) {
                address = connectionManager_->GetSystemAddress(id);
                break;
            }
        }
        connectionEventCallback_(event, address);
    }
}

void Node::SubscribeEvent(const std::string &name,
                          std::function<void(BinaryPacket &)> handler) {
    std::lock_guard<std::mutex> lock(eventMutex_);
    eventSubscriptions_[name].push_back(std::move(handler));
}

void Node::SendObject(const Object &obj) {
    if (!peer_ || !isRunning_) return;

    BinaryStream bs;
    bs.Write(
        static_cast<unsigned char>(NetworkSerializer::ID_KAI_OBJECT_MESSAGE));
    NetworkSerializer::SerializeObject(bs, obj);

    peer_->Send(bs, SendReliability::Reliable, BufferOffset(0), NetAddress(),
                SendRouting::Broadcast);
    peer_->Flush();
}

void Node::SubscribeObjectMessage(std::function<void(const Object &)> handler) {
    std::lock_guard<std::mutex> lock(objectMessageMutex_);
    objectMessageHandlers_.push_back(std::move(handler));
}

void Node::SetConnectionEventCallback(
    std::function<void(ConnectionEvent, const NetAddress &)> callback) {
    connectionEventCallback_ = std::move(callback);
}

void Node::BindProxyAddress(NetHandle handle, const NetAddress &address) {
    proxyAddresses_[handle.value] = address;
}

void Node::SendPropertyGet(NetHandle handle, int futureId,
                           const std::string &name) {
    if (!peer_ || !isRunning_) return;

    BinaryStream bs;
    bs.Write(
        static_cast<unsigned char>(NetworkSerializer::ID_KAI_PROPERTY_GET));
    bs.Write(handle.value);
    bs.Write(futureId);
    NetworkSerializer::WriteString(bs, name);

    NetAddress target = RouteAddress(handle);
    if (target.IsValid()) {
        peer_->Send(bs, SendReliability::Reliable, BufferOffset(0), target,
                    SendRouting::Unicast);
        peer_->Flush();
    }
}

void Node::SendPropertySet(NetHandle handle, int futureId,
                           const std::string &name, const Object &value) {
    if (!peer_ || !isRunning_) return;

    BinaryStream bs;
    bs.Write(
        static_cast<unsigned char>(NetworkSerializer::ID_KAI_PROPERTY_SET));
    bs.Write(handle.value);
    bs.Write(futureId);
    NetworkSerializer::WriteString(bs, name);
    NetworkSerializer::SerializeObject(bs, value);

    NetAddress target = RouteAddress(handle);
    if (target.IsValid()) {
        peer_->Send(bs, SendReliability::Reliable, BufferOffset(0), target,
                    SendRouting::Unicast);
        peer_->Flush();
    }
}

NetAddress Node::RouteAddress(NetHandle handle) const {
    auto it = proxyAddresses_.find(handle.value);
    if (it != proxyAddresses_.end()) {
        return it->second;
    }
    if (connectionManager_) {
        auto connections = connectionManager_->GetAllConnections();
        if (!connections.empty()) {
            return connectionManager_->GetSystemAddress(connections.front());
        }
    }
    return NetAddress();
}

void Node::ProcessPropertyGet(const NetPacket &packet) {
    if (!registry_) return;

    BinaryPacket stream(
        reinterpret_cast<const char *>(packet.data.data()),
        reinterpret_cast<const char *>(packet.data.data() + packet.data.size()),
        registry_);

    unsigned char msgId = 0;
    int handleValue = 0;
    int futureId = 0;
    std::string propertyName;
    if (!stream.Read(msgId) || !stream.Read(handleValue) ||
        !stream.Read(futureId) ||
        !NetworkSerializer::ReadString(stream, propertyName)) {
        NetworkLogger::LogMessage("ProcessPropertyGet: failed to read header");
        return;
    }

    std::shared_ptr<detail::PropertyAccessorBase> accessor;
    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        auto entry = agentEntries_.find(handleValue);
        if (entry != agentEntries_.end()) {
            auto it = entry->second.properties.find(propertyName);
            if (it != entry->second.properties.end()) {
                accessor = it->second;
            }
        }
    }

    ResponseType response = ResponseType::Returned;
    std::string errorMessage;
    Object result;
    if (!accessor) {
        response = ResponseType::BadRequest;
        errorMessage = "Unknown property";
    } else {
        try {
            result = accessor->GetAsObject(registry_);
        } catch (const std::exception &e) {
            response = ResponseType::BadRequest;
            errorMessage = e.what();
        }
    }

    BinaryStream responseStream;
    responseStream.Write(static_cast<unsigned char>(
        NetworkSerializer::ID_KAI_FUNCTION_RESPONSE));
    responseStream.Write(futureId);
    responseStream.Write(static_cast<int>(response));
    NetworkSerializer::WriteString(responseStream, errorMessage);
    NetworkSerializer::SerializeObject(responseStream, result);

    peer_->Send(responseStream, SendReliability::Reliable, BufferOffset(0),
                packet.address, SendRouting::Unicast);
    peer_->Flush();
}

void Node::ProcessPropertySet(const NetPacket &packet) {
    if (!registry_) return;

    BinaryPacket stream(
        reinterpret_cast<const char *>(packet.data.data()),
        reinterpret_cast<const char *>(packet.data.data() + packet.data.size()),
        registry_);

    unsigned char msgId = 0;
    int handleValue = 0;
    int futureId = 0;
    std::string propertyName;
    if (!stream.Read(msgId) || !stream.Read(handleValue) ||
        !stream.Read(futureId) ||
        !NetworkSerializer::ReadString(stream, propertyName)) {
        NetworkLogger::LogMessage("ProcessPropertySet: failed to read header");
        return;
    }

    Object valueObj = NetworkSerializer::DeserializeObject(stream, *registry_);

    std::shared_ptr<detail::PropertyAccessorBase> accessor;
    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        auto entry = agentEntries_.find(handleValue);
        if (entry != agentEntries_.end()) {
            auto it = entry->second.properties.find(propertyName);
            if (it != entry->second.properties.end()) {
                accessor = it->second;
            }
        }
    }

    ResponseType response = ResponseType::Returned;
    std::string errorMessage;
    if (!accessor) {
        response = ResponseType::BadRequest;
        errorMessage = "Unknown property";
    } else {
        try {
            accessor->SetFromObject(valueObj);
        } catch (const std::exception &e) {
            response = ResponseType::BadRequest;
            errorMessage = e.what();
        }
    }

    Object empty;
    BinaryStream responseStream;
    responseStream.Write(static_cast<unsigned char>(
        NetworkSerializer::ID_KAI_FUNCTION_RESPONSE));
    responseStream.Write(futureId);
    responseStream.Write(static_cast<int>(response));
    NetworkSerializer::WriteString(responseStream, errorMessage);
    NetworkSerializer::SerializeObject(responseStream, empty);

    peer_->Send(responseStream, SendReliability::Reliable, BufferOffset(0),
                packet.address, SendRouting::Unicast);
    peer_->Flush();
}

KAI_NET_END
