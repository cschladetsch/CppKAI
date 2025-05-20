#pragma once

// This file provides a unified interface to either the real RakNet
// implementation or the stub based on the KAI_USE_RAKNET_STUB define

#ifdef KAI_USE_RAKNET_STUB
#include "KAI/Network/RakNetStub.h"
#else
// Use the real RakNet implementation
// The stub already has everything we need, so just include it for now
#include "KAI/Network/RakNetStub.h"

// Define a namespace with our custom functionality on top of RakNet
namespace KAI {
namespace Network {
namespace RakNetAdapter {

// Helper functions for packet handling
inline bool Initialize() {
    // Any additional initialization needed
    return true;
}

inline void Shutdown() {
    // Any additional cleanup needed
}

// This function creates a real packet that we can use with our custom
// NetworkPeer implementation
inline RakNet::Packet* CreatePacket(RakNet::MessageID msgId,
                                    const RakNet::SystemAddress& addr) {
    RakNet::Packet* packet = new RakNet::Packet();
    packet->systemAddress = addr;
    packet->length = sizeof(RakNet::MessageID);
    packet->data = new unsigned char[packet->length];
    packet->data[0] = msgId;
    return packet;
}

}  // namespace RakNetAdapter
}  // namespace Network
}  // namespace KAI

#endif  // KAI_USE_RAKNET_STUB