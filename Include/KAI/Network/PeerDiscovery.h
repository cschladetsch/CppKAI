#pragma once

#include "KAI/Network/Network.h"
#include <functional>
#include <vector>

namespace RakNet {
    class RakPeerInterface;
    class Packet;
    struct SystemAddress;
}

KAI_NET_BEGIN

// Peer discovery class for finding nodes on the local network
class PeerDiscovery
{
public:
    // Callback type for peer discovery events
    using DiscoveryCallback = std::function<void(const RakNet::SystemAddress&)>;
    
    PeerDiscovery(RakNet::RakPeerInterface* peer);
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
    const std::vector<RakNet::SystemAddress>& GetDiscoveredPeers() const;
    
    // Check if discovery is in progress
    bool IsActive() const;
    
    // Clear list of discovered peers
    void ClearDiscoveredPeers();

private:
    RakNet::RakPeerInterface* _peer;
    DiscoveryCallback _callback;
    std::vector<RakNet::SystemAddress> _discoveredPeers;
    bool _isDiscovering;
    int _discoveryPort;
    
    // Process a discovery response packet
    void ProcessDiscoveryResponse(RakNet::Packet* packet);
};

KAI_NET_END