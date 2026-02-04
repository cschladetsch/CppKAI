#include "KAI/Network/Transport.h"

#include <enet/enet.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

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

NetAddress ToNetAddress(const ENetAddress& addr) {
    char ip[64] = {0};
    enet_address_get_host_ip(&addr, ip, sizeof(ip));
    return NetAddress(ip, addr.port);
}

bool ToEnetAddress(const NetAddress& addr, ENetAddress& out) {
    out.port = addr.port;
    if (addr.host.empty() || addr.host == "0.0.0.0" ||
        addr.host == "127.0.0.1") {
        out.host = ENET_HOST_ANY;
        return true;
    }
    return enet_address_set_host(&out, addr.host.c_str()) == 0;
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
        return host_ != nullptr;
    }

    void Shutdown(int timeoutMs) override {
        if (!host_) {
            return;
        }

        if (timeoutMs > 0) {
            enet_host_flush(host_);
        }

        enet_host_destroy(host_);
        host_ = nullptr;
        peersByAddress_.clear();
        pendingOutgoing_.clear();
    }

    bool Connect(const NetAddress& remoteAddress) override {
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
                    packet->data.push_back(
                        static_cast<unsigned char>(
                            SystemMessage::ConnectionRequestAccepted));
                } else {
                    packet->data.push_back(
                        static_cast<unsigned char>(
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
                packet->data.assign(event.packet->data,
                                    event.packet->data +
                                        event.packet->dataLength);
                enet_packet_destroy(event.packet);
                return packet;
            }
            default:
                break;
        }

        return nullptr;
    }

    bool Send(const unsigned char* data, std::size_t size, bool reliable,
              int channel, const NetAddress& target,
              bool broadcast) override {
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
        if (!host_) {
            return NetAddress();
        }

        return ToNetAddress(host_->address);
    }

    int GetAveragePing(const NetAddress& address) const override {
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

   private:
    std::unique_ptr<EnetInitGuard> initGuard_;
    ENetHost* host_ = nullptr;
    int maxConnections_ = 32;
    std::unordered_map<std::string, ENetPeer*> peersByAddress_;
    std::unordered_set<std::string> pendingOutgoing_;
};

std::unique_ptr<NetPeer> NetPeer::Create() {
    return std::make_unique<EnetPeer>();
}

KAI_NET_END
