#pragma once

#include <KAI/Network/Future.h>
#include <KAI/Network/Representative.h>
#include <KAI/Network/Node.h>

KAI_NET_BEGIN

struct ProxyBase : Representative {
   protected:
    typedef StringStream StreamType;

    ProxyBase(Node &node, NetHandle handle) : Representative(node, handle) {}

    template <class Ty, class... Args>
    Future<Ty> Exec(const char *name, Args &&...args) {
        return GetNode().Invoke<Ty>(GetHandle(), name,
                                    std::forward<Args>(args)...);
    }

    template <class Ty>
    Future<Ty> Fetch(const char *name) {
        return GetNode().FetchProperty<Ty>(GetHandle(), name);
    }

    template <class Ty>
    Future<void> Store(const char *name, Ty &&value) {
        return GetNode().StoreProperty(GetHandle(), name,
                                       std::forward<Ty>(value));
    }

    template <typename HandlerType>
    void RegisterEventHandler(const char *name, HandlerType handler) {
        // Register an event handler for the given event
    }

    void UnregisterEventHandler(const char *name) {
        // Unregister an event handler
    }
};

KAI_NET_END
