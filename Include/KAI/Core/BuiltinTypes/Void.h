#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Meta/Base.h>
#include <KAI/Core/Type/Number.h>
#include <KAI/Core/Type/Traits.h>

KAI_BEGIN

struct Void {};

inline StringStream &operator<<(StringStream &S, Void const &) { return S; }
inline StringStream &operator>>(StringStream &S, Void &) { return S; }
inline BinaryStream &operator<<(BinaryStream &S, Void const &) { return S; }
inline BinaryPacket &operator>>(BinaryPacket &S, Void &) { return S; }

inline HashValue GetHash(Void) { return 42; }

namespace Type {
template <>
struct Traits<void>
    : TraitsBase<void, Number::Void, 0, /*TODO Properties::Streaming,*/ Void,
                 Void &, const Void &> {
    static const char *Name() { return "void"; }

    struct ContainerOps {
        template <class A, class B, class C>
        static void SetSwitch(A, B, C) {}

        template <class A, class B>
        static void SetMarked(A, B) {}

        template <class A, class B>
        static void ForEachContained(A, B) {}

        template <class A, class B>
        static void MakeReachableGrey(A, B) {}

        template <class A, class B>
        static void DetachFromContainer(A, B) {}

        template <class A, class B>
        static void Erase(A, B){};
    };
};

template <>
struct Traits<meta::Null> : Traits<void> {};
template <>
struct Traits<Void> : Traits<void> {};
}  // namespace Type

KAI_END
