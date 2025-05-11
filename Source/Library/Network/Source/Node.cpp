#include "KAI/Network/Node.h"

#include <iostream>

#include "KAI/Network/ConnectionManager.h"
#include "KAI/Network/NetworkLogger.h"
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

    // Initialize NetworkLogger
    NetworkLogger::Init();
}

Node::~Node() {
    Shutdown();

    // Clean up RakNet
    if (_peer) {
        RakNet::RakPeerInterface::DestroyInstance(_peer);
        _peer = nullptr;
    }
}

void Node::Listen(int port) { Listen(IpAddress("0.0.0.0"), port); }

void Node::Listen(IpAddress const &address, int port) {
    if (!_peer) return;

    // Max 32 connections, use all available network interfaces
    RakNet::SocketDescriptor sd(port, address.ToString().c_str());
    RakNet::StartupResult result = _peer->Startup(32, &sd, 1);

    if (result != RakNet::RAKNET_STARTED) {
        std::string errorMsg = "Failed to start RakNet server, error code: " +
                               std::to_string(result);
        std::cerr << errorMsg << std::endl;
        NetworkLogger::LogStatus(errorMsg);
        return;
    }

    _peer->SetMaximumIncomingConnections(32);
    _isRunning = true;

    // Log that we're listening
    std::string logMessage = "Network node listening on " + address.ToString() +
                             ":" + std::to_string(port);
    std::cout << logMessage << std::endl;
    NetworkLogger::LogStatus(logMessage);
}

void Node::Connect(IpAddress const &ip, int port) {
    if (!_peer) return;

    // If not started yet, start with any available port
    if (!_isRunning) {
        RakNet::SocketDescriptor sd(0, nullptr);  // Use any available port
        RakNet::StartupResult result = _peer->Startup(32, &sd, 1);

        if (result != RakNet::RAKNET_STARTED) {
            std::string errorMsg =
                "Failed to start RakNet client, error code: " +
                std::to_string(result);
            std::cerr << errorMsg << std::endl;
            NetworkLogger::LogStatus(errorMsg);
            return;
        }

        _isRunning = true;
    }

    // Connect to remote peer
    RakNet::ConnectionAttemptResult result =
        _peer->Connect(ip.ToString().c_str(), port, nullptr, 0);

    if (result != RakNet::CONNECTION_ATTEMPT_STARTED) {
        std::string errorMsg = "Failed to connect to " + ip.ToString() + ":" +
                               std::to_string(port) +
                               ", error code: " + std::to_string(result);
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
    if (!_peer || !_isRunning) return;

    // Disconnect from all peers gracefully
    _peer->Shutdown(300);  // Give 300ms to send disconnect packets

    // Reset connection manager
    if (_connectionManager) {
        auto connections = _connectionManager->GetAllConnections();
        for (auto id : connections) {
            _connectionManager->RemoveConnection(id);
        }
    }

    _isRunning = false;

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
    if (!_peer || !_isRunning) return false;

    bool processedPackets = false;

    // Process incoming packets
    RakNet::Packet *packet = nullptr;
    while ((packet = _peer->Receive()) != nullptr) {
        processedPackets = true;
        ProcessPacket(packet);
        _peer->DeallocatePacket(packet);
    }

    // Update connection manager
    if (_connectionManager) {
        _connectionManager->Update();
    }

    // Update peer discovery
    if (_peerDiscovery && _peerDiscovery->IsDiscovering()) {
        _peerDiscovery->Update();
    }

    return processedPackets;
}

void Node::StartDiscovery(int discoveryPort) {
    if (_peerDiscovery) {
        _peerDiscovery->Start(discoveryPort);
        NetworkLogger::LogDiscovery("Node started peer discovery on port " +
                                    std::to_string(discoveryPort));
    }
}

void Node::StopDiscovery() {
    if (_peerDiscovery && _peerDiscovery->IsDiscovering()) {
        _peerDiscovery->Stop();
        NetworkLogger::LogDiscovery("Node stopped peer discovery");
    }
}

bool Node::IsDiscovering() const {
    return _peerDiscovery ? _peerDiscovery->IsDiscovering() : false;
}

std::vector<RakNet::SystemAddress> Node::GetDiscoveredPeers() const {
    return _peerDiscovery ? _peerDiscovery->GetDiscoveredPeers()
                          : std::vector<RakNet::SystemAddress>();
}

void Node::SetPeerDiscoveryCallback(
    std::function<void(const RakNet::SystemAddress &)> callback) {
    if (_peerDiscovery) {
        _peerDiscovery->SetDiscoveryCallback(callback);
    }
}

std::vector<RakNet::SystemAddress> Node::GetConnections() const {
    std::vector<RakNet::SystemAddress> result;
    if (_connectionManager) {
        auto connectionIds = _connectionManager->GetAllConnections();
        for (auto id : connectionIds) {
            result.push_back(_connectionManager->GetSystemAddress(id));
        }
    }
    return result;
}

bool Node::IsConnectedTo(const IpAddress &address, int port) const {
    if (!_connectionManager) return false;

    std::string addrStr = address.ToString() + ":" + std::to_string(port);
    for (auto conn : GetConnections()) {
        if (conn.ToString() == addrStr) {
            return true;
        }
    }
    return false;
}

size_t Node::GetConnectionCount() const {
    return _connectionManager ? _connectionManager->GetConnectionCount() : 0;
}

int Node::GetPing(const IpAddress &address, int port) const {
    if (!_peer || !_connectionManager) return -1;

    RakNet::SystemAddress systemAddr =
        RakNet::SystemAddress(address.ToString().c_str(), port);

    return _peer->GetAveragePing(systemAddr);
}

unsigned char Node::GetPacketIdentifier(RakNet::Packet *packet) {
    if (!packet || packet->length < 1) return 255;

    return (packet->data[0]);
}

void Node::ProcessPacket(RakNet::Packet *packet) {
    if (!packet) return;

    // Get the packet identifier
    unsigned char packetId = GetPacketIdentifier(packet);

    // Log the packet
    std::string packetType = "unknown";
    switch (packetId) {
        case RakNet::ID_CONNECTION_REQUEST_ACCEPTED:
            packetType = "connection request accepted";
            break;
        case RakNet::ID_CONNECTION_ATTEMPT_FAILED:
            packetType = "connection attempt failed";
            break;
        case RakNet::ID_ALREADY_CONNECTED:
            packetType = "already connected";
            break;
        case RakNet::ID_NEW_INCOMING_CONNECTION:
            packetType = "new incoming connection";
            break;
        case RakNet::ID_NO_FREE_INCOMING_CONNECTIONS:
            packetType = "no free incoming connections";
            break;
        case RakNet::ID_DISCONNECTION_NOTIFICATION:
            packetType = "disconnection notification";
            break;
        case RakNet::ID_CONNECTION_LOST:
            packetType = "connection lost";
            break;
        default:
            packetType = "custom type: " + std::to_string(packetId);
            break;
    }

    std::string logMessage = "Received packet of type '" + packetType +
                             "' from " + packet->systemAddress.ToString();
    NetworkLogger::LogMessage(logMessage);

    // Update connection activity
    if (_connectionManager) {
        _connectionManager->UpdateActivity(packet->systemAddress);
    }

    // Handle standard connection events
    switch (packetId) {
        case RakNet::ID_CONNECTION_REQUEST_ACCEPTED: {
            // We connected to another system
            OnConnectionEvent(
                _connectionManager->AddConnection(packet->systemAddress),
                ConnectionEvent::Connected);
            break;
        }
        case RakNet::ID_CONNECTION_ATTEMPT_FAILED: {
            // Connection attempt failed
            OnConnectionEvent(0, ConnectionEvent::ConnectionFailed);
            break;
        }
        case RakNet::ID_ALREADY_CONNECTED: {
            // We're already connected to this system
            // Re-use Connected event since AlreadyConnected is not defined
            OnConnectionEvent(
                _connectionManager->GetConnectionId(packet->systemAddress),
                ConnectionEvent::Connected);
            break;
        }
        case RakNet::ID_NEW_INCOMING_CONNECTION: {
            // A remote system connected to us
            OnConnectionEvent(
                _connectionManager->AddConnection(packet->systemAddress),
                ConnectionEvent::Connected);
            break;
        }
        case RakNet::ID_NO_FREE_INCOMING_CONNECTIONS: {
            // Remote system has no free incoming connections
            OnConnectionEvent(0, ConnectionEvent::ConnectionFailed);
            break;
        }
        case RakNet::ID_DISCONNECTION_NOTIFICATION: {
            // Remote system disconnected
            OnConnectionEvent(
                _connectionManager->GetConnectionId(packet->systemAddress),
                ConnectionEvent::Disconnected);
            _connectionManager->RemoveConnection(packet->systemAddress);
            break;
        }
        case RakNet::ID_CONNECTION_LOST: {
            // Connection lost
            OnConnectionEvent(
                _connectionManager->GetConnectionId(packet->systemAddress),
                ConnectionEvent::ConnectionLost);
            _connectionManager->RemoveConnection(packet->systemAddress);
            break;
        }
        default: {
            // Check if this is a custom message type
            if (packetId >= RakNet::ID_USER_PACKET_ENUM) {
                if (packetId == RakNet::ID_USER_PACKET_ENUM) {
                    // Process object message (serialized object)
                    ProcessObjectMessage(packet);
                } else if (packetId == RakNet::ID_USER_PACKET_ENUM + 1) {
                    // Process function call
                    ProcessFunctionCall(packet);
                } else if (packetId == RakNet::ID_USER_PACKET_ENUM + 2) {
                    // Process event notification
                    ProcessEventNotification(packet);
                }
            }
            break;
        }
    }
}

void Node::ProcessObjectMessage(RakNet::Packet *packet) {
    // TODO: Implement object message processing
    NetworkLogger::LogMessage(
        "Processing object message (not yet implemented)");
}

void Node::ProcessFunctionCall(RakNet::Packet *packet) {
    // TODO: Implement function call processing
    NetworkLogger::LogMessage("Processing function call (not yet implemented)");
}

void Node::ProcessEventNotification(RakNet::Packet *packet) {
    // TODO: Implement event notification processing
    NetworkLogger::LogMessage(
        "Processing event notification (not yet implemented)");
}

void Node::OnConnectionEvent(int connectionId, ConnectionEvent event) {
    // Log the connection event
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

    std::string logMessage = "Connection event: " + eventType +
                             " for connection ID " +
                             std::to_string(connectionId);
    NetworkLogger::LogConnection(logMessage);

    // TODO: Implement connection event handling
}

KAI_NET_END