#pragma once

#include "KAI/Network/Node.h"
#include "KAI/Network/RakNetStub.h"
#include "KAI/Network/Serialization.h"

KAI_NET_BEGIN

template <class T>
Future<T> Node::Send(NetHandle handle, const Object& obj) {
    if (!_isRunning || !_peer) return Future<T>();

    // Create a BitStream to hold our data
    RakNet::BitStream bitStream;

    // Write the message type
    unsigned char msgId = RakNet::ID_USER_PACKET_ENUM +
                          1;  // NetworkSerializer::ID_KAI_OBJECT_MESSAGE
    bitStream.Write(msgId);

    // Write the net handle
    unsigned int handleValue = handle.Value();
    bitStream.Write(handleValue);

    // Here we would serialize the object
    // NetworkSerializer::SerializeObject(bitStream, obj);

    // Find target address
    RakNet::SystemAddress targetAddress;
    bool found = false;

    // Find the target address based on the handle
    const auto& connections = _connectionManager->GetAllConnections();
    for (const auto& pair : connections) {
        // TODO: Match handle to address
        found = true;
        targetAddress = pair.second.address;
        break;
    }

    if (found) {
        _peer->Send((char*)bitStream.GetData(),
                    bitStream.GetNumberOfBytesUsed(), RakNet::HIGH_PRIORITY,
                    RakNet::RELIABLE_ORDERED, 0, targetAddress, false);
    }

    // Create and return a future for the response
    return Future<T>();
}

template <class T>
Future<T> Node::Receive(NetHandle handle, Object obj) {
    KAI_UNUSED_2(handle, obj);

    // This would be implemented as part of the message processing loop
    // that handles incoming packets and dispatches them to the appropriate
    // handler based on the message type

    return Future<T>();
}

// Broadcast an object to all connected peers
template <class T>
void Node::Broadcast(const Object& obj) {
    if (!_peer || !_isRunning) return;

    // Create a BitStream for our message
    RakNet::BitStream bs;

    // Write message type
    unsigned char msgId = RakNet::ID_USER_PACKET_ENUM +
                          1;  // NetworkSerializer::ID_KAI_OBJECT_MESSAGE
    bs.Write(msgId);

    // Write broadcast indicator (handle = 0)
    unsigned int handleValue = 0;
    bs.Write(handleValue);

    // Here we would serialize the object
    // NetworkSerializer::SerializeObject(bs, obj);

    // Broadcast to all connected peers
    _peer->Send((char*)bs.GetData(), bs.GetNumberOfBytesUsed(),
                RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0,
                RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

KAI_NET_END