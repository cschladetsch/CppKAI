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
        : node_(&node), netHandle_(handle) {}

    virtual ~Representative() = default;

    Node &GetNode() const { return *node_; }
    NetHandle GetHandle() const { return netHandle_; }

    void SetHandle(NetHandle handle) { netHandle_ = handle; }

   private:
    Node *node_;
    NetHandle netHandle_;
};

KAI_NET_END
