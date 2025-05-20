#pragma once

// This is the real RakNet implementation (not a stub)
// It wraps the actual RakNet functionality

#include <BitStream.h>
#include <MessageIdentifiers.h>
#include <PacketLogger.h>
#include <RakNetTypes.h>
#include <RakPeerInterface.h>
#include <SocketDescriptor.h>

#include "KAI/Network/FwdDeclarations.h"

// Ensure we're not using the stub
#ifdef KAI_USE_RAKNET_STUB
#undef KAI_USE_RAKNET_STUB
#endif

// Define that we're using the real RakNet implementation
#define KAI_USE_REAL_RAKNET

namespace KAI {
namespace Network {

// No need to define a full namespace - we'll just use the RakNet namespace
// directly
using namespace RakNet;

// We only need to define functions that the stub doesn't have or that need
// special handling
namespace RakNetImpl {
// Additional utility functions if needed
inline bool Initialize() { return true; }

inline void Shutdown() {
    // Nothing special to do
}
}  // namespace RakNetImpl

}  // namespace Network
}  // namespace KAI