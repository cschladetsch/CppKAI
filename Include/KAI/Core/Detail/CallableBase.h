#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Meta/Base.h>
#include <KAI/Core/Pointer.h>
#include <KAI/Core/Type/Deref.h>

#include <tuple>

#include "KAI/Core/RootType.h"

KAI_BEGIN

namespace detail {
using namespace std;
using namespace meta;

template <typename F, typename Tuple, size_t... I>
decltype(auto) CallFunImpl(F f, Tuple &&t, index_sequence<I...>) {
    return std::forward<F>(f)(get<I>(std::forward<Tuple>(t))...);
}

// invoke a function with any arity given arguments in a tuple
template <typename F, typename Tuple>
decltype(auto) CallFun(F f, Tuple &&t) {
    using idx = make_index_sequence<tuple_size<decay_t<Tuple>>::value>;
    return CallFunImpl(std::forward<F>(f), std::forward<Tuple>(t), idx{});
}

template <class Q, typename F, typename Tuple, size_t... I>
decltype(auto) CallMethodImpl(Q &q, F f, Tuple &&t, index_sequence<I...>) {
    auto m = std::forward<F>(f);
    return (q.*m)(get<I>(std::forward<Tuple>(t))...);
}

// invoke a method of any arity given arguments in a tuple
template <class Q, typename F, typename Tuple>
decltype(auto) CallMethod(Q &q, F f, Tuple &&t) {
    using idx = make_index_sequence<tuple_size<decay_t<Tuple>>::value>;
    return CallMethodImpl(q, std::forward<F>(f), std::forward<Tuple>(t), idx{});
}

template <class T>
struct MakeAssignment {
    static void Perform(T &A, const Object &B) { A = Deref<T>(B); }
};

template <class T>
struct MakeAssignment<Pointer<T>> {
    static void Perform(Pointer<T> &A, const Object &B) { A = B; }
};

template <class T>
struct MakeAssignment<ConstPointer<T>> : MakeAssignment<T> {};

// pull arguments into a tuple, recursively
template <int N>
struct Add {
    template <class... Args>
    static void Arg(Stack &input, tuple<Args...> &args) {
        typedef typename RootType<decltype(std::get<N>(args))>::Type Ty;
        Object back = input.Top();
        input.Pop();
        MakeAssignment<Ty>::Perform(get<N>(args), back);
        Add<N - 1>::Arg(input, args);
    }
};

// pull last argument
template <>
struct Add<0> {
    template <class... Args>
    static void Arg(Stack &input, tuple<Args...> &args) {
        typedef typename RootType<decltype(std::get<0>(args))>::Type Ty;
        Object back = input.Top();
        input.Pop();
        MakeAssignment<Ty>::Perform(get<0>(args), back);
    }
};
template <>
struct Add<-1> {
    template <class... Args>
    static void Arg(Stack &input, tuple<Args...> &args) {}
};
}  // namespace detail

KAI_END
