#pragma once

/// Strip all qualifiers from a given type.

template <class T>
struct RootType {
    typedef T Type;
};

template <class T>
struct RootType<T &> {
    typedef T Type;
};

template <class T>
struct RootType<const T &> {
    typedef T Type;
};

template <class T>
struct RootType<T &&> {
    typedef T Type;
};
