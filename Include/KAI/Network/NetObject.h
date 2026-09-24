#pragma once

#include "KAI/Network/Config.h"
#include "KAI/Network/NetHandle.h"

KAI_NET_BEGIN

struct NetObject : Object {
    NetHandle netHandle;
    Domain* domain;

public:
    [[nodiscard]] NetHandle GetNetHandle() const;
};

KAI_NET_END
