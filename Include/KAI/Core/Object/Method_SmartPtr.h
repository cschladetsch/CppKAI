#pragma once

#include <KAI/Core/Detail/Arity.h>
#include <KAI/Core/Meta/Base.h>

#include <memory>

#include "KAI/Core/Detail/CallableBase.h"
#include "MethodBase.h"

KAI_BEGIN

namespace method_detail {
using namespace std;
using namespace meta;
using namespace detail;

// Same method implementations as before...
// (keeping the actual method classes unchanged)

template <class T, class R, class... Args>
struct MethodConst : ConstMethodBase<R (T::*)(Args...) const> {
    typedef R (T::*MethodType)(Args...) const;
    typedef ConstMethodBase<MethodType> Parent;
    MethodType meth;
    tuple<Args...> args_;
    static int constexpr arity = (int)sizeof...(Args);

    MethodConst(MethodType m, const Label &N) : meth(m), Parent(m, N) {}

    void ConstInvoke(const Object &servant, Stack &stack) {
        detail::Add<arity - 1>::Arg(stack, args_);
        stack.Push(servant.GetRegistry()->New(
            CallMethod(ConstDeref<T>(servant), meth, args_)));
    }
};

// ... other method implementations remain the same ...

}  // namespace method_detail

// Method template remains the same
template <class T, class R, bool C, class... Args>
struct Method : method_detail::Selector<T, R, C, Args...>::Type {
    typedef typename method_detail::Selector<T, R, C, Args...>::Type Parent;
    Method(typename Parent::MethodType M, const Label &L) : Parent(M, L) {}
};

// UPDATED: Factory methods now return unique_ptr
template <class T, class R, class... Args>
std::unique_ptr<MethodBase> MakeMethod(R (T::*method)(Args...),
                                       const Label &N) {
    return std::make_unique<Method<T, R, false, Args...>>(method, N);
}

template <class T, class R, class... Args>
std::unique_ptr<MethodBase> MakeMethod(R (T::*method)(Args...) const,
                                       const Label &N) {
    return std::make_unique<Method<T, R, true, Args...>>(method, N);
}

// Backward compatibility wrapper - use with caution
template <class T, class R, class... Args>
[[deprecated("Use MakeMethod returning unique_ptr instead")]]
MethodBase *MakeMethodRaw(R (T::*method)(Args...), const Label &N) {
    return MakeMethod(method, N).release();
}

template <class T, class R, class... Args>
[[deprecated("Use MakeMethod returning unique_ptr instead")]]
MethodBase *MakeMethodRaw(R (T::*method)(Args...) const, const Label &N) {
    return MakeMethod(method, N).release();
}

KAI_END