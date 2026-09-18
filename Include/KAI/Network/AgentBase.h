#pragma once

#include "KAI/Network/Representative.h"

KAI_NET_BEGIN

struct AgentBase : Representative {
    explicit AgentBase(Node &node);
    ~AgentBase() override;
};

KAI_NET_END
