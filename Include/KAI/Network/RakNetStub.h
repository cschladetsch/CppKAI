#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <cstring>

namespace RakNet {
    // Basic RakNet types that we need for our implementation
    typedef uint64_t RakNetGUID;
    typedef uint32_t SystemIndex;
    typedef uint32_t TimeMS;
    typedef unsigned char MessageID;
    
    // Simple address representation
    struct SystemAddress {
        std::string ip;
        unsigned short port;
        
        SystemAddress() : port(0) {}
        
        SystemAddress(const char* ipAddress, unsigned short portNumber) 
            : ip(ipAddress), port(portNumber) {}
            
        bool operator==(const SystemAddress& rhs) const {
            return ip == rhs.ip && port == rhs.port;
        }
        
        std::string ToString() const {
            return ip + ":" + std::to_string(port);
        }
        
        void FromString(const char* str, unsigned short defaultPort = 0) {
            std::string s(str);
            size_t pos = s.find(":");
            if (pos != std::string::npos) {
                ip = s.substr(0, pos);
                port = (unsigned short)std::stoi(s.substr(pos + 1));
            } else {
                ip = s;
                port = defaultPort;
            }
        }
    };
    
    // Connection state
    enum ConnectionState {
        IS_NOT_CONNECTED,
        IS_CONNECTING,
        IS_CONNECTED,
        IS_DISCONNECTING
    };
    
    // Basic packet structure
    struct Packet {
        SystemAddress systemAddress;
        unsigned char* data;
        size_t length;
        bool deleteData;
        
        Packet() : data(nullptr), length(0), deleteData(true) {}
        ~Packet() {
            if (deleteData && data != nullptr) {
                delete[] data;
            }
        }
    };
    
    // Priority levels for sending packets
    enum PacketPriority {
        IMMEDIATE_PRIORITY,
        HIGH_PRIORITY,
        MEDIUM_PRIORITY,
        LOW_PRIORITY,
        NUMBER_OF_PRIORITIES
    };

    // Reliability types for sending packets
    enum PacketReliability {
        UNRELIABLE,
        UNRELIABLE_SEQUENCED,
        RELIABLE,
        RELIABLE_ORDERED,
        RELIABLE_SEQUENCED,
        UNRELIABLE_WITH_ACK_RECEIPT,
        RELIABLE_WITH_ACK_RECEIPT,
        RELIABLE_ORDERED_WITH_ACK_RECEIPT,
        NUMBER_OF_RELIABILITIES
    };
    
    // Startup result
    enum StartupResult {
        RAKNET_STARTED,
        RAKNET_ALREADY_STARTED,
        INVALID_SOCKET_DESCRIPTORS,
        INVALID_MAX_CONNECTIONS,
        SOCKET_FAMILY_NOT_SUPPORTED,
        SOCKET_PORT_ALREADY_IN_USE,
        SOCKET_FAILED_TO_BIND,
        SOCKET_FAILED_TEST_SEND,
        PORT_CANNOT_BE_ZERO,
        FAILED_TO_CREATE_NETWORK_THREAD,
        COULD_NOT_GENERATE_GUID,
        STARTUP_OTHER_FAILURE
    };
    
    // Connection attempt result
    enum ConnectionAttemptResult {
        CONNECTION_ATTEMPT_STARTED,
        INVALID_PARAMETER,
        CANNOT_RESOLVE_DOMAIN_NAME,
        ALREADY_CONNECTED_TO_ENDPOINT,
        CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS,
        SECURITY_INITIALIZATION_FAILED
    };
    
    // Socket descriptor
    struct SocketDescriptor {
        unsigned short port;
        const char* hostAddress;
        
        SocketDescriptor() : port(0), hostAddress(nullptr) {}
        SocketDescriptor(unsigned short _port, const char* _hostAddress)
            : port(_port), hostAddress(_hostAddress) {}
    };
    
    // Common message IDs
    enum DefaultMessageIDTypes {
        ID_CONNECTED_PING,  
        ID_UNCONNECTED_PING,
        ID_UNCONNECTED_PING_OPEN_CONNECTIONS,
        ID_CONNECTED_PONG,
        ID_DETECT_LOST_CONNECTIONS,
        ID_OPEN_CONNECTION_REQUEST_1,
        ID_OPEN_CONNECTION_REPLY_1,
        ID_OPEN_CONNECTION_REQUEST_2,
        ID_OPEN_CONNECTION_REPLY_2,
        ID_CONNECTION_REQUEST,
        ID_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY,
        ID_OUR_SYSTEM_REQUIRES_SECURITY,
        ID_PUBLIC_KEY_MISMATCH,
        ID_OUT_OF_BAND_INTERNAL,
        ID_SND_RECEIPT_ACKED,
        ID_SND_RECEIPT_LOSS,
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
        ID_IP_RECENTLY_CONNECTED,
        ID_TIMESTAMP,
        ID_UNCONNECTED_PONG,
        ID_ADVERTISE_SYSTEM,
        ID_DOWNLOAD_PROGRESS,
        ID_REMOTE_DISCONNECTION_NOTIFICATION,
        ID_REMOTE_CONNECTION_LOST,
        ID_REMOTE_NEW_INCOMING_CONNECTION,
        ID_FILE_LIST_TRANSFER_HEADER,
        ID_FILE_LIST_TRANSFER_FILE,
        ID_FILE_LIST_REFERENCE_PUSH_ACK,
        ID_DDT_DOWNLOAD_REQUEST,
        ID_TRANSPORT_STRING,
        ID_REPLICA_MANAGER_CONSTRUCTION,
        ID_REPLICA_MANAGER_SCOPE_CHANGE,
        ID_REPLICA_MANAGER_SERIALIZE,
        ID_REPLICA_MANAGER_DOWNLOAD_STARTED,
        ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE,
        ID_RAKVOICE_OPEN_CHANNEL_REQUEST,
        ID_RAKVOICE_OPEN_CHANNEL_REPLY,
        ID_RAKVOICE_CLOSE_CHANNEL,
        ID_RAKVOICE_DATA,
        ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE,
        ID_AUTOPATCHER_CREATION_LIST,
        ID_AUTOPATCHER_DELETION_LIST,
        ID_AUTOPATCHER_GET_PATCH,
        ID_AUTOPATCHER_PATCH_LIST,
        ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR,
        ID_AUTOPATCHER_CANNOT_DOWNLOAD_ORIGINAL_UNMODIFIED_FILES,
        ID_AUTOPATCHER_FINISHED_INTERNAL,
        ID_AUTOPATCHER_FINISHED,
        ID_AUTOPATCHER_RESTART_APPLICATION,
        ID_NAT_PUNCHTHROUGH_REQUEST,
        ID_NAT_PUNCHTHROUGH_REPLY,
        ID_NAT_CONNECT_AT_TIME,
        ID_NAT_GET_MOST_RECENT_PORT,
        ID_NAT_CLIENT_READY,
        ID_NAT_TARGET_NOT_CONNECTED,
        ID_NAT_TARGET_UNRESPONSIVE,
        ID_NAT_CONNECTION_TO_TARGET_LOST,
        ID_NAT_ALREADY_IN_PROGRESS,
        ID_NAT_PUNCHTHROUGH_FAILED,
        ID_NAT_PUNCHTHROUGH_SUCCEEDED,
        ID_READY_EVENT_SET,
        ID_READY_EVENT_UNSET,
        ID_READY_EVENT_ALL_SET,
        ID_READY_EVENT_QUERY,
        ID_LOBBY_GENERAL,
        ID_RPC_REMOTE_ERROR,
        ID_RPC_PLUGIN,
        ID_FILE_LIST_REFERENCE_PUSH,
        ID_READY_EVENT_FORCE_ALL_SET,
        ID_ROOMS_EXECUTE_FUNC,
        ID_ROOMS_LOGON_STATUS,
        ID_ROOMS_HANDLE_CHANGE,
        ID_LOBBY2_SEND_MESSAGE,
        ID_LOBBY2_SERVER_ERROR,
        ID_FCM2_NEW_HOST,
        ID_FCM2_REQUEST_FCMGUID,
        ID_FCM2_RESPOND_CONNECTION_COUNT,
        ID_FCM2_INFORM_FCMGUID,
        ID_FCM2_UPDATE_MIN_TOTAL_CONNECTION_COUNT,
        ID_FCM2_VERIFIED_JOIN_START,
        ID_FCM2_VERIFIED_JOIN_CAPABLE,
        ID_FCM2_VERIFIED_JOIN_FAILED,
        ID_FCM2_VERIFIED_JOIN_ACCEPTED,
        ID_FCM2_VERIFIED_JOIN_REJECTED,
        ID_UDP_PROXY_GENERAL,
        ID_SQLite3_EXEC,
        ID_SQLite3_UNKNOWN_DB,
        ID_SQLLITE_LOGGER,
        ID_NAT_TYPE_DETECTION_REQUEST,
        ID_NAT_TYPE_DETECTION_RESULT,
        ID_ROUTER_2_INTERNAL,
        ID_ROUTER_2_FORWARDING_NO_PATH,
        ID_ROUTER_2_FORWARDING_ESTABLISHED,
        ID_ROUTER_2_REROUTED,
        ID_TEAM_BALANCER_INTERNAL,
        ID_TEAM_BALANCER_REQUESTED_TEAM_FULL,
        ID_TEAM_BALANCER_REQUESTED_TEAM_LOCKED,
        ID_TEAM_BALANCER_TEAM_REQUESTED_CANCELLED,
        ID_TEAM_BALANCER_TEAM_ASSIGNED,
        ID_LIGHTSPEED_INTEGRATION,
        ID_XBOX_LOBBY,
        ID_TWO_WAY_AUTHENTICATION_INCOMING_CHALLENGE_SUCCESS,
        ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_SUCCESS,
        ID_TWO_WAY_AUTHENTICATION_INCOMING_CHALLENGE_FAILURE,
        ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_FAILURE,
        ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_TIMEOUT,
        ID_TWO_WAY_AUTHENTICATION_NEGOTIATION,
        ID_CLOUD_POST_REQUEST,
        ID_CLOUD_RELEASE_REQUEST,
        ID_CLOUD_GET_REQUEST,
        ID_CLOUD_GET_RESPONSE,
        ID_CLOUD_UNSUBSCRIBE_REQUEST,
        ID_CLOUD_SERVER_TO_SERVER_COMMAND,
        ID_CLOUD_SUBSCRIPTION_NOTIFICATION,
        ID_LIB_VOICE,
        ID_RELAY_PLUGIN,
        ID_NAT_REQUEST_BOUND_ADDRESSES,
        ID_NAT_RESPOND_BOUND_ADDRESSES,
        ID_FCM2_UPDATE_USER_CONTEXT,
        ID_RESERVED_3,
        ID_RESERVED_4,
        ID_RESERVED_5,
        ID_RESERVED_6,
        ID_RESERVED_7,
        ID_RESERVED_8,
        ID_RESERVED_9,
        ID_USER_PACKET_ENUM
    };

    // Special system address for broadcast
    const SystemAddress UNASSIGNED_SYSTEM_ADDRESS("255.255.255.255", 0);
    
    // BitStream class for serialization
    class BitStream {
    public:
        BitStream() {}
        BitStream(unsigned char* data, size_t length, bool copyData = false) 
            : _data(nullptr), _length(0), _readOffset(0), _writeOffset(0)
        {
            if (data && length > 0) {
                _data = new unsigned char[length];
                _length = length;
                memcpy(_data, data, length);
                
                if (!copyData) {
                    _ownData = false;
                }
            }
        }
        
        ~BitStream() {
            if (_ownData && _data) {
                delete[] _data;
                _data = nullptr;
            }
        }
        
        // Basic methods for reading data
        void IgnoreBytes(size_t numberOfBytes) {
            _readOffset = numberOfBytes;
            if (_readOffset > _length) {
                _readOffset = _length;
            }
        }
        
        template<typename T>
        bool Read(T& value) {
            if (_readOffset + sizeof(T) <= _length) {
                memcpy(&value, _data + _readOffset, sizeof(T));
                _readOffset += sizeof(T);
                return true;
            }
            return false;
        }
        
        bool Read(char* output, size_t size) {
            if (_readOffset + size <= _length) {
                memcpy(output, _data + _readOffset, size);
                _readOffset += size;
                return true;
            }
            return false;
        }
        
        // Methods for writing data
        bool Write(unsigned int value) {
            EnsureCapacity(_writeOffset + sizeof(unsigned int));
            memcpy(_data + _writeOffset, &value, sizeof(unsigned int));
            _writeOffset += sizeof(unsigned int);
            return true;
        }
        
        bool Write(const char* input, size_t size) {
            EnsureCapacity(_writeOffset + size);
            memcpy(_data + _writeOffset, input, size);
            _writeOffset += size;
            return true;
        }
        
        // Get raw data
        unsigned char* GetData() const { 
            return _data; 
        }
        
        size_t GetNumberOfBytesUsed() const { 
            return _writeOffset; 
        }
        
        // Debug info
        void PrintBits() const {
            // Simple debug output
            std::cout << "BitStream: " << _length << " bytes total, " 
                      << _writeOffset << " bytes used, " 
                      << _readOffset << " bytes read" << std::endl;
        }
        
    private:
        void EnsureCapacity(size_t requiredSize) {
            if (requiredSize > _length) {
                size_t newSize = _length * 2;
                if (newSize < requiredSize) {
                    newSize = requiredSize;
                }
                
                unsigned char* newData = new unsigned char[newSize];
                if (_data) {
                    memcpy(newData, _data, _length);
                    if (_ownData) {
                        delete[] _data;
                    }
                }
                
                _data = newData;
                _length = newSize;
                _ownData = true;
            }
        }
        
        unsigned char* _data = nullptr;
        size_t _length = 0;
        size_t _readOffset = 0;
        size_t _writeOffset = 0;
        bool _ownData = true;
    };
    
    // RakPeer interface class
    class RakPeerInterface {
    public:
        // Get singleton instance
        static RakPeerInterface* GetInstance() { return new RakPeerInterface(); }
        
        // Destroy instance
        static void DestroyInstance(RakPeerInterface* instance) { delete instance; }
        
        // Connect to a remote system
        virtual ConnectionAttemptResult Connect(const char* host, unsigned short remotePort, 
                                              const char* passwordData, int passwordDataLength) {
            return CONNECTION_ATTEMPT_STARTED;
        }
        
        // Startup the networking system
        virtual StartupResult Startup(unsigned int maxConnections, SocketDescriptor* socketDescriptors,
                                     unsigned short socketDescriptorCount) {
            return RAKNET_STARTED;
        }
        
        // Shutdown the networking system
        virtual void Shutdown(unsigned int blockDuration = 0) {}
        
        // Set maximum incoming connections
        virtual void SetMaximumIncomingConnections(unsigned short numberAllowed) {}
        
        // Get ping of a remote system
        virtual int GetAveragePing(const SystemAddress& systemAddress) {
            return 0;
        }
        
        // Send a ping request
        virtual void Ping(const char* host, unsigned short remotePort, bool onlyReplyOnAcceptingConnections) {}
        
        // Set offline ping response
        virtual void SetOfflinePingResponse(const char* data, const unsigned int length) {}
        
        // Receive a packet
        virtual Packet* Receive() {
            return nullptr;
        }
        
        // Deallocate a packet
        virtual void DeallocatePacket(Packet* packet) {
            delete packet;
        }
        
        // Get connection state
        virtual ConnectionState GetConnectionState(const SystemAddress& systemAddress) {
            return IS_NOT_CONNECTED;
        }
        
        // Send a packet to a specific address or broadcast
        virtual bool Send(const char* data, unsigned int length, PacketPriority priority, 
                         PacketReliability reliability, char orderingChannel, 
                         const SystemAddress& systemAddress, bool broadcast) {
            // Stub implementation
            return true;
        }
    };
    
    // Helper function
    inline TimeMS GetTimeMS() {
        return static_cast<TimeMS>(time(nullptr) * 1000);
    }
}