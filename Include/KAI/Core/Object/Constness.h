#pragma once

#include <KAI/Core/Base.h>

KAI_BEGIN

/// Specify the constness of a thing
struct Constness {
    enum Type { None, Const, Mutable };
    Type value;
    Constness(Type T = None) : value(T) {}

    const char *ToString() const;
    friend bool operator==(Constness A, Constness B) {
        return A.value == B.value;
    }
    friend bool operator<(Constness A, Constness B) {
        return A.value < B.value;
    }
};

KAI_END
