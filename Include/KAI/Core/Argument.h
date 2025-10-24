#pragma once

#include <KAI/Core/Config/Base.h>

KAI_BEGIN

template <class T>
struct Argument {
    typedef const T &Type;
};

template <class T>
struct Argument<const T> {
    typedef const T &Type;
};

template <class T>
struct Argument<T &> {
    typedef T &Type;
};

template <class T>
struct Argument<const T &> {
    typedef const T &Type;
};

KAI_END
