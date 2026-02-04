#pragma once

#include <string>
#include <vector>

#include "KAI/Core/BinaryPacket.h"
#include "KAI/Core/BinaryStream.h"
#include "KAI/Core/Object.h"
#include "KAI/Network/Network.h"
#include "KAI/Network/Transport.h"

KAI_NET_BEGIN

// Helper class to serialize KAI objects to network buffers
class NetworkSerializer {
   public:
    // Serialize an object to a BinaryStream using KAI's serialization system
    static void SerializeObject(BinaryStream &stream, const Object &object) {
        try {
            if (!object.Exists()) {
                // Write a size of 0 for null objects
                int size = 0;
                stream.Write(size);
                return;
            }

            // Create a BinaryStream with the object's registry
            Registry *reg = object.GetRegistry();
            if (!reg) {
                int size = 0;
                stream.Write(size);
                return;
            }

            BinaryStream objectStream(*reg);

            // Serialize the object to the BinaryStream (includes type info)
            objectStream << object;

            // Write the size of the serialized data
            int size = objectStream.Size();
            stream.Write(size);

            // Write the serialized data
            if (size > 0) {
                stream.Write(size, objectStream.Begin());
            }
        } catch (const Exception::Base &e) {
            // Handle KAI serialization errors
            int size = 0;
            stream.Write(size);

            // Optionally, log the error
            KAI_TRACE_ERROR() << "Error serializing object: " << e.ToString();
        } catch (const std::exception &e) {
            // Handle standard exceptions
            int size = 0;
            stream.Write(size);

            // Optionally, log the error
            KAI_TRACE_ERROR() << "Error serializing object: " << e.what();
        } catch (...) {
            // Handle unknown serialization errors
            int size = 0;
            stream.Write(size);

            // Optionally, log the error
            KAI_TRACE_ERROR() << "Unknown error serializing object";
        }
    }

    // Deserialize an object from a BinaryPacket using KAI's serialization system
    static Object DeserializeObject(BinaryPacket &packet,
                                    Registry &registry) {
        int size = 0;
        if (!packet.Read(size)) {
            return Object();
        }

        if (size <= 0) {
            return Object();
        }

        std::vector<char> buffer(static_cast<std::size_t>(size));
        if (!packet.Read(size, buffer.data())) {
            return Object();
        }

        BinaryStream stream(registry);
        stream.Write(size, buffer.data());
        stream.SetRegistry(&registry);

        Object obj;
        stream >> obj;
        return obj;
    }

    static void WriteString(BinaryStream &stream, const std::string &value) {
        int size = static_cast<int>(value.size());
        stream.Write(size);
        if (size > 0) {
            stream.Write(size, value.data());
        }
    }

    static bool ReadString(BinaryPacket &packet, std::string &value) {
        int size = 0;
        if (!packet.Read(size)) {
            return false;
        }

        if (size < 0) {
            return false;
        }

        value.clear();
        if (size == 0) {
            return true;
        }

        value.resize(static_cast<std::size_t>(size));
        return packet.Read(size, value.data());
    }

    // Network message types
    enum MessageTypes {
        ID_KAI_OBJECT_MESSAGE = kUserPacketStart,
        ID_KAI_FUNCTION_CALL = kUserPacketStart + 1,
        ID_KAI_EVENT_NOTIFICATION = kUserPacketStart + 2,
        ID_KAI_FUNCTION_RESPONSE = kUserPacketStart + 3
    };
};

KAI_NET_END
