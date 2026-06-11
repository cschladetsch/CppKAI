#include <enet/enet.h>

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "KAI/Network/Transport.h"

KAI_NET_BEGIN

namespace {
struct EnetInitGuard {
    EnetInitGuard() { Acquire(); }
    ~EnetInitGuard() { Release(); }

    static void Acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (refCount_ == 0) {
            enet_initialize();
        }
        ++refCount_;
    }

    static void Release() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (refCount_ == 0) {
            return;
        }
        --refCount_;
        if (refCount_ == 0) {
            enet_deinitialize();
        }
    }

    static std::mutex mutex_;
    static int refCount_;
};

std::mutex EnetInitGuard::mutex_;
int EnetInitGuard::refCount_ = 0;

std::string AddressKey(const ENetAddress& addr) {
    char ip[64] = {0};
    enet_address_get_host_ip(&addr, ip, sizeof(ip));
    return std::string(ip) + ":" + std::to_string(addr.port);
}

std::string NormalizeHost(std::string host) {
    if (host == "localhost" || host == "::1") {
        return "127.0.0.1";
    }
    return host;
}

NetAddress ToNetAddress(const ENetAddress& addr) {
    char ip[64] = {0};
    enet_address_get_host_ip(&addr, ip, sizeof(ip));
    return NetAddress(ip, addr.port);
}

bool ToEnetAddress(const NetAddress& addr, ENetAddress& out) {
    out.port = addr.port;
    // ENET_HOST_ANY is only appropriate for bind addresses (listen sockets).
    // Specific hosts — including loopback — must be resolved properly.
    const std::string host = NormalizeHost(addr.host);
    if (host.empty() || host == "0.0.0.0") {
        out.host = ENET_HOST_ANY;
        return true;
    }
    return enet_address_set_host(&out, host.c_str()) == 0;
}

struct MemoryState {
    std::mutex mutex;
    int nextEphemeralPort = 40000;
    std::unordered_map<std::string, NetPeer*> peers;
};

MemoryState& GetMemoryState() {
    static MemoryState state;
    return state;
}

std::string MemoryKey(const NetAddress& addr) {
    return NormalizeHost(addr.host) + ":" + std::to_string(addr.port);
}

unsigned char SystemMessageByte(SystemMessage msg) {
    return static_cast<unsigned char>(msg);
}
}  // namespace

class EnetPeer final : public NetPeer {
   public:
    EnetPeer() : initGuard_(std::make_unique<EnetInitGuard>()) {}

    ~EnetPeer() override { Shutdown(0); }

    bool Startup(int maxConnections, const NetAddress& bindAddress) override {
        if (host_) {
            return true;
        }

        ENetAddress addr;
        if (!ToEnetAddress(bindAddress, addr)) {
            return false;
        }

        host_ = enet_host_create(&addr, maxConnections, 2, 0, 0);
        if (host_) {
            memoryMode_ = false;
            return true;
        }

        memoryMode_ = true;
        maxConnections_ = maxConnections;
        memoryAddress_ = bindAddress;
        if (memoryAddress_.host.empty() || memoryAddress_.host == "0.0.0.0") {
            memoryAddress_.host = "127.0.0.1";
        }
        if (memoryAddress_.port == 0) {
            std::lock_guard<std::mutex> lock(GetMemoryState().mutex);
            memoryAddress_.port = static_cast<unsigned short>(
                GetMemoryState().nextEphemeralPort++);
        }

        std::lock_guard<std::mutex> lock(GetMemoryState().mutex);
        GetMemoryState().peers[MemoryKey(memoryAddress_)] = this;
        return true;
    }

    void Shutdown(int timeoutMs) override {
        if (memoryMode_) {
            std::vector<std::string> peersToNotify;
            {
                std::lock_guard<std::mutex> lock(memoryMutex_);
                peersToNotify.assign(memoryConnections_.begin(),
                                     memoryConnections_.end());
                memoryConnections_.clear();
                memoryInbox_.clear();
            }

            std::lock_guard<std::mutex> lock(GetMemoryState().mutex);
            for (const auto& peerKey : peersToNotify) {
                auto it = GetMemoryState().peers.find(peerKey);
                if (it == GetMemoryState().peers.end() ||
                    it->second == nullptr) {
                    continue;
                }
                auto* remote = dynamic_cast<EnetPeer*>(it->second);
                if (!remote) {
                    continue;
                }

                NetPacket packet;
                packet.address = memoryAddress_;
                packet.data.push_back(SystemMessageByte(
                    SystemMessage::DisconnectionNotification));
                remote->EnqueueMemoryPacket(packet);
                remote->RemoveMemoryConnection(memoryAddress_);
            }

            GetMemoryState().peers.erase(MemoryKey(memoryAddress_));
            memoryMode_ = false;
            return;
        }

        if (!host_) {
            return;
        }

        // Send graceful disconnect notification to all connected peers so they
        // can update their peer lists promptly rather than waiting for timeout.
        if (timeoutMs > 0 && !peersByAddress_.empty()) {
            for (auto& [key, peer] : peersByAddress_) {
                enet_peer_disconnect(peer, 0);
            }
            // Give ENet time to send the disconnect packets and receive acks.
            ENetEvent event;
            int waited = 0;
            const int pollMs = 10;
            while (waited < timeoutMs && !peersByAddress_.empty()) {
                if (enet_host_service(host_, &event, pollMs) > 0) {
                    if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                        peersByAddress_.erase(AddressKey(event.peer->address));
                    }
                }
                waited += pollMs;
            }
        }

        enet_host_destroy(host_);
        host_ = nullptr;
        peersByAddress_.clear();
        pendingOutgoing_.clear();
    }

    bool Connect(const NetAddress& remoteAddress) override {
        if (memoryMode_) {
            EnetPeer* remote = nullptr;
            {
                std::lock_guard<std::mutex> lock(GetMemoryState().mutex);
                auto it = GetMemoryState().peers.find(MemoryKey(remoteAddress));
                if (it != GetMemoryState().peers.end()) {
                    remote = dynamic_cast<EnetPeer*>(it->second);
                }
            }

            if (!remote) {
                return false;
            }

            memoryConnections_.insert(MemoryKey(remoteAddress));
            remote->memoryConnections_.insert(MemoryKey(memoryAddress_));

            NetPacket accepted;
            accepted.address = remoteAddress;
            accepted.data.push_back(
                SystemMessageByte(SystemMessage::ConnectionRequestAccepted));
            EnqueueMemoryPacket(accepted);

            NetPacket incoming;
            incoming.address = memoryAddress_;
            incoming.data.push_back(
                SystemMessageByte(SystemMessage::NewIncomingConnection));
            remote->EnqueueMemoryPacket(incoming);
            return true;
        }

        if (!host_) {
            return false;
        }

        ENetAddress addr;
        if (!ToEnetAddress(remoteAddress, addr)) {
            return false;
        }

        ENetPeer* peer = enet_host_connect(host_, &addr, 2, 0);
        if (!peer) {
            return false;
        }

        pendingOutgoing_.insert(AddressKey(addr));
        return true;
    }

    void SetMaximumIncomingConnections(int maxConnections) override {
        maxConnections_ = maxConnections;
    }

    std::unique_ptr<NetPacket> Receive() override {
        if (memoryMode_) {
            std::lock_guard<std::mutex> lock(memoryMutex_);
            if (memoryInbox_.empty()) {
                return nullptr;
            }

            auto packet = std::make_unique<NetPacket>();
            *packet = std::move(memoryInbox_.front());
            memoryInbox_.pop_front();
            return packet;
        }

        if (!host_) {
            return nullptr;
        }

        ENetEvent event;
        int result = enet_host_service(host_, &event, 0);
        if (result <= 0) {
            return nullptr;
        }

        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                auto packet = std::make_unique<NetPacket>();
                packet->address = ToNetAddress(event.peer->address);
                const std::string key = AddressKey(event.peer->address);
                if (pendingOutgoing_.erase(key) > 0) {
                    packet->data.push_back(static_cast<unsigned char>(
                        SystemMessage::ConnectionRequestAccepted));
                } else {
                    packet->data.push_back(static_cast<unsigned char>(
                        SystemMessage::NewIncomingConnection));
                }
                peersByAddress_[key] = event.peer;
                return packet;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                auto packet = std::make_unique<NetPacket>();
                packet->address = ToNetAddress(event.peer->address);
                packet->data.push_back(
                    static_cast<unsigned char>(SystemMessage::ConnectionLost));
                peersByAddress_.erase(AddressKey(event.peer->address));
                return packet;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                auto packet = std::make_unique<NetPacket>();
                packet->address = ToNetAddress(event.peer->address);
                packet->data.assign(
                    event.packet->data,
                    event.packet->data + event.packet->dataLength);
                enet_packet_destroy(event.packet);
                return packet;
            }
            default:
                break;
        }

        return nullptr;
    }

    bool Send(const unsigned char* data, std::size_t size, bool reliable,
              int channel, const NetAddress& target, bool broadcast) override {
        if (memoryMode_) {
            NetPacket packet;
            packet.address = memoryAddress_;
            packet.data.assign(data, data + size);

            if (broadcast) {
                std::vector<EnetPeer*> peers;
                {
                    std::lock_guard<std::mutex> lock(GetMemoryState().mutex);
                    for (const auto& [key, peer] : GetMemoryState().peers) {
                        auto* memoryPeer = dynamic_cast<EnetPeer*>(peer);
                        if (memoryPeer && key != MemoryKey(memoryAddress_)) {
                            peers.push_back(memoryPeer);
                        }
                    }
                }

                for (auto* peer : peers) {
                    peer->EnqueueMemoryPacket(packet);
                }
                return true;
            }

            EnetPeer* peer = nullptr;
            {
                std::lock_guard<std::mutex> lock(GetMemoryState().mutex);
                auto it = GetMemoryState().peers.find(MemoryKey(target));
                if (it != GetMemoryState().peers.end()) {
                    peer = dynamic_cast<EnetPeer*>(it->second);
                }
            }

            if (!peer) {
                return false;
            }

            peer->EnqueueMemoryPacket(packet);
            return true;
        }

        if (!host_) {
            return false;
        }

        const int flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
        ENetPacket* packet =
            enet_packet_create(data, size, static_cast<enet_uint32>(flags));
        if (!packet) {
            return false;
        }

        if (broadcast) {
            enet_host_broadcast(host_, channel, packet);
            return true;
        }

        ENetAddress addr;
        if (!ToEnetAddress(target, addr)) {
            enet_packet_destroy(packet);
            return false;
        }

        auto it = peersByAddress_.find(AddressKey(addr));
        if (it == peersByAddress_.end()) {
            enet_packet_destroy(packet);
            return false;
        }

        enet_peer_send(it->second, channel, packet);
        return true;
    }

    NetAddress GetInternalAddress() const override {
        if (memoryMode_) {
            return memoryAddress_;
        }

        if (!host_) {
            return NetAddress();
        }

        return ToNetAddress(host_->address);
    }

    int GetAveragePing(const NetAddress& address) const override {
        if (memoryMode_) {
            std::lock_guard<std::mutex> lock(memoryMutex_);
            return memoryConnections_.count(MemoryKey(address)) > 0 ? 0 : -1;
        }

        ENetAddress addr;
        if (!ToEnetAddress(address, addr)) {
            return -1;
        }

        auto it = peersByAddress_.find(AddressKey(addr));
        if (it == peersByAddress_.end() || it->second == nullptr) {
            return -1;
        }

        return static_cast<int>(it->second->roundTripTime);
    }

    void Ping(const NetAddress& /*address*/) override {}

    void SetOfflinePingResponse(const unsigned char* /*data*/,
                                std::size_t /*size*/) override {}

    void Flush() override {
        if (memoryMode_) {
            return;
        }

        if (host_) enet_host_flush(host_);
    }

    void EnqueueMemoryPacket(const NetPacket& packet) {
        std::lock_guard<std::mutex> lock(memoryMutex_);
        memoryInbox_.push_back(packet);
    }

    void RemoveMemoryConnection(const NetAddress& address) {
        std::lock_guard<std::mutex> lock(memoryMutex_);
        memoryConnections_.erase(MemoryKey(address));
    }

   private:
    std::unique_ptr<EnetInitGuard> initGuard_;
    ENetHost* host_ = nullptr;
    int maxConnections_ = 32;
    std::unordered_map<std::string, ENetPeer*> peersByAddress_;
    std::unordered_set<std::string> pendingOutgoing_;
    bool memoryMode_ = false;
    NetAddress memoryAddress_;
    std::deque<NetPacket> memoryInbox_;
    std::unordered_set<std::string> memoryConnections_;
    mutable std::mutex memoryMutex_;
};

std::unique_ptr<NetPeer> NetPeer::Create() {
    return std::make_unique<EnetPeer>();
}

KAI_NET_END
