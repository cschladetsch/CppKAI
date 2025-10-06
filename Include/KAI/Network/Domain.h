#pragma once

#include <KAI/Network/Agent.h>
#include <KAI/Network/Node.h>
#include <KAI/Network/Proxy.h>

#include <memory>

KAI_NET_BEGIN

// A Network Domain is shared amoung a set of Nodes. Object Handles are shared
// accross all nodes within a common network domain.
struct Domain {
    Domain(Node &);

    // Agents are end-points to network calls. They may reside
    // on this node, or another
    template <class T>
    Agent<T> MakeAgent(std::shared_ptr<T> servant = std::make_shared<T>());

    // proxies are local representatives to remote agents.
    template <class T>
    Proxy<T> MakeProxy(NetHandle handle);

   private:
    Node &node_;
};

inline Domain::Domain(Node &node) : node_(node) {}

template <class T>
Agent<T> Domain::MakeAgent(std::shared_ptr<T> servant) {
    Agent<T> agent(node_, std::move(servant));
    return agent;
}

template <class T>
Proxy<T> Domain::MakeProxy(NetHandle handle) {
    return Proxy<T>(node_, handle);
}

KAI_NET_END
