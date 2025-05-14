#pragma once

#include "KAI/Network/AgentBase.h"
#include "KAI/Network/Future.h"  // Added explicit include for Future
#include "KAI/Network/NetHandle.h"
#include "NetPointer.h"

KAI_NET_BEGIN

template <class T>
struct Agent : AgentBase {
    typedef Pointer<T> Servant;

    Agent(Node &node, NetPointer<Servant> server) : AgentBase(node, server) {
        servant_ = server;
    }

    template <class R = void>
    Future<R> Respond(NetHandle handle, int ty, Object request) {
        KAI_UNUSED_3(handle, ty, request);
        return Future<R>();
    }

   private:
    Pointer<Servant> servant_;
};

KAI_NET_END
