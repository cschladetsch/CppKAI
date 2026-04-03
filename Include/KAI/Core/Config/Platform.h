#pragma once

#include <boost/preprocessor/cat.hpp>

#include "boost/preprocessor/stringize.hpp"

//#define KAI_PLATFORM_SOURCE_DIR(X) \
//    BOOST_PP_CAT("Platform/", KAI_PP_STRINGISE(X))
//#define KAI_PLATFORM_SOURCE(S) \
//    BOOST_PP_STRINGIZE(BOOST_PP_CAT(KAI_PLATFORM_SOURCE_DIR(BOOST_PP_CAT(KAI_PLATFORM, /)), S))
//#define KAI_PLATFORM_INCLUDE_DIR(X) \
//    BOOST_PP_CAT("KAI/Platform/", #X)
//#define KAI_PLATFORM_INCLUDE(S) \
//    BOOST_PP_STRINGIZE(BOOST_PP_CAT(KAI_PLATFORM_INCLUDE_DIR(BOOST_PP_CAT(KAI_PLATFORM, /)), S))

#define KAI_PLATFORM_HEADER KAI_PLATFORM_INCLUDE

#define KAI_PLATFORM_BEGIN \
    KAI_BEGIN              \
    namespace Platform {   \
    namespace KAI_PLATFORM {

#define KAI_PLATFORM_END \
    }                    \
    }                    \
    KAI_END

#define KAI_PLATFORM_IMPL_NAME(X) \
    BOOST_PP_CAT(KAI_NAMESPACED(Platform::KAI_PLATFORM::), X)

#define KAI_PLATFORM_IMPL_DECL(X) \
    KAI_PLATFORM_BEGIN            \
    struct X;                     \
    KAI_PLATFORM_END

#define KAI_PLATFORM_IMPL_DECL_CLASS(X) \
    KAI_PLATFORM_BEGIN                  \
    class X;                            \
    KAI_PLATFORM_END

// EOF
