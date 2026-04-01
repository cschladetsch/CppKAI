#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "KAI/Core/BinaryStream.h"
#include "KAI/Network/Config.h"

KAI_NET_BEGIN

enum class SendReliability { Unreliable, Reliable };
enum class SendRouting    { Unicast, Broadcast };

// System message identifiers reserved for the transport layer.
enum class SystemMessage : unsigned char {
    NewIncomingConnection = 1,
    ConnectionRequestAccepted = 2,
    ConnectionAttemptFailed = 3,
    DisconnectionNotification = 4,
    ConnectionLost = 5,
    AlreadyConnected = 6,
    NoFreeIncomingConnections = 7,
    UnconnectedPong = 8
};

// User-level message space starts here.
constexpr unsigned char kUserPacketStart = 128;

struct NetAddress {
    std::string host;
    unsigned short port = 0;

    NetAddress() = default;
    NetAddress(std::string host_, unsigned short port_)
        : host(std::move(host_)), port(port_) {}

    bool IsValid() const { return !host.empty() && port != 0; }

    std::string ToString() const { return host + ":" + std::to_string(port); }

    unsigned short GetPort() const { return port; }

    friend bool operator==(const NetAddress& a, const NetAddress& b) {
        return a.port == b.port && a.host == b.host;
    }

    friend bool operator!=(const NetAddress& a, const NetAddress& b) {
        return !(a == b);
    }
};

struct NetPacket {
    std::vector<unsigned char> data;
    NetAddress address;
};

class NetPeer {
   public:
    virtual ~NetPeer() = default;

    static std::unique_ptr<NetPeer> Create();

    virtual bool Startup(int maxConnections, const NetAddress& bindAddress) = 0;
    virtual void Shutdown(int timeoutMs) = 0;
    virtual bool Connect(const NetAddress& remoteAddress) = 0;
    virtual void SetMaximumIncomingConnections(int maxConnections) = 0;
    virtual std::unique_ptr<NetPacket> Receive() = 0;
    virtual bool Send(const unsigned char* data, std::size_t size, bool reliable,
                      int channel, const NetAddress& target,
                      bool broadcast) = 0;

    // Convenience overload: takes a BinaryStream and named enums instead of
    // raw pointer/size and boolean flags.
    bool Send(const BinaryStream& stream, SendReliability reliability,
              int channel, const NetAddress& target, SendRouting routing) {
        return Send(
            reinterpret_cast<const unsigned char *>(stream.Begin()),
            static_cast<std::size_t>(stream.Size()),
            reliability == SendReliability::Reliable,
            channel,
            target,
            routing == SendRouting::Broadcast);
    }
    virtual NetAddress GetInternalAddress() const = 0;
    virtual int GetAveragePing(const NetAddress& address) const = 0;
    virtual void Ping(const NetAddress& address) = 0;
    virtual void SetOfflinePingResponse(const unsigned char* data,
                                        std::size_t size) = 0;
    // Flush all queued outgoing packets immediately.
    virtual void Flush() = 0;
};

KAI_NET_END
