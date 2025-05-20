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
        try {
            if (!object.Exists()) {
                // Write a size of 0 for null objects
                unsigned int size = 0;
                bitStream.Write(size);
                return;
            }

            // Create a BinaryStream with the object's registry
            Registry *reg = object.GetRegistry();
            if (!reg) {
                // Handle null registry
                unsigned int size = 0;
                bitStream.Write(size);
                return;
            }
            BinaryStream stream(*reg);

            // Write the object type number for type checking on deserialization
            Type::Number typeNumber = object.GetTypeNumber();
            stream.Write(sizeof(typeNumber),
                         reinterpret_cast<const char *>(&typeNumber));

            // Serialize the object to the BinaryStream
            stream << object;

            // Write the size of the serialized data
            unsigned int size = stream.Size();
            bitStream.Write(size);

            // Write the serialized data
            if (size > 0) {
                bitStream.Write((const char *)stream.Begin(), size);
            }
        } catch (const Exception::Base &e) {
            // Handle KAI serialization errors
            unsigned int size = 0;
            bitStream.Write(size);

            // Optionally, log the error
            KAI_TRACE_ERROR() << "Error serializing object: " << e.ToString();
        } catch (const std::exception &e) {
            // Handle standard exceptions
            unsigned int size = 0;
            bitStream.Write(size);

            // Optionally, log the error
            KAI_TRACE_ERROR() << "Error serializing object: " << e.what();
        } catch (...) {
            // Handle unknown serialization errors
            unsigned int size = 0;
            bitStream.Write(size);

            // Optionally, log the error
            KAI_TRACE_ERROR() << "Unknown error serializing object";
        }
    }

    // Deserialize an object from a BitStream using KAI's serialization system
    static Object DeserializeObject(RakNet::BitStream &bitStream,
                                    Registry &registry) {
        // Simplified implementation for testing
        // Returns an empty object for now
        return Object();
    }

    // Network message types
    enum MessageTypes {
        ID_KAI_OBJECT_MESSAGE = RakNet::ID_USER_PACKET_ENUM + 1,
        ID_KAI_FUNCTION_CALL,
        ID_KAI_EVENT_NOTIFICATION
    };
};

KAI_NET_END