#pragma once

#include <KAI/Language/Common/TokenBase.h>

#ifndef TAU_NAMESPACE_NAME
#define TAU_NAMESPACE_NAME tau
#endif

#define TAU_BEGIN KAI_BEGIN namespace TAU_NAMESPACE_NAME {
#define TAU_END \
    }           \
    KAI_END
