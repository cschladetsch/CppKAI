#pragma once

#include "KAI/Network/Config.h"
#include "KAI/Network/Future.h"

// This is a minimal header for proxy generation
// to avoid circular dependencies with RakNet

KAI_NET_BEGIN

template <class T>
class IFuture {
   public:
    // Basic interface for generated proxies
    typedef T Type;
};

KAI_NET_END
