#pragma once

#ifdef KAI_USE_ENET
#include <enet/enet.h>
#endif

#include <algorithm>
#include <atomic>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace RakNet {
// Common types
typedef unsigned int TimeMS;

// Get current time in milliseconds
inline TimeMS GetTimeMS() {
    return 1000;  // Stub implementation always returns 1000
}

// Message IDs
enum MessageID {
    ID_CONNECTED_PING,
    ID_UNCONNECTED_PING,
    ID_UNCONNECTED_PING_OPEN_CONNECTIONS,
    ID_CONNECTED_PONG,
    ID_UNCONNECTED_PONG,
    ID_CONNECTION_REQUEST,
    ID_CONNECTION_REQUEST_ACCEPTED,
    ID_CONNECTION_ATTEMPT_FAILED,
    ID_ALREADY_CONNECTED,
    ID_NEW_INCOMING_CONNECTION,
    ID_NO_FREE_INCOMING_CONNECTIONS,
    ID_DISCONNECTION_NOTIFICATION,
    ID_CONNECTION_LOST,
    ID_CONNECTION_BANNED,
    ID_INVALID_PASSWORD,
    ID_INCOMPATIBLE_PROTOCOL_VERSION,
    ID_TIMESTAMP,
    ID_USER_PACKET_ENUM
};

// Priority constants
enum PacketPriority {
    IMMEDIATE_PRIORITY,
    HIGH_PRIORITY,
    MEDIUM_PRIORITY,
    LOW_PRIORITY
};

// Reliability constants  
enum PacketReliability {
    UNRELIABLE,
    UNRELIABLE_SEQUENCED,
    RELIABLE,
    RELIABLE_ORDERED,
    RELIABLE_SEQUENCED
};

// System address class
class SystemAddress {
   public:
    std::string ip;
    unsigned short port;

    SystemAddress() : ip("0.0.0.0"), port(0) {}

    SystemAddress(const char* _ip, unsigned short _port)
        : ip(_ip), port(_port) {}

    std::string ToString() const { return ip + ":" + std::to_string(port); }

    std::string ToString(bool withPort) const {
        if (withPort) {
            return ip + ":" + std::to_string(port);
        }
        return ip;
    }

    unsigned short GetPort() const { return port; }

    void FromString(const char* str) {
        std::string s(str);
        size_t pos = s.find(":");
        if (pos != std::string::npos) {
            ip = s.substr(0, pos);
            port = std::stoi(s.substr(pos + 1));
        }
    }

    // Overload that also sets port directly
    void FromString(const char* str, int& port_out) {
        std::string s(str);
        size_t pos = s.find(":");
        if (pos != std::string::npos) {
            ip = s.substr(0, pos);
            port = std::stoi(s.substr(pos + 1));
        } else {
            ip = s;
        }
        port_out = port;
    }

    bool operator==(const SystemAddress& other) const {
        return ip == other.ip && port == other.port;
    }

    bool operator!=(const SystemAddress& other) const {
        return !(*this == other);
    }
};

// Unassigned system address constant
const SystemAddress UNASSIGNED_SYSTEM_ADDRESS("0.0.0.0", 0);

// BitStream class for serialization
class BitStream {
   private:
    std::vector<unsigned char> data;
    size_t readPos;

   public:
    BitStream() : readPos(0) {}

    // Constructor taking pointer to data and length
    BitStream(unsigned char* data_, size_t length, bool /*copyData*/) : readPos(0) {
        if (data_ != nullptr && length > 0) {
            data.resize(length);
            memcpy(data.data(), data_, length);
        }
    }

    // Write functions for various types
    void Write(unsigned int value) {
        const unsigned char* bytes =
            reinterpret_cast<const unsigned char*>(&value);
        for (size_t i = 0; i < sizeof(unsigned int); i++) {
            data.push_back(bytes[i]);
        }
    }

    void Write(int value) {
        unsigned int* ptr = reinterpret_cast<unsigned int*>(&value);
        Write(*ptr);
    }

    void Write(float value) {
        const unsigned char* bytes =
            reinterpret_cast<const unsigned char*>(&value);
        for (size_t i = 0; i < sizeof(float); i++) {
            data.push_back(bytes[i]);
        }
    }

    void Write(const char* str, size_t length) {
        for (size_t i = 0; i < length; i++) {
            data.push_back(static_cast<unsigned char>(str[i]));
        }
    }

    void Write(const std::string& str) {
        Write(static_cast<unsigned int>(str.length()));
        Write(str.c_str(), str.length());
    }

    void Write(unsigned char value) { data.push_back(value); }

    // Read functions for various types
    bool Read(unsigned int& value) {
        if (readPos + sizeof(unsigned int) <= data.size()) {
            memcpy(&value, &data[readPos], sizeof(unsigned int));
            readPos += sizeof(unsigned int);
            return true;
        }
        return false;
    }

    bool Read(int& value) {
        unsigned int temp;
        if (Read(temp)) {
            value = *reinterpret_cast<int*>(&temp);
            return true;
        }
        return false;
    }

    bool Read(float& value) {
        if (readPos + sizeof(float) <= data.size()) {
            memcpy(&value, &data[readPos], sizeof(float));
            readPos += sizeof(float);
            return true;
        }
        return false;
    }

    bool Read(char* output, size_t length) {
        if (readPos + length <= data.size()) {
            memcpy(output, &data[readPos], length);
            readPos += length;
            return true;
        }
        return false;
    }

    bool Read(std::string& str) {
        unsigned int length;
        if (!Read(length)) {
            return false;
        }

        if (readPos + length > data.size()) {
            return false;
        }

        str.resize(length);
        if (length > 0) {
            memcpy(&str[0], &data[readPos], length);
            readPos += length;
        }
        return true;
    }

    bool Read(unsigned char& value) {
        if (readPos < data.size()) {
            value = data[readPos++];
            return true;
        }
        return false;
    }

    // Utility functions
    size_t GetNumberOfBytesUsed() const { return data.size(); }

    void IgnoreBytes(size_t bytes) { readPos = bytes; }

    void Reset() { readPos = 0; }

    const unsigned char* GetData() const { return data.data(); }

    std::vector<unsigned char>::iterator begin() { return data.begin(); }

    std::vector<unsigned char>::iterator end() { return data.end(); }
};

// GUID
struct RakNetGUID {
    unsigned int g;

    RakNetGUID() : g(0) {}
    bool operator==(const RakNetGUID& other) const { return g == other.g; }
};

// Network packet
struct Packet {
    SystemAddress systemAddress;
    unsigned char* data;
    size_t length;

    Packet() : data(nullptr), length(0) {}
    ~Packet() {
        if (data) delete[] data;
    }
};

// Connection states and results
enum StartupResult {
    RAKNET_STARTED,
    RAKNET_ALREADY_STARTED,
    INVALID_SOCKET_DESCRIPTORS,
    INVALID_MAX_CONNECTIONS,
    SOCKET_FAILED_TO_BIND,
    SOCKET_PORT_ALREADY_IN_USE,
    SOCKET_FAILED_TEST_SEND,
    PORT_CANNOT_BE_ZERO
};

enum ConnectionAttemptResult {
    CONNECTION_ATTEMPT_STARTED,
    INVALID_PARAMETER,
    CANNOT_RESOLVE_DOMAIN_NAME,
    ALREADY_CONNECTED_TO_ENDPOINT,
    CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS,
    SYSTEM_ADDRESS_IS_BLACKLISTED
};

// Connection state enum
enum ConnectionState {
    IS_DISCONNECTED,
    IS_CONNECTING,
    IS_CONNECTED,
    IS_DISCONNECTING
};

// Socket descriptor
struct SocketDescriptor {
    unsigned short port;
    const char* hostAddress;

    SocketDescriptor() : port(0), hostAddress(nullptr) {}
    SocketDescriptor(unsigned short _port, const char* _host)
        : port(_port), hostAddress(_host) {}
};

// Main RakPeer interface
class RakPeerInterface {
   public:
    static RakPeerInterface* GetInstance() { return new RakPeerInterface(); }

    static void DestroyInstance(RakPeerInterface* instance) { delete instance; }

    StartupResult Startup(unsigned int maxConnections,
                          SocketDescriptor* socketDescriptors,
                          unsigned short socketDescriptorCount) {
#ifdef KAI_USE_ENET
        if (socketDescriptorCount == 0 || socketDescriptors == nullptr) {
            return INVALID_SOCKET_DESCRIPTORS;
        }

        if (!EnsureEnetInitialized()) {
            return SOCKET_FAILED_TEST_SEND;
        }

        listenPort_ = socketDescriptors[0].port;
        listenIp_ = socketDescriptors[0].hostAddress ? socketDescriptors[0].hostAddress : "127.0.0.1";
        maxConnections_ = maxConnections;

        ENetAddress address;
        if (socketDescriptors[0].hostAddress && *socketDescriptors[0].hostAddress) {
            if (enet_address_set_host(&address, socketDescriptors[0].hostAddress) != 0) {
                return SOCKET_FAILED_TO_BIND;
            }
        } else {
            address.host = ENET_HOST_ANY;
        }
        address.port = listenPort_;

        {
            std::lock_guard<std::mutex> lock(hostMutex_);
            host_ = enet_host_create(&address, maxConnections_ ? maxConnections_ : 32, 2, 0, 0);
        }

        if (!host_) {
            return SOCKET_FAILED_TO_BIND;
        }

        // Update port if zero was supplied.
        listenPort_ = host_->address.port;
        started_ = true;
        guid_.g = NextGuid();

        return RAKNET_STARTED;
#else
        if (socketDescriptorCount == 0 || socketDescriptors == nullptr) {
            return INVALID_SOCKET_DESCRIPTORS;
        }

        listenPort_ = socketDescriptors[0].port;
        listenIp_ = socketDescriptors[0].hostAddress ? socketDescriptors[0].hostAddress : "127.0.0.1";
        maxConnections_ = maxConnections;
        started_ = true;
        guid_.g = NextGuid();

        std::lock_guard<std::mutex> lock(globalMutex_);
        peersByPort_[listenPort_] = this;
        return RAKNET_STARTED;
#endif
    }

    void Shutdown(unsigned int /*blockDuration*/) {
#ifdef KAI_USE_ENET
        std::lock_guard<std::mutex> lock(hostMutex_);
        if (host_) {
            enet_host_destroy(host_);
            host_ = nullptr;
        }
        {
            std::lock_guard<std::mutex> connLock(connectionMutex_);
            connections_.clear();
            pendingOutgoing_.clear();
        }
        started_ = false;
        ReleaseEnet();
#else
        std::vector<RakPeerInterface*> peersToNotify;
        {
            std::lock_guard<std::mutex> lock(connectionMutex_);
            for (const auto& conn : connections_) {
                peersToNotify.push_back(conn.peer);
            }
            connections_.clear();
        }

        SystemAddress selfAddress(listenIp_.c_str(), listenPort_);
        for (auto* peer : peersToNotify) {
            peer->RemoveConnectionWithPeer(this);
            peer->QueueSystemMessage(selfAddress, ID_CONNECTION_LOST);
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            while (!incomingPackets_.empty()) {
                delete incomingPackets_.front();
                incomingPackets_.pop();
            }
        }

        {
            std::lock_guard<std::mutex> lock(globalMutex_);
            auto it = peersByPort_.find(listenPort_);
            if (it != peersByPort_.end() && it->second == this) {
                peersByPort_.erase(it);
            }
        }

        started_ = false;
#endif
    }

    void SetMaximumIncomingConnections(unsigned short numberAllowed) {
        maxConnections_ = numberAllowed;
    }

    ConnectionAttemptResult Connect(const char* host, unsigned short remotePort,
                                    const char* /*passwordData*/, int /*passwordLength*/) {
#ifdef KAI_USE_ENET
        if (!host_ || !started_) {
            return INVALID_PARAMETER;
        }

        ENetAddress address;
        if (!host || !*host || enet_address_set_host(&address, host) != 0) {
            return CANNOT_RESOLVE_DOMAIN_NAME;
        }
        address.port = remotePort;

        std::lock_guard<std::mutex> lock(hostMutex_);
        ENetPeer* peer = enet_host_connect(host_, &address, 2, 0);
        if (!peer) {
            return INVALID_PARAMETER;
        }
        {
            std::lock_guard<std::mutex> connLock(connectionMutex_);
            pendingOutgoing_.insert(peer);
        }
        return CONNECTION_ATTEMPT_STARTED;
#else
        RakPeerInterface* remote = nullptr;
        {
            std::lock_guard<std::mutex> lock(globalMutex_);
            auto it = peersByPort_.find(remotePort);
            if (it == peersByPort_.end()) {
                return CANNOT_RESOLVE_DOMAIN_NAME;
            }
            remote = it->second;
        }

        if (remote == this) {
            return ALREADY_CONNECTED_TO_ENDPOINT;
        }

        if (!remote->started_) {
            return CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS;
        }

        {
            std::lock_guard<std::mutex> lock(remote->connectionMutex_);
            if (remote->maxConnections_ > 0 &&
                remote->connections_.size() >= remote->maxConnections_) {
                return SYSTEM_ADDRESS_IS_BLACKLISTED;
            }
        }

        SystemAddress remoteAddress(host && *host ? host : remote->listenIp_.c_str(), remotePort);
        SystemAddress localAddress(listenIp_.c_str(), listenPort_);

        AddConnection(remoteAddress, remote);
        remote->AddConnection(localAddress, this);

        QueueSystemMessage(remoteAddress, ID_CONNECTION_REQUEST_ACCEPTED);
        remote->QueueSystemMessage(localAddress, ID_NEW_INCOMING_CONNECTION);

        return CONNECTION_ATTEMPT_STARTED;
#endif
    }

    Packet* Receive() {
#ifdef KAI_USE_ENET
        std::lock_guard<std::mutex> lock(hostMutex_);
        if (!host_) {
            return nullptr;
        }

        ENetEvent event;
        int serviced = enet_host_service(host_, &event, 0);
        if (serviced <= 0) {
            return nullptr;
        }

        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            SystemAddress addr = EnetToSystemAddress(event.peer->address);
            AddConnection(addr, event.peer);

            bool isOutgoing = false;
            {
                std::lock_guard<std::mutex> connLock(connectionMutex_);
                auto it = pendingOutgoing_.find(event.peer);
                if (it != pendingOutgoing_.end()) {
                    pendingOutgoing_.erase(it);
                    isOutgoing = true;
                }
            }

            QueueSystemMessage(addr, isOutgoing ? ID_CONNECTION_REQUEST_ACCEPTED
                                                : ID_NEW_INCOMING_CONNECTION);
        } else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            Packet* packet = new Packet();
            packet->systemAddress = EnetToSystemAddress(event.peer->address);
            packet->length = event.packet->dataLength;
            packet->data = new unsigned char[packet->length];
            memcpy(packet->data, event.packet->data, packet->length);
            enet_packet_destroy(event.packet);
            return packet;
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            SystemAddress addr = EnetToSystemAddress(event.peer->address);
            RemoveConnectionWithPeer(event.peer);
            QueueSystemMessage(addr, ID_CONNECTION_LOST);
        }

        // If we queued a system message, fall through to pop it.
        {
            std::lock_guard<std::mutex> qlock(queueMutex_);
            if (incomingPackets_.empty()) {
                return nullptr;
            }
            Packet* packet = incomingPackets_.front();
            incomingPackets_.pop();
            return packet;
        }
#else
        std::lock_guard<std::mutex> lock(queueMutex_);
        if (incomingPackets_.empty()) {
            return nullptr;
        }
        Packet* packet = incomingPackets_.front();
        incomingPackets_.pop();
        return packet;
#endif
    }

    void DeallocatePacket(Packet* packet) { delete packet; }

    bool Send(const char* data, int length, PacketPriority priority, PacketReliability reliability,
              char channel, SystemAddress systemAddress, bool broadcast) {
        BitStream stream(reinterpret_cast<unsigned char*>(const_cast<char*>(data)),
                         static_cast<size_t>(length), true);
        return Send(&stream, priority, reliability, channel, systemAddress, broadcast);
    }

    bool Send(BitStream* bitStream, PacketPriority /*priority*/, PacketReliability /*reliability*/, char /*channel*/,
              SystemAddress systemAddress, bool broadcast) {
#ifdef KAI_USE_ENET
        if (!bitStream || bitStream->GetNumberOfBytesUsed() == 0 || !host_) {
            return false;
        }

        std::vector<ENetPeer*> targets;
        {
            std::lock_guard<std::mutex> lock(connectionMutex_);
            if (broadcast) {
                for (const auto& conn : connections_) {
                    if (conn.enetPeer) {
                        targets.push_back(conn.enetPeer);
                    }
                }
            } else {
                for (const auto& conn : connections_) {
                    if (conn.address == systemAddress ||
                        conn.address.ToString() == systemAddress.ToString()) {
                        if (conn.enetPeer) {
                            targets.push_back(conn.enetPeer);
                        }
                        break;
                    }
                }
            }
        }

        if (targets.empty()) {
            return false;
        }

        const unsigned char* data = bitStream->GetData();
        size_t length = bitStream->GetNumberOfBytesUsed();

        std::lock_guard<std::mutex> lock(hostMutex_);
        for (auto* peer : targets) {
            ENetPacket* packet = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
            if (packet) {
                enet_peer_send(peer, 0, packet);
            }
        }
        enet_host_flush(host_);
        return true;
#else
        if (!bitStream || bitStream->GetNumberOfBytesUsed() == 0) {
            return false;
        }

        std::vector<Connection> targets;
        {
            std::lock_guard<std::mutex> lock(connectionMutex_);
            if (broadcast) {
                targets = connections_;
            } else {
                for (const auto& conn : connections_) {
                    if (conn.address == systemAddress ||
                        conn.address.ToString() == systemAddress.ToString()) {
                        targets.push_back(conn);
                        break;
                    }
                }
            }
        }

        if (targets.empty()) {
            return false;
        }

        const unsigned char* data = bitStream->GetData();
        size_t length = bitStream->GetNumberOfBytesUsed();

        for (const auto& target : targets) {
            auto* packet = new Packet();
            packet->systemAddress = SystemAddress(listenIp_.c_str(), listenPort_);
            packet->length = length;
            packet->data = new unsigned char[length];
            memcpy(packet->data, data, length);
            target.peer->EnqueuePacket(packet);
        }

        return true;
#endif
    }

    void StartOccasionalPing() {}

    void StopOccasionalPing() {}

    bool Ping(const char* /*host*/, unsigned short /*remotePort*/,
              bool /*onlyReplyOnAcceptingConnections*/) {
        return true;
    }

    void SetOfflinePingResponse(const char* /*data*/, const unsigned int /*length*/) {}

    ConnectionState GetConnectionState(const SystemAddress& address) {
#ifdef KAI_USE_ENET
        std::lock_guard<std::mutex> lock(connectionMutex_);
        for (const auto& conn : connections_) {
            if (conn.address == address || conn.address.ToString() == address.ToString()) {
                return IS_CONNECTED;
            }
        }
        return IS_DISCONNECTED;
#else
        std::lock_guard<std::mutex> lock(connectionMutex_);
        for (const auto& conn : connections_) {
            if (conn.address == address || conn.address.ToString() == address.ToString()) {
                return IS_CONNECTED;
            }
        }
        return IS_DISCONNECTED;
#endif
    }

    SystemAddress GetInternalID() const {
#ifdef KAI_USE_ENET
        return SystemAddress(listenIp_.c_str(), listenPort_);
#else
        return SystemAddress(listenIp_.c_str(), listenPort_);
#endif
    }

    int GetAveragePing(const SystemAddress& /*address*/) { return 50; }
    int GetLastPing(const SystemAddress& /*address*/) { return 50; }
    int GetLowestPing(const SystemAddress& /*address*/) { return 45; }

   private:
    RakPeerInterface()
        : started_(false), maxConnections_(0), listenPort_(0), listenIp_("127.0.0.1") {
        guid_.g = 0;
#ifdef KAI_USE_ENET
        host_ = nullptr;
#endif
    }

    struct Connection {
        SystemAddress address;
        RakPeerInterface* peer;
#ifdef KAI_USE_ENET
        ENetPeer* enetPeer;
#endif
    };

    static unsigned int NextGuid() {
        static std::atomic<unsigned int> counter{1};
        return counter++;
    }

    void EnqueuePacket(Packet* packet) {
        std::lock_guard<std::mutex> lock(queueMutex_);
        incomingPackets_.push(packet);
    }

    void QueueSystemMessage(const SystemAddress& from, unsigned char msgId) {
        auto* packet = new Packet();
        packet->systemAddress = from;
        packet->length = 1;
        packet->data = new unsigned char[1];
        packet->data[0] = msgId;
        EnqueuePacket(packet);
    }

    void AddConnection(const SystemAddress& address, RakPeerInterface* peer) {
        std::lock_guard<std::mutex> lock(connectionMutex_);
        auto it = std::find_if(connections_.begin(), connections_.end(),
                               [peer](const Connection& conn) { return conn.peer == peer; });
        if (it == connections_.end()) {
            connections_.push_back({address, peer
#ifdef KAI_USE_ENET
                                    , nullptr
#endif
                                   });
        }
    }

#ifdef KAI_USE_ENET
    void AddConnection(const SystemAddress& address, ENetPeer* peer) {
        std::lock_guard<std::mutex> lock(connectionMutex_);
        auto it = std::find_if(connections_.begin(), connections_.end(),
                               [peer](const Connection& conn) { return conn.enetPeer == peer; });
        if (it == connections_.end()) {
            connections_.push_back({address, nullptr, peer});
        }
    }

    void RemoveConnectionWithPeer(ENetPeer* peer) {
        std::lock_guard<std::mutex> lock(connectionMutex_);
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                           [peer](const Connection& conn) { return conn.enetPeer == peer; }),
            connections_.end());
    }
#endif

    void RemoveConnectionWithPeer(RakPeerInterface* peer) {
        std::lock_guard<std::mutex> lock(connectionMutex_);
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(),
                           [peer](const Connection& conn) { return conn.peer == peer; }),
            connections_.end());
    }

    bool started_;
    unsigned int maxConnections_;
    unsigned short listenPort_;
    std::string listenIp_;
    RakNetGUID guid_;

    mutable std::mutex queueMutex_;
    std::queue<Packet*> incomingPackets_;

    mutable std::mutex connectionMutex_;
    std::vector<Connection> connections_;
#ifdef KAI_USE_ENET
    std::unordered_set<ENetPeer*> pendingOutgoing_;
    mutable std::mutex hostMutex_;
    ENetHost* host_;
#endif

    static std::mutex globalMutex_;
    static std::unordered_map<unsigned short, RakPeerInterface*> peersByPort_;

#ifdef KAI_USE_ENET
    static std::mutex enetInitMutex_;
    static std::atomic<int> enetRefCount_;

    static bool EnsureEnetInitialized() {
        std::lock_guard<std::mutex> lock(enetInitMutex_);
        if (enetRefCount_ == 0) {
            if (enet_initialize() != 0) {
                return false;
            }
        }
        ++enetRefCount_;
        return true;
    }

    static void ReleaseEnet() {
        std::lock_guard<std::mutex> lock(enetInitMutex_);
        if (enetRefCount_ > 0) {
            --enetRefCount_;
            if (enetRefCount_ == 0) {
                enet_deinitialize();
            }
        }
    }

    static SystemAddress EnetToSystemAddress(const ENetAddress& address) {
        char ip[64] = {0};
        enet_address_get_host_ip(&address, ip, sizeof(ip));
        return SystemAddress(ip, address.port);
    }
#endif
};

inline std::mutex RakPeerInterface::globalMutex_;
inline std::unordered_map<unsigned short, RakPeerInterface*> RakPeerInterface::peersByPort_;
#ifdef KAI_USE_ENET
inline std::mutex RakPeerInterface::enetInitMutex_;
inline std::atomic<int> RakPeerInterface::enetRefCount_{0};
#endif

}  // namespace RakNet
