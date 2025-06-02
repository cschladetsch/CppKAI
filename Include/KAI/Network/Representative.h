#pragma once

#include "KAI/Network/Config.h"
#include "KAI/Network/FwdDeclarations.h"
#include "KAI/Network/NetHandle.h"
#include "KAI/Network/NetObject.h"

// Forward declare Node to avoid circular dependency
KAI_NET_BEGIN
struct Node;
KAI_NET_END

KAI_NET_BEGIN

NetHandle GetNetHandle(Object const &t, Node const &);

// common to either proxy or agent
struct Representative : Reflected {
   protected:
    Representative(Node &node, NetHandle handle)
        : node_(node), netHandle_(handle) {}

   protected:
    void Receive(NetHandle sender, BinaryStream &packet);
    void Receive(NetHandle sender, StringStream &packet);

    void Send(NetHandle recipient, const char *);
    void Send(NetHandle recipient, BinaryPacket const &);

   private:
    Node &node_;
    NetHandle netHandle_;
};

KAI_NET_END
