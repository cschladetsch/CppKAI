#pragma once

#include "KAI/Core/BuiltinTypes.h"

#ifndef KAI_NET_NAMESPACE_NAME
#define KAI_NET_NAMESPACE_NAME net
#endif

#define KAI_NET_BEGIN KAI_BEGIN namespace KAI_NET_NAMESPACE_NAME {
#define KAI_NET_END \
    }               \
    }
