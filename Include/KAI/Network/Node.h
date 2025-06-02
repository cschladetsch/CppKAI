#pragma once

#include "KAI/Network/Network.h"
#include "KAI/Network/RakNetStub.h"  // Include RakNetStub.h directly

KAI_NET_BEGIN

// Forward declarations for KAI network classes
enum class ConnectionEvent;
struct ProxyBase;
struct AgentBase;

// A peer on the network. Nominally, there are no servers or clients. Just
// a collection of nodes that connect and communicate with each other.
// Forward declarations
class ConnectionManager;
class PeerDiscovery;

struct Node {
   private:
    RakNet::RakPeerInterface *peer_;
    bool isRunning_;
    std::unique_ptr<ConnectionManager> connectionManager_;
    std::unique_ptr<PeerDiscovery> peerDiscovery_;

   public:
    static int constexpr DefaultPort = 14589;

    Node();
    ~Node();

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
    std::vector<RakNet::SystemAddress> GetDiscoveredPeers() const;
    void SetPeerDiscoveryCallback(
        std::function<void(const RakNet::SystemAddress &)> callback);

    // Get all active connections
    std::vector<RakNet::SystemAddress> GetConnections() const;

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

   private:
    void ProcessPacket(RakNet::Packet *packet);
    void ProcessObjectMessage(RakNet::Packet *packet);
    void ProcessFunctionCall(RakNet::Packet *packet);
    void ProcessEventNotification(RakNet::Packet *packet);
    void OnConnectionEvent(int connectionId, ConnectionEvent event);

    // Helper method to get the packet identifier
    unsigned char GetPacketIdentifier(RakNet::Packet *packet);

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
};

KAI_NET_END
