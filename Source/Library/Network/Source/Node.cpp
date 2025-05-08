#include "KAI/Network/Node.h"

#include <iostream>

#include "KAI/Network/ConnectionManager.h"
#include "KAI/Network/PeerDiscovery.h"
#include "KAI/Network/RakNetStub.h"
#include "KAI/Network/Serialization.h"

KAI_NET_BEGIN

Node::Node() : _peer(nullptr), _isRunning(false) {
    // Use an empty registry for now
    // _reg = std::make_shared<Registry>();

    // Initialize RakNet
    _peer = RakNet::RakPeerInterface::GetInstance();
    if (_peer == nullptr) {
        std::cerr << "Failed to create RakNet peer interface" << std::endl;
        return;
    }

    // Create the connection manager
    _connectionManager = std::make_unique<ConnectionManager>(_peer);

    // Create the peer discovery component
    _peerDiscovery = std::make_unique<PeerDiscovery>(_peer);
}

Node::~Node() {
    Shutdown();

    // Clean up RakNet
    if (_peer) {
        RakNet::RakPeerInterface::DestroyInstance(_peer);
        _peer = nullptr;
    }
}

void Node::Listen(int port) {
    if (!_peer) return;

    // Max 32 connections, use all available network interfaces
    RakNet::SocketDescriptor sd(port, nullptr);
    RakNet::StartupResult result = _peer->Startup(32, &sd, 1);

    if (result != RakNet::RAKNET_STARTED) {
        std::cerr << "Failed to start RakNet server, error code: " << result
                  << std::endl;
        return;
    }

    _peer->SetMaximumIncomingConnections(32);
    _isRunning = true;

    std::cout << "Network node listening on port " << port << std::endl;
}

void Node::Listen(IpAddress const &address, int port) {
    if (!_peer) return;

    // Use specific interface for binding
    RakNet::SocketDescriptor sd(port, address.ToString().c_str());
    RakNet::StartupResult result = _peer->Startup(32, &sd, 1);

    if (result != RakNet::RAKNET_STARTED) {
        std::cerr
            << "Failed to start RakNet server on specific address, error code: "
            << result << std::endl;
        return;
    }

    _peer->SetMaximumIncomingConnections(32);
    _isRunning = true;

    std::cout << "Network node listening on " << address.ToString() << ":"
              << port << std::endl;
}

void Node::Connect(IpAddress const &ip, int port) {
    if (!_peer) return;

    // If not started yet, start with any available port
    if (!_isRunning) {
        RakNet::SocketDescriptor sd(0, nullptr);  // Use any available port
        RakNet::StartupResult result = _peer->Startup(32, &sd, 1);

        if (result != RakNet::RAKNET_STARTED) {
            std::cerr << "Failed to start RakNet client, error code: " << result
                      << std::endl;
            return;
        }

        _isRunning = true;
    }

    // Connect to remote peer
    RakNet::ConnectionAttemptResult result =
        _peer->Connect(ip.ToString().c_str(), port, nullptr, 0);

    if (result != RakNet::CONNECTION_ATTEMPT_STARTED) {
        std::cerr << "Failed to connect to " << ip.ToString() << ":" << port
                  << ", error code: " << result << std::endl;
        return;
    }

    std::cout << "Connecting to " << ip.ToString() << ":" << port << "..."
              << std::endl;
}

void Node::Disconnect() {
    if (!_peer || !_isRunning) return;

    // Disconnect from all peers gracefully
    _peer->Shutdown(300);  // Give 300ms to send disconnect packets

    // Reset connection manager
    if (_connectionManager) {
        auto connections = _connectionManager->GetAllConnections();
        for (const auto &pair : connections) {
            _connectionManager->RemoveConnection(pair.first);
        }
    }
}

void Node::Shutdown() {
    if (!_isRunning) return;

    // Stop peer discovery if active
    if (_peerDiscovery && _peerDiscovery->IsActive()) {
        _peerDiscovery->Stop();
    }

    // Disconnect from all peers
    Disconnect();

    _isRunning = false;
}

bool Node::Update() {
    if (!_peer || !_isRunning) return false;

    bool processedPackets = false;

    // Process pending packets
    RakNet::Packet *packet = nullptr;
    while ((packet = _peer->Receive()) != nullptr) {
        ProcessPacket(packet);
        _peer->DeallocatePacket(packet);
        processedPackets = true;
    }

    // Update connection manager
    if (_connectionManager) {
        _connectionManager->Update();
    }

    // Update peer discovery
    if (_peerDiscovery && _peerDiscovery->IsActive()) {
        _peerDiscovery->Update();
    }

    return processedPackets;
}

void Node::StartDiscovery(int discoveryPort) {
    if (!_peer || !_isRunning) return;

    if (!_peerDiscovery) return;

    // Start the peer discovery service
    _peerDiscovery->Start(discoveryPort);
}

void Node::StopDiscovery() {
    if (!_peerDiscovery) return;

    _peerDiscovery->Stop();
}

bool Node::IsDiscovering() const {
    if (!_peerDiscovery) return false;

    return _peerDiscovery->IsActive();
}

std::vector<RakNet::SystemAddress> Node::GetDiscoveredPeers() const {
    if (!_peerDiscovery) return std::vector<RakNet::SystemAddress>();

    return _peerDiscovery->GetDiscoveredPeers();
}

void Node::SetPeerDiscoveryCallback(
    std::function<void(const RakNet::SystemAddress &)> callback) {
    if (!_peerDiscovery) return;

    _peerDiscovery->SetDiscoveryCallback(callback);
}

std::vector<RakNet::SystemAddress> Node::GetConnections() const {
    if (!_peer || !_connectionManager)
        return std::vector<RakNet::SystemAddress>();

    std::vector<RakNet::SystemAddress> result;
    const auto &connections = _connectionManager->GetAllConnections();

    for (const auto &pair : connections) {
        result.push_back(pair.second.address);
    }

    return result;
}

bool Node::IsConnectedTo(const IpAddress &address, int port) const {
    if (!_peer || !_connectionManager) return false;

    // Create RakNet system address
    RakNet::SystemAddress rakAddress;
    rakAddress.FromString(address.ToString().c_str(), port);

    return _connectionManager->IsConnected(rakAddress);
}

size_t Node::GetConnectionCount() const {
    if (!_connectionManager) return 0;

    return _connectionManager->GetConnectionCount();
}

int Node::GetPing(const IpAddress &address, int port) const {
    if (!_peer) return 0;

    // Create RakNet system address
    RakNet::SystemAddress rakAddress;
    rakAddress.FromString(address.ToString().c_str(), port);

    return _peer->GetAveragePing(rakAddress);
}

void Node::ProcessPacket(RakNet::Packet *packet) {
    if (!packet || !_peer || !_connectionManager) return;

    unsigned char packetType = GetPacketIdentifier(packet);

    switch (packetType) {
        case RakNet::ID_CONNECTION_REQUEST_ACCEPTED:
            // We connected to a peer
            _connectionManager->AddConnection(packet->systemAddress);
            _connectionManager->OnConnectionEvent(packet->systemAddress,
                                                  ConnectionEvent::Connected);
            break;

        case RakNet::ID_CONNECTION_ATTEMPT_FAILED:
            // Failed to connect to peer
            _connectionManager->OnConnectionEvent(
                packet->systemAddress, ConnectionEvent::ConnectionFailed);
            break;

        case RakNet::ID_NEW_INCOMING_CONNECTION:
            // A peer connected to us
            _connectionManager->AddConnection(packet->systemAddress);
            _connectionManager->OnConnectionEvent(packet->systemAddress,
                                                  ConnectionEvent::Connected);
            break;

        case RakNet::ID_DISCONNECTION_NOTIFICATION:
            // A peer disconnected gracefully
            _connectionManager->OnConnectionEvent(
                packet->systemAddress, ConnectionEvent::Disconnected);
            _connectionManager->RemoveConnection(packet->systemAddress);
            break;

        case RakNet::ID_CONNECTION_LOST:
            // A peer disconnected ungracefully
            _connectionManager->OnConnectionEvent(
                packet->systemAddress, ConnectionEvent::ConnectionLost);
            _connectionManager->RemoveConnection(packet->systemAddress);
            break;

        case RakNet::ID_USER_PACKET_ENUM +
            1:  // NetworkSerializer::ID_KAI_OBJECT_MESSAGE:
            // Handle KAI object message
            ProcessObjectMessage(packet);
            break;

        case RakNet::ID_USER_PACKET_ENUM +
            2:  // NetworkSerializer::ID_KAI_FUNCTION_CALL:
            // Handle KAI function call
            ProcessFunctionCall(packet);
            break;

        case RakNet::ID_USER_PACKET_ENUM +
            3:  // NetworkSerializer::ID_KAI_EVENT_NOTIFICATION:
            // Handle KAI event notification
            ProcessEventNotification(packet);
            break;

        default:
            // Handle other message types
            // Custom user messages should be handled here
            break;
    }
}

void Node::ProcessObjectMessage(RakNet::Packet *packet) {
    // Create a BitStream from the packet
    RakNet::BitStream bs(packet->data, packet->length, false);

    // Skip the message ID
    bs.IgnoreBytes(sizeof(RakNet::MessageID));

    // Read the target handle
    unsigned int handleValue = 0;
    bs.Read(handleValue);

    // Deserialize the object
    Object obj;

    // In a real implementation, we would do:
    // Object obj = NetworkSerializer::DeserializeObject(bs, *_reg);

    // If this is a broadcast (handle == 0), notify all local objects
    if (handleValue == 0) {
        // TODO: Notify all local objects
    } else {
        // Find the specific object to notify
        NetHandle handle(handleValue);
        // TODO: Find and notify the specific object
    }
}

void Node::ProcessFunctionCall(RakNet::Packet *packet) {
    // Create a BitStream from the packet
    RakNet::BitStream bs(packet->data, packet->length, false);

    // Skip the message ID
    bs.IgnoreBytes(sizeof(RakNet::MessageID));

    // TODO: Implement function call processing
}

void Node::ProcessEventNotification(RakNet::Packet *packet) {
    // Create a BitStream from the packet
    RakNet::BitStream bs(packet->data, packet->length, false);

    // Skip the message ID
    bs.IgnoreBytes(sizeof(RakNet::MessageID));

    // TODO: Implement event notification processing
}

void Node::OnConnectionEvent(int connectionId, ConnectionEvent event) {
    // Handle connection events
    switch (event) {
        case ConnectionEvent::Connected:
            std::cout << "Connection established with peer " << connectionId
                      << std::endl;
            break;

        case ConnectionEvent::Disconnected:
            std::cout << "Peer " << connectionId << " disconnected gracefully"
                      << std::endl;
            break;

        case ConnectionEvent::ConnectionLost:
            std::cout << "Connection lost with peer " << connectionId
                      << std::endl;
            break;

        case ConnectionEvent::ConnectionFailed:
            std::cout << "Failed to connect to peer " << connectionId
                      << std::endl;
            break;

        default:
            break;
    }
}

// Helper function to get the packet identifier
unsigned char Node::GetPacketIdentifier(RakNet::Packet *packet) {
    if (!packet || packet->length == 0) return 255;

    if ((unsigned char)packet->data[0] == RakNet::ID_TIMESTAMP) {
        // Timestamp included - skip it to get the real packet type
        if (packet->length > sizeof(RakNet::MessageID) + sizeof(RakNet::TimeMS))
            return (unsigned char)packet
                ->data[sizeof(RakNet::MessageID) + sizeof(RakNet::TimeMS)];
        else
            return 255;
    } else {
        return (unsigned char)packet->data[0];
    }
}

KAI_NET_END