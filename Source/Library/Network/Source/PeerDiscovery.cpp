#include "KAI/Network/PeerDiscovery.h"

#include <iostream>

#include "KAI/Network/RakNetStub.h"

KAI_NET_BEGIN

PeerDiscovery::PeerDiscovery(RakNet::RakPeerInterface* peer)
    : peer_(peer), isDiscovering_(false), discoveryPort_(14589) {}

PeerDiscovery::~PeerDiscovery() { Stop(); }

void PeerDiscovery::Start(int discoveryPort) {
    if (!peer_) return;

    discoveryPort_ = discoveryPort;

    // Start sending discovery pings to find other peers
    peer_->Ping("255.255.255.255", discoveryPort_, false);

    // Enable responding to discovery pings
    peer_->SetOfflinePingResponse("KAINode", 7);

    isDiscovering_ = true;
    std::cout << "Started peer discovery on port " << discoveryPort_
              << std::endl;
}

void PeerDiscovery::Stop() {
    if (!peer_) return;

    isDiscovering_ = false;
    std::cout << "Stopped peer discovery" << std::endl;
}

void PeerDiscovery::Update() {
    if (!peer_ || !isDiscovering_) return;

    // Periodically send out discovery pings
    static RakNet::TimeMS lastPingTime = 0;
    RakNet::TimeMS currentTime = RakNet::GetTimeMS();

    if (currentTime - lastPingTime > 5000) {  // Ping every 5 seconds
        peer_->Ping("255.255.255.255", discoveryPort_, false);
        lastPingTime = currentTime;
    }

    // Process discovery responses
    RakNet::Packet* packet;
    while ((packet = peer_->Receive()) != nullptr) {
        unsigned char packetId = packet->data[0];

        if (packetId == RakNet::ID_UNCONNECTED_PONG) {
            ProcessDiscoveryResponse(packet);
        }

        peer_->DeallocatePacket(packet);
    }
}

void PeerDiscovery::SetDiscoveryCallback(DiscoveryCallback callback) {
    callback_ = callback;
}

const std::vector<RakNet::SystemAddress>& PeerDiscovery::GetDiscoveredPeers()
    const {
    return discoveredPeers_;
}

bool PeerDiscovery::IsActive() const { return isDiscovering_; }

bool PeerDiscovery::IsDiscovering() const { return IsActive(); }

void PeerDiscovery::ClearDiscoveredPeers() { discoveredPeers_.clear(); }

void PeerDiscovery::ProcessDiscoveryResponse(RakNet::Packet* packet) {
    if (!packet) return;

    // Extract the data from the pong packet
    RakNet::TimeMS time = 0;
    RakNet::BitStream bs(packet->data, packet->length, false);

    bs.IgnoreBytes(1);  // Skip message ID
    bs.Read(time);      // Read time (ping response time)

    // See if we already know about this peer
    bool alreadyDiscovered = false;
    for (const auto& addr : discoveredPeers_) {
        if (addr == packet->systemAddress) {
            alreadyDiscovered = true;
            break;
        }
    }

    // If this is a new peer, add it to our list
    if (!alreadyDiscovered) {
        discoveredPeers_.push_back(packet->systemAddress);

        // Notify callback if set
        if (callback_) {
            callback_(packet->systemAddress);
        }

        std::cout << "Discovered peer: " << packet->systemAddress.ToString()
                  << std::endl;
    }
}

KAI_NET_END