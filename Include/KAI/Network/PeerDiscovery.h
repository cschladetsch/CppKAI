#pragma once

#include <functional>
#include <vector>

#include "KAI/Network/Network.h"
#include "KAI/Network/Transport.h"

KAI_NET_BEGIN

// Peer discovery class for finding nodes on the local network
class PeerDiscovery {
   public:
    // Callback type for peer discovery events
    using DiscoveryCallback = std::function<void(const NetAddress&)>;

    PeerDiscovery(NetPeer* peer);
    ~PeerDiscovery();

    // Start discovery of peers on the local network
    void Start(int discoveryPort = 14589);

    // Stop the discovery process
    void Stop();

    // Process discovery responses (call in update loop)
    void Update();

    // Set callback for discovery events
    void SetDiscoveryCallback(DiscoveryCallback callback);

    // Get discovered peers
    const std::vector<NetAddress>& GetDiscoveredPeers() const;

    // Check if discovery is in progress
    bool IsActive() const;

    // Check if discovery is in progress (alias for IsActive())
    bool IsDiscovering() const;

    // Clear list of discovered peers
    void ClearDiscoveredPeers();

   private:
    NetPeer* peer_;
    DiscoveryCallback callback_;
    std::vector<NetAddress> discoveredPeers_;
    bool isDiscovering_;
    int discoveryPort_;

    // Process a discovery response packet
    void ProcessDiscoveryResponse(const NetPacket& packet);
};

KAI_NET_END
