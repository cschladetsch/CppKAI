#pragma once

#include "KAI/Network/Config.h"
#include "KAI/Network/Future.h"
#include "KAI/Network/ProxyBase.h"

// Forward declare Node to avoid circular dependency
KAI_NET_BEGIN
struct Node;
KAI_NET_END

KAI_NET_BEGIN

template <class T>
struct Proxy : ProxyBase {
    Proxy(Node &node, NetHandle remote) : ProxyBase(node, remote) {}

   private:
};

KAI_NET_END
