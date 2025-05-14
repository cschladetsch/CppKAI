#pragma once

#include <KAI/Network/Agent.h>
#include <KAI/Network/Node.h>
#include <KAI/Network/Proxy.h>

KAI_NET_BEGIN

// A Network Domain is shared amoung a set of Nodes. Object Handles are shared
// accross all nodes within a common network domain.
struct Domain {
    Domain(Node &);

    // Agents are end-points to network calls. They may reside
    // on this node, or another
    template <class T>
    Agent<T> MakeAgent();

    // proxies are local representatives to remote agents.
    template <class T>
    Proxy<T> MakeProxy(NetHandle handle);

   private:
    Node &node_;
};

KAI_NET_END
