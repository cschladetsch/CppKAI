#pragma once

#include <cstring>
#include <iostream>
#include <memory>
#include <string>
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
    BitStream(unsigned char* data_, size_t length, bool copyData) : readPos(0) {
        if (copyData && data_ != nullptr && length > 0) {
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
    static RakPeerInterface* GetInstance() {
        static RakPeerInterface instance;
        return &instance;
    }

    static void DestroyInstance(RakPeerInterface* instance) {
        // Do nothing in stub implementation
    }

    StartupResult Startup(unsigned int maxConnections,
                          SocketDescriptor* socketDescriptors,
                          unsigned short socketDescriptorCount) {
        return RAKNET_STARTED;
    }

    void Shutdown(unsigned int blockDuration) {
        // Do nothing in stub
    }

    void SetMaximumIncomingConnections(unsigned short numberAllowed) {
        // Do nothing in stub
    }

    ConnectionAttemptResult Connect(const char* host, unsigned short remotePort,
                                    const char* passwordData,
                                    int passwordDataLength) {
        return CONNECTION_ATTEMPT_STARTED;
    }

    Packet* Receive() {
        return nullptr;  // Stub always returns no packets
    }

    void DeallocatePacket(Packet* packet) { delete packet; }

    bool Send(const char* data, int length, PacketPriority priority, PacketReliability reliability,
              char channel, SystemAddress systemAddress, bool broadcast) {
        return true;  // Stub always succeeds
    }

    bool Send(BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char channel,
              SystemAddress systemAddress, bool broadcast) {
        return true;  // Stub always succeeds
    }

    void StartOccasionalPing() {
        // Do nothing in stub
    }

    void StopOccasionalPing() {
        // Do nothing in stub
    }

    // Additional functions needed by the network code
    bool Ping(const char* host, unsigned short remotePort,
              bool onlyReplyOnAcceptingConnections) {
        return true;  // Stub always succeeds
    }

    void SetOfflinePingResponse(const char* data, const unsigned int length) {
        // Do nothing in stub
    }

    // Connection state management
    ConnectionState GetConnectionState(const SystemAddress& address) {
        return IS_CONNECTED;  // Stub always returns connected
    }

    // Get internal ID (address of this peer)
    SystemAddress GetInternalID() const {
        return SystemAddress("127.0.0.1", 0);  // Stub returns localhost
    }

    // Ping functionality
    int GetAveragePing(const SystemAddress& address) {
        return 100;  // Stub returns a reasonable ping time
    }

    int GetLastPing(const SystemAddress& address) {
        return 100;  // Stub returns a reasonable ping time
    }

    int GetLowestPing(const SystemAddress& address) {
        return 90;  // Stub returns a reasonable ping time
    }
};

}  // namespace RakNet