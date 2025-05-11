#include "KAI/Network/PeerDiscovery.h"

#include <iostream>

#include "KAI/Network/RakNetStub.h"

KAI_NET_BEGIN

PeerDiscovery::PeerDiscovery(RakNet::RakPeerInterface* peer)
    : _peer(peer), _isDiscovering(false), _discoveryPort(14589) {}

PeerDiscovery::~PeerDiscovery() { Stop(); }

void PeerDiscovery::Start(int discoveryPort) {
    if (!_peer) return;

    _discoveryPort = discoveryPort;

    // Start sending discovery pings to find other peers
    _peer->Ping("255.255.255.255", _discoveryPort, false);

    // Enable responding to discovery pings
    _peer->SetOfflinePingResponse("KAINode", 7);

    _isDiscovering = true;
    std::cout << "Started peer discovery on port " << _discoveryPort
              << std::endl;
}

void PeerDiscovery::Stop() {
    if (!_peer) return;

    _isDiscovering = false;
    std::cout << "Stopped peer discovery" << std::endl;
}

void PeerDiscovery::Update() {
    if (!_peer || !_isDiscovering) return;

    // Periodically send out discovery pings
    static RakNet::TimeMS lastPingTime = 0;
    RakNet::TimeMS currentTime = RakNet::GetTimeMS();

    if (currentTime - lastPingTime > 5000) {  // Ping every 5 seconds
        _peer->Ping("255.255.255.255", _discoveryPort, false);
        lastPingTime = currentTime;
    }

    // Process discovery responses
    RakNet::Packet* packet;
    while ((packet = _peer->Receive()) != nullptr) {
        unsigned char packetId = packet->data[0];

        if (packetId == RakNet::ID_UNCONNECTED_PONG) {
            ProcessDiscoveryResponse(packet);
        }

        _peer->DeallocatePacket(packet);
    }
}

void PeerDiscovery::SetDiscoveryCallback(DiscoveryCallback callback) {
    _callback = callback;
}

const std::vector<RakNet::SystemAddress>& PeerDiscovery::GetDiscoveredPeers()
    const {
    return _discoveredPeers;
}

bool PeerDiscovery::IsActive() const { return _isDiscovering; }

bool PeerDiscovery::IsDiscovering() const { return IsActive(); }

void PeerDiscovery::ClearDiscoveredPeers() { _discoveredPeers.clear(); }

void PeerDiscovery::ProcessDiscoveryResponse(RakNet::Packet* packet) {
    if (!packet) return;

    // Extract the data from the pong packet
    RakNet::TimeMS time = 0;
    RakNet::BitStream bs(packet->data, packet->length, false);

    bs.IgnoreBytes(1);  // Skip message ID
    bs.Read(time);      // Read time (ping response time)

    // See if we already know about this peer
    bool alreadyDiscovered = false;
    for (const auto& addr : _discoveredPeers) {
        if (addr == packet->systemAddress) {
            alreadyDiscovered = true;
            break;
        }
    }

    // If this is a new peer, add it to our list
    if (!alreadyDiscovered) {
        _discoveredPeers.push_back(packet->systemAddress);

        // Notify callback if set
        if (_callback) {
            _callback(packet->systemAddress);
        }

        std::cout << "Discovered peer: " << packet->systemAddress.ToString()
                  << std::endl;
    }
}

KAI_NET_END