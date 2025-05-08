#pragma once

#include "KAI/Network/Config.h"
#include "KAI/Network/DateTime.h"
#include "KAI/Network/NetHandle.h"
#include "KAI/Network/ResponseType.h"

KAI_NET_BEGIN

// Simplified Future class that doesn't have template issues
template <class T = void>
struct Future {
    // Id that is unique to the creator of the Future
    int Id = 0;

    // the response type
    ResponseType Response = ResponseType::None;

    // if true, this future has been completed one way or another
    bool Complete = false;

    // Simplified - no need for DateTime which has dependencies
    // DateTime When;

    // Simplified - no need for Pointer which causes circular dependency
    // Pointer<T> Result;
};

KAI_NET_END
