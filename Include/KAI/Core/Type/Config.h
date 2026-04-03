#pragma once

#ifndef KAI_OPTION_USE_NAMESPACES
#define KAI_TYPE_BEGIN KAI_BEGIN namespace Type {
#define KAI_TYPE_END \
    KAI_END          \
    }
#else
#pragma error "I don't support lack of namespaces any more"
#endif

#define KAI_TYPENUMBER_TYPR int
