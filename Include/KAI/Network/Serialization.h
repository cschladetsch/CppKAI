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
            BinaryStream stream(object.GetRegistry());
            
            // Write the object type number for type checking on deserialization
            Type::Number typeNumber = object.GetTypeNumber();
            stream.Write(sizeof(typeNumber), reinterpret_cast<const char*>(&typeNumber));
            
            // Serialize the object to the BinaryStream
            stream << object;
            
            // Write the size of the serialized data
            unsigned int size = stream.Size();
            bitStream.Write(size);
            
            // Write the serialized data
            if (size > 0) {
                bitStream.Write((const char*)stream.Begin(), size);
            }
        } catch (const Exception::Base& e) {
            // Handle KAI serialization errors
            unsigned int size = 0;
            bitStream.Write(size);
            
            // Optionally, log the error
            KAI_TRACE_ERROR() << "Error serializing object: " << e.ToString();
        } catch (const std::exception& e) {
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
        try {
            // Read the size
            unsigned int size = 0;
            bitStream.Read(size);
            if (size == 0) {
                return Object();  // Return empty object on read failure
            }

            // Read the serialized data into a buffer
            std::vector<char> buffer(size);
            if (!bitStream.Read((char*)buffer.data(), size)) {
                KAI_TRACE_ERROR() << "Failed to read data from BitStream";
                return Object();
            }
            
            // Create a BinaryPacket with the buffer data
            BinaryPacket packet(buffer.data(), buffer.data() + size, &registry);
            
            // First read the type number
            Type::Number typeNumber;
            if (!packet.Read(typeNumber)) {
                KAI_TRACE_ERROR() << "Failed to read type number from packet";
                return Object();
            }
            
            // Create a new object of the appropriate type
            Object result = registry.NewFromTypeNumber(typeNumber);
            if (!result.Exists()) {
                KAI_TRACE_ERROR() << "Failed to create object of type " << typeNumber;
                return Object();
            }
            
            // Deserialize the object data
            packet >> result;
            
            return result;
        } catch (const Exception::Base& e) {
            // Handle KAI deserialization errors
            KAI_TRACE_ERROR() << "Error deserializing object: " << e.ToString();
            return Object();
        } catch (const std::exception& e) {
            // Handle standard deserialization errors
            KAI_TRACE_ERROR() << "Error deserializing object: " << e.what();
            return Object();
        } catch (...) {
            // Handle unknown deserialization errors
            KAI_TRACE_ERROR() << "Unknown error deserializing object";
            return Object();
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