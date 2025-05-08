#pragma once

#include "KAI/Network/Network.h"

KAI_NET_BEGIN

// Connection event types
enum class ConnectionEvent {
    Connected,
    Disconnected,
    ConnectionFailed,
    ConnectionLost,
    Timeout
};

KAI_NET_END