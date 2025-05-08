#pragma once

#include "KAI/Network/Network.h"
#include "KAI/Core/Object.h"
#include "KAI/Core/BinaryPacket.h"
#include "KAI/Core/BinaryStream.h"
#include "KAI/Network/RakNetStub.h"
#include <vector>

KAI_NET_BEGIN

// Helper class to serialize KAI objects to RakNet BitStream
class NetworkSerializer
{
public:
    // Serialize an object to a BitStream (stub implementation)
    static void SerializeObject(RakNet::BitStream &bitStream, const Object &object)
    {
        // TODO: Implement a proper serialization method when BinaryPacket is fully implemented
        
        // For now, just write a placeholder size
        unsigned int size = 4;
        bitStream.Write(size);
        
        // Write some placeholder data
        bitStream.Write("TEST", 4);
    }
    
    // Deserialize an object from a BitStream (stub implementation)
    static Object DeserializeObject(RakNet::BitStream &bitStream, Registry &registry)
    {
        // TODO: Implement a proper deserialization method
        
        // Read the size
        unsigned int size;
        bitStream.Read(size);
        
        // Read the placeholder data (and ignore it for now)
        char buffer[256];
        if (size <= sizeof(buffer)) {
            bitStream.Read(buffer, size);
        }
        
        // Return an empty object for now
        return Object();
    }
    
    // Network message types
    enum MessageTypes
    {
        ID_KAI_OBJECT_MESSAGE = RakNet::ID_USER_PACKET_ENUM + 1,
        ID_KAI_FUNCTION_CALL,
        ID_KAI_EVENT_NOTIFICATION
    };
};

KAI_NET_END