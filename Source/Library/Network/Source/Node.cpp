#include "KAI/Network/Node.h"

#include <iostream>

#include "KAI/Network/ConnectionManager.h"
#include "KAI/Network/NetworkLogger.h"
#include "KAI/Network/PeerDiscovery.h"
#include "KAI/Network/RakNetStub.h"
#include "KAI/Network/Serialization.h"

KAI_NET_BEGIN

Node::Node() : peer_(nullptr), isRunning_(false) {
    // Use an empty registry for now

    // Initialize RakNet
    peer_ = RakNet::RakPeerInterface::GetInstance();
    if (peer_ == nullptr) {
        std::cerr << "Failed to create RakNet peer interface" << std::endl;
        return;
    }

    // Create the connection manager
    connectionManager_ = std::make_unique<ConnectionManager>(peer_);

    // Create the peer discovery component
    peerDiscovery_ = std::make_unique<PeerDiscovery>(peer_);

    // Initialize NetworkLogger
    NetworkLogger::Init();
}

Node::~Node() {
    Shutdown();

    // Clean up RakNet
    if (peer_) {
        RakNet::RakPeerInterface::DestroyInstance(peer_);
        peer_ = nullptr;
    }
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

    // Max 32 connections, use all available network interfaces
    RakNet::SocketDescriptor sd(port, address.ToString().c_str());
    RakNet::StartupResult result = peer_->Startup(32, &sd, 1);

    if (result != RakNet::RAKNET_STARTED) {
        std::string errorMsg;

        // Provide more specific error messages for common issues
        switch (result) {
            case RakNet::SOCKET_PORT_ALREADY_IN_USE:
                errorMsg = "Failed to start RakNet server: Port " +
                           std::to_string(port) +
                           " is already in use. Try a different port.";
                break;
            case RakNet::SOCKET_FAILED_TO_BIND:
                errorMsg =
                    "Failed to start RakNet server: Could not bind to "
                    "address " +
                    address.ToString() + ":" + std::to_string(port);
                break;
            case RakNet::SOCKET_FAILED_TEST_SEND:
                errorMsg =
                    "Failed to start RakNet server: Test send failed, network "
                    "may be unavailable";
                break;
            default:
                errorMsg = "Failed to start RakNet server, error code: " +
                           std::to_string(result);
                break;
        }

        std::cerr << errorMsg << std::endl;
        NetworkLogger::LogStatus(errorMsg);
        return;
    }

    peer_->SetMaximumIncomingConnections(32);
    isRunning_ = true;

    // Log that we're listening
    std::string logMessage = "Network node listening on " + address.ToString() +
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
        // For loopback connections, it's better to explicitly bind to loopback
        RakNet::SocketDescriptor sd;

        // When connecting to loopback, bind on loopback interface
        if (ip.ToString() == "127.0.0.1" || ip.ToString() == "localhost") {
            sd.port = 0;  // Use any available port
            sd.hostAddress = "127.0.0.1";
        } else {
            sd.port = 0;               // Use any available port
            sd.hostAddress = nullptr;  // Use any available interface
        }

        RakNet::StartupResult result = peer_->Startup(32, &sd, 1);

        if (result != RakNet::RAKNET_STARTED) {
            std::string errorMsg;

            // Provide more specific error messages for common issues
            switch (result) {
                case RakNet::SOCKET_PORT_ALREADY_IN_USE:
                    errorMsg =
                        "Failed to start RakNet client: Port is already in use";
                    break;
                case RakNet::SOCKET_FAILED_TO_BIND:
                    errorMsg =
                        "Failed to start RakNet client: Could not bind to "
                        "address";
                    break;
                case RakNet::SOCKET_FAILED_TEST_SEND:
                    errorMsg =
                        "Failed to start RakNet client: Test send failed, "
                        "network may be unavailable";
                    break;
                default:
                    errorMsg = "Failed to start RakNet client, error code: " +
                               std::to_string(result);
                    break;
            }

            std::cerr << errorMsg << std::endl;
            NetworkLogger::LogStatus(errorMsg);
            return;
        }

        isRunning_ = true;

        // Log the port we're using
        int usedPort = peer_->GetInternalID().GetPort();
        std::string startupMsg =
            "Node started and bound to port " + std::to_string(usedPort);
        std::cout << startupMsg << std::endl;
        NetworkLogger::LogStatus(startupMsg);
    }

    // Connect to remote peer
    RakNet::ConnectionAttemptResult result =
        peer_->Connect(ip.ToString().c_str(), port, nullptr, 0);

    if (result != RakNet::CONNECTION_ATTEMPT_STARTED) {
        std::string errorMsg;

        // Provide more specific error messages
        switch (result) {
            case RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS:
                errorMsg = "Connection attempt to " + ip.ToString() + ":" +
                           std::to_string(port) + " already in progress";
                break;
            case RakNet::ALREADY_CONNECTED_TO_ENDPOINT:
                errorMsg = "Already connected to " + ip.ToString() + ":" +
                           std::to_string(port);
                break;
            case RakNet::CONNECTION_ATTEMPT_STARTED:
                errorMsg = "Connection attempt started";
                break;
            case RakNet::CANNOT_RESOLVE_DOMAIN_NAME:
                errorMsg = "Cannot resolve domain name: " + ip.ToString();
                break;
            case RakNet::INVALID_PARAMETER:
                errorMsg = "Invalid parameter when connecting to " +
                           ip.ToString() + ":" + std::to_string(port);
                break;
            default:
                errorMsg = "Failed to connect to " + ip.ToString() + ":" +
                           std::to_string(port) +
                           ", error code: " + std::to_string(result);
                break;
        }

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
    RakNet::Packet *packet = nullptr;
    while ((packet = peer_->Receive()) != nullptr) {
        processedPackets = true;
        ProcessPacket(packet);
        peer_->DeallocatePacket(packet);
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

std::vector<RakNet::SystemAddress> Node::GetDiscoveredPeers() const {
    return peerDiscovery_ ? peerDiscovery_->GetDiscoveredPeers()
                          : std::vector<RakNet::SystemAddress>();
}

void Node::SetPeerDiscoveryCallback(
    std::function<void(const RakNet::SystemAddress &)> callback) {
    if (peerDiscovery_) {
        peerDiscovery_->SetDiscoveryCallback(callback);
    }
}

std::vector<RakNet::SystemAddress> Node::GetConnections() const {
    std::vector<RakNet::SystemAddress> result;
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

    // Create a proper SystemAddress for comparison
    RakNet::SystemAddress targetAddr(address.ToString().c_str(), port);

    // Get all connections and check each one
    for (auto conn : GetConnections()) {
        // Compare IPs and ports separately to handle different string
        // representations
        if (conn.GetPort() == targetAddr.GetPort() &&
            conn.ToString(false) == targetAddr.ToString(false)) {
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

    RakNet::SystemAddress systemAddr =
        RakNet::SystemAddress(address.ToString().c_str(), port);

    return peer_->GetAveragePing(systemAddr);
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
    if (connectionManager_) {
        connectionManager_->UpdateActivity(packet->systemAddress);
    }

    // Handle standard connection events
    switch (packetId) {
        case RakNet::ID_CONNECTION_REQUEST_ACCEPTED: {
            // We connected to another system
            OnConnectionEvent(
                connectionManager_->AddConnection(packet->systemAddress),
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
                connectionManager_->GetConnectionId(packet->systemAddress),
                ConnectionEvent::Connected);
            break;
        }
        case RakNet::ID_NEW_INCOMING_CONNECTION: {
            // A remote system connected to us
            OnConnectionEvent(
                connectionManager_->AddConnection(packet->systemAddress),
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
                connectionManager_->GetConnectionId(packet->systemAddress),
                ConnectionEvent::Disconnected);
            connectionManager_->RemoveConnection(packet->systemAddress);
            break;
        }
        case RakNet::ID_CONNECTION_LOST: {
            // Connection lost
            OnConnectionEvent(
                connectionManager_->GetConnectionId(packet->systemAddress),
                ConnectionEvent::ConnectionLost);
            connectionManager_->RemoveConnection(packet->systemAddress);
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