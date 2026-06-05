#pragma once

#ifndef KAI_PRE_PROCESSOR_H
#define KAI_PRE_PROCESSOR_H
#
// Token-paste and stringise helpers (formerly aliased to boost/preprocessor).
#define KAI_PP_CAT_II(a, b) a##b
#define KAI_PP_CAT_I(a, b) KAI_PP_CAT_II(a, b)
#define KAI_PP_CAT(a, b) KAI_PP_CAT_I(a, b)
#define KAI_PP_STRINGISE_I(x) #x
#define KAI_PP_STRINGISE(x) KAI_PP_STRINGISE_I(x)
#
// KAI_STATIC_MESSAGE
// usage:
//        #pragma KAI_STATIC_MESSAGE("hello world")
// results in:
//        File.cpp(123): hello world
// in the compiler output window
#ifdef KAI_COMPILER_MSVC
#define KAI_STATIC_TODO(T)                                                 \
    message(KAI_PP_CAT(                                                    \
        KAI_PP_CAT(KAI_PP_CAT(__FILE__, "("), KAI_PP_STRINGISE(__LINE__)), \
        KAI_STATIC_MESSAGE_TRAIL(KAI_PP_CAT("TODO: ", T))))
#define KAI_STATIC_MESSAGE(T)                                              \
    message(KAI_PP_CAT(                                                    \
        KAI_PP_CAT(KAI_PP_CAT(__FILE__, "("), KAI_PP_STRINGISE(__LINE__)), \
        KAI_STATIC_MESSAGE_TRAIL(T)))
#define KAI_STATIC_MESSAGE_TRAIL(T) \
    KAI_PP_CAT(KAI_PP_CAT(KAI_PP_CAT("): ", "Func"), ": "), T)
#else
#define KAI_STATIC_MESSAGE(T) error(T)
#endif
#endif  // KAI_PRE_PROCESSOR_H
