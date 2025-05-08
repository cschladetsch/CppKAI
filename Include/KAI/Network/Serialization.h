#pragma once

#include <vector>

#include "KAI/Core/BinaryPacket.h"
#include "KAI/Core/BinaryStream.h"
#include "KAI/Core/Object.h"
#include "KAI/Network/Network.h"
#include "KAI/Network/RakNetStub.h"

KAI_NET_BEGIN

// Helper class to serialize KAI objects to RakNet BitStream
class NetworkSerializer {
   public:
    // Serialize an object to a BitStream using KAI's serialization system
    static void SerializeObject(RakNet::BitStream &bitStream,
                                const Object &object) {
        // In a full implementation, we would:
        // 1. Create a BinaryStream with the object's registry
        // 2. Serialize the object to the BinaryStream
        // 3. Get the data from the BinaryStream and write it to the BitStream

        try {
            // Create a placeholder for serialized data
            // In a real implementation, we would use:
            // BinaryStream stream(object.GetRegistry());
            // stream << object;

            // For now, just write a placeholder size and data
            // This can be replaced with real serialization when BinaryStream is
            // fully implemented
            unsigned int size = sizeof(int);
            bitStream.Write(size);

            // Write some metadata that could identify our object type
            int objectTypeId = 1;  // Placeholder value
            bitStream.Write(objectTypeId);
        } catch (...) {
            // Handle serialization errors
            // In a production system, you might want to log this error
            // For now, write a zero-size packet to indicate an error
            unsigned int size = 0;
            bitStream.Write(size);
        }
    }

    // Deserialize an object from a BitStream using KAI's serialization system
    static Object DeserializeObject(RakNet::BitStream &bitStream,
                                    Registry &registry) {
        // In a full implementation, we would:
        // 1. Read the serialized data from the BitStream
        // 2. Create a BinaryPacket with that data
        // 3. Deserialize the object from the BinaryPacket using the registry

        try {
            // Read the size
            unsigned int size = 0;
            bitStream.Read(size);
            if (size == 0)
                return Object();  // Return empty object on read failure

            // In a real implementation, we would read the data and deserialize
            // it: std::vector<char> buffer(size);
            // bitStream.Read((char*)buffer.data(), size);
            // BinaryPacket packet(buffer.data(), buffer.data() + size,
            // &registry); Object result; packet >> result; return result;

            // For now, just read the placeholder data and return an empty
            // object
            unsigned int objectTypeId = 0;
            bitStream.Read(objectTypeId);

            // Create a new object of the appropriate type based on objectTypeId
            // For now, just return an empty object
            return Object();
        } catch (...) {
            // Handle deserialization errors
            return Object();  // Return empty object on error
        }
    }

    // Network message types
    enum MessageTypes {
        ID_KAI_OBJECT_MESSAGE = RakNet::ID_USER_PACKET_ENUM + 1,
        ID_KAI_FUNCTION_CALL,
        ID_KAI_EVENT_NOTIFICATION
    };
};

KAI_NET_END