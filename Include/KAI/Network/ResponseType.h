#pragma once

#include "KAI/Network/Config.h"

KAI_NET_BEGIN

enum class ResponseType {
    None,
    Pending,
    TimedOut,
    Delivered,
    BadRequest,
    UnknownAgent,
    Returned,
};

KAI_NET_END
