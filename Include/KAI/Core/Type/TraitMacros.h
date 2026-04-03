#pragma once

#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object/Handle.h>

#include "KAI/Core/Type/Number.h"
#include "KAI/Core/Type/Properties.h"
#include "KAI/Core/Type/Traits.h"

KAI_BEGIN

#define KAI_TYPE_TRAITS_NAMED(T, N, M, Ops)    \
    namespace Type {                           \
    template <>                                \
    struct Traits<T> : TraitsBase<T, N, Ops> { \
        typedef TraitsBase<T, N, Ops> Tr;      \
        using typename Tr::Store;              \
        using typename Tr::Reference;          \
        using typename Tr::ConstReference;     \
        static std::string Name() {            \
            typedef TraitsBase<T, N, Ops> Tr;  \
            return Tr::Name();                 \
        }                                      \
    };                                         \
    }

// T = the type
// N = the number from ::kai::Type::Number
// P = operations allowed on the type
#define KAI_TYPE_TRAITS(T, N, Ops) \
    KAI_TYPE_TRAITS_NAMED(T, N, #T, Ops | Properties::Assign)

#define KAI_TYPE_TRAITS_BASIC(T, N) KAI_TYPE_TRAITS(T, N, 0)

// need these defined before they are fully defined for forward references to
// their Traits
KAI_TYPE_TRAITS(String, Number::String,
                Properties::Streaming | Properties::Plus | Properties::Equiv |
                    Properties::Relational | Properties::Boolean)

KAI_TYPE_TRAITS(Handle, Number::Handle,
                Properties::Boolean | Properties::StringStreamInsert)

KAI_TYPE_TRAITS(::kai::Type::Number, ::kai::Type::Number::TypeNumber,
                ::kai::Type::Properties::StringStreamInsert);

KAI_END
