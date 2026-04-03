#include "KAI/Network/PeerDiscovery.h"

#include <chrono>
#include <iostream>

KAI_NET_BEGIN

namespace {
int64_t NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}
}  // namespace

PeerDiscovery::PeerDiscovery(NetPeer* peer)
    : peer_(peer), isDiscovering_(false), discoveryPort_(14589) {}

PeerDiscovery::~PeerDiscovery() { Stop(); }

void PeerDiscovery::Start(int discoveryPort) {
    if (!peer_) return;

    discoveryPort_ = discoveryPort;

    // Start sending discovery pings to find other peers
    peer_->Ping(NetAddress("255.255.255.255",
                           static_cast<unsigned short>(discoveryPort_)));

    // Enable responding to discovery pings
    static const unsigned char response[] = {'K', 'A', 'I', 'N', 'o', 'd', 'e'};
    peer_->SetOfflinePingResponse(response, sizeof(response));

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
    static int64_t lastPingTime = 0;
    int64_t currentTime = NowMs();

    if (currentTime - lastPingTime > 5000) {  // Ping every 5 seconds
        peer_->Ping(NetAddress("255.255.255.255",
                               static_cast<unsigned short>(discoveryPort_)));
        lastPingTime = currentTime;
    }

    // Process discovery responses
    while (true) {
        auto packet = peer_->Receive();
        if (!packet) {
            break;
        }
        if (!packet->data.empty() &&
            packet->data[0] ==
                static_cast<unsigned char>(SystemMessage::UnconnectedPong)) {
            ProcessDiscoveryResponse(*packet);
        }
    }
}

void PeerDiscovery::SetDiscoveryCallback(DiscoveryCallback callback) {
    callback_ = callback;
}

const std::vector<NetAddress>& PeerDiscovery::GetDiscoveredPeers() const {
    return discoveredPeers_;
}

bool PeerDiscovery::IsActive() const { return isDiscovering_; }

bool PeerDiscovery::IsDiscovering() const { return IsActive(); }

void PeerDiscovery::ClearDiscoveredPeers() { discoveredPeers_.clear(); }

void PeerDiscovery::ProcessDiscoveryResponse(const NetPacket& packet) {
    if (packet.data.empty()) return;

    // See if we already know about this peer
    bool alreadyDiscovered = false;
    for (const auto& addr : discoveredPeers_) {
        if (addr == packet.address) {
            alreadyDiscovered = true;
            break;
        }
    }

    // If this is a new peer, add it to our list
    if (!alreadyDiscovered) {
        discoveredPeers_.push_back(packet.address);

        // Notify callback if set
        if (callback_) {
            callback_(packet.address);
        }

        std::cout << "Discovered peer: " << packet.address.ToString()
                  << std::endl;
    }
}

KAI_NET_END
