#pragma once

#include <chrono>

#include "KAI/Network/Config.h"

KAI_NET_BEGIN

struct DateTime {
    using TimePoint = std::chrono::system_clock::time_point;

    static TimePoint Now() { return std::chrono::system_clock::now(); }
};

KAI_NET_END
