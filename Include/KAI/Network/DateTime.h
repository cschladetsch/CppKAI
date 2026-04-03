#pragma once

#include "KAI/Network/Config.h"
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/date_time/time_zone_base.hpp"

KAI_NET_BEGIN

struct DateTime {
    static boost::local_time::local_date_time Now();
};

KAI_NET_END
