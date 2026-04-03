#pragma once

#include <KAI/Core/Base.h>
#include <KAI/Core/Config/Base.h>
#include <KAI/Core/TriColor.h>

KAI_TYPE_BEGIN

template <typename Reference, bool IsContainer>
struct ContainerOperations {
    struct ColorSetter {
        ObjectColor::Color c_;

        ColorSetter(ObjectColor::Color c) : c_(c) {}
        template <class T>
        void operator()(T &obj) {
            obj.SetColor(c_);
        }
    };

    static void SetMarked(Reference R, bool M) { ForEach(R, SetMarked(M)); }
    static void SetSwitch(Reference R, int S, bool M) {
        ForEach(R, SetSwitch(S, M));
    }
    static void SetColor(Reference R, ObjectColor::Color C) {
        ForEach(R, ColorSetter(C));
    }

    static void Erase(Reference R, Object const &Q) { R.Erase(Q); }
    template <class Fun>
    static Fun ForEachContained(Reference R, Fun F) {
        return ForEach(R, F);
    }
};

template <typename Reference>
struct ContainerOperations<Reference, false> {
    static void SetMarked(Reference, bool) {}
    static void SetSwitch(Reference, int, bool) {}
    static void SetColor(Reference, ObjectColor::Color) {}
    static void Erase(Reference, Object const &) {}

    template <class Fun, class Ref>
    static Fun ForEachContained(Ref, Fun const &F) {
        return F;
    }
};

KAI_TYPE_END
