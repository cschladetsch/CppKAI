#pragma once

#include "KAI/Network/Config.h"
#include "KAI/Network/Future.h"
#include "KAI/Network/ProxyBase.h"

#include <string>

// Forward declare Node to avoid circular dependency
KAI_NET_BEGIN
struct Node;
KAI_NET_END

KAI_NET_BEGIN

template <class T>
struct Proxy : ProxyBase {
    Proxy(Node &node, NetHandle remote) : ProxyBase(node, remote) {}

    template <typename R, typename... Args>
    Future<R> Call(const std::string &name, Args &&...args) {
        return this->Exec<R>(name.c_str(), std::forward<Args>(args)...);
    }

    template <typename P>
    Future<P> Get(const std::string &name) {
        return this->Fetch<P>(name.c_str());
    }

    template <typename P>
    Future<void> Set(const std::string &name, P &&value) {
        return this->Store(name.c_str(), std::forward<P>(value));
    }
};

KAI_NET_END
