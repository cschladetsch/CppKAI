#pragma once

#include <KAI/Network/Future.h>
#include <KAI/Network/Representative.h>

KAI_NET_BEGIN

struct ProxyBase : Representative {
   protected:
    typedef StringStream StreamType;

    ProxyBase(Node &node, NetHandle handle) : Representative(node, handle) {}

    template <class Ty>
    Future<Ty> Exec(const char *name, StreamType &args) {
        return Future<Ty>();
    }

    template <class Ty>
    Future<Ty> Fetch(const char *name) {
        return Future<Ty>();
    }

    void Store(const char *name, const Object &value) {
        // Store a property value on the remote object
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
