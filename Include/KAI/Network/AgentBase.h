#pragma once

#include "KAI/Network/Representative.h"

KAI_NET_BEGIN

struct AgentBase : Representative {
    explicit AgentBase(Node &node);
    virtual ~AgentBase();
};

KAI_NET_END
