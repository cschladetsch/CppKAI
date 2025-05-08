#pragma once

#include <KAI/Core/Config/Options.h>
#include <KAI/Core/Config/Platform.h>

#ifndef KAI_NAMESPACE_NAME
#define KAI_NAMESPACE_NAME kai
#endif

#define KAI_STRINGISE(A) KAI_STRINGISE2(A)
#define KAI_STRINGISE2(A) #A

#define KAI_STRINGISE_WIDE(A) KAI_STRINGISE_WIDE2(A)
#define KAI_STRINGISE_WIDE2(A) L## #A

#ifdef KAI_OPTION_USE_NAMESPACES
#define KAI_BEGIN namespace KAI_NAMESPACE_NAME {
#define KAI_END }
#define KAI_TYPE_BEGIN KAI_BEGIN namespace Type {
#define KAI_TYPE_END \
    }                \
    }
#define USING_NAMESPACE_KAI using namespace KAI_NAMESPACE_NAME;
#define KAI_NAMESPACE(E) ::KAI_NAMESPACE_NAME::E
#else
#undef KAI_NAMESPACE_NAME
#define KAI_NAMESPACE(E) ::E
#define KAI_BEGIN
#define KAI_END
#define USING_NAMESPACE_KAI
#pragma error "nah use namespaces"
#endif

// use gc-count based caching system for object validity and value lookups
#define KAI_CACHE_OBJECT_LOOKUPS

// These no longer work :(
//
// #    define KAI_PLATFORM_INCLUDE(Header) \
//    KAI_PP_CONCAT( \
//        KAI_PP_CONCAT("KAI/Platform", KAI_PP_STRINGISE(KAI_PLATFORM_LABEL)) \
//        , KAI_PP_STRINGISE(Header))
// #    define KAI_PLATFORM_Source(Source) \
//     KAI_PP_CONCAT( \
//         KAI_PP_CONCAT("./Platform", KAI_PP_STRINGISE(KAI_PLATFORM_LABEL)) \
//         , KAI_PP_STRINGISE(Header))

#define KAI_UNUSED(A) (&reinterpret_cast<const int&>(A))
#define KAI_UNUSED_1(A) KAI_UNUSED(A)
#define KAI_UNUSED_2(A, B) KAI_UNUSED(A), KAI_UNUSED(B)
#define KAI_UNUSED_3(A, B, C) KAI_UNUSED(A), KAI_UNUSED(B), KAI_UNUSED(C)

#include <KAI/Core/Config/IntegralTypes.h>
#include <KAI/Core/Config/Platform.h>
#include <KAI/Core/FwdDeclarations.h>
