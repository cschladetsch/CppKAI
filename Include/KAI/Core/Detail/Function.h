#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Detail/AddArgType.h>
#include <KAI/Core/Detail/Arity.h>
#include <KAI/Core/Meta/Base.h>

#include "KAI/Core/BuiltinTypes/Void.h"
#include "KAI/Core/Detail/CallableBase.h"
#include "KAI/Core/FunctionBase.h"
#include "KAI/Core/Type.h"

KAI_BEGIN

namespace function_detail {
using namespace detail;

template <class... Args>
struct VoidFun : FunctionBase {
    typedef void (*Func)(Args...);
    Func fun;
    using FunctionBase::arguments;
    using FunctionBase::return_type;

    static constexpr int arity = sizeof...(Args);

    VoidFun(Func f, const Label &N) : fun(f), FunctionBase(N) {
        AddArgType<arity, Args...>::Add(arguments);
        return_type = Type::Traits<void>::Number;
    }

    void Invoke(Registry &reg, Stack &stack) {
        std::tuple<Args...> args_;
        Add<arity - 1>::Arg(stack, args_);
        CallFun(fun, args_);
    }
};

template <class Ret, class... Args>
struct NonVoidFun : FunctionBase {
    typedef Ret (*Func)(Args...);
    Func fun;
    std::tuple<Args...> args_;
    static constexpr int arity = sizeof...(Args);
    using FunctionBase::arguments;
    using FunctionBase::return_type;

    NonVoidFun(Func f, const Label &N) : fun(f), FunctionBase(N) {
        AddArgType<arity, Args...>::Add(arguments);
        return_type = Type::Traits<Ret>::Number;
    }

    void Invoke(Registry &reg, Stack &stack) {
        Add<arity - 1>::Arg(stack, args_);
        stack.Push(reg.New<Ret>(CallFun(fun, args_)));
    }
};

template <class Ret, class... Args>
struct FunctionSelect {
    typedef Ret (*Fun)(Args...);
    enum { VoidRet = SameType<Ret, void>::value };
    typedef typename meta::If<VoidRet, VoidFun<Args...>,
                              NonVoidFun<Ret, Args...> >::Type Type;
};
}  // namespace function_detail

template <class R, class... Args>
struct Function : function_detail::FunctionSelect<R, Args...>::Type {
    typedef typename function_detail::FunctionSelect<R, Args...>::Type Parent;
    Function(typename Parent::Func M, const Label &N) : Parent(M, N) {}
};

#ifndef KAI_UNNAMED_FUNCTION_NAME
#define KAI_UNNAMED_FUNCTION_NAME Label("UnnamedFunction")
#endif

inline FunctionBase *AddDescription(FunctionBase *F, const char *D) {
    if (D) F->Description = D;
    return F;
}

template <class R, class... Args>
FunctionBase *MakeFunction(R (*Fun)(Args...),
                           const Label &L = KAI_UNNAMED_FUNCTION_NAME,
                           const char *D = 0) {
    return AddDescription(new Function<R, Args...>(Fun, L), D);
}

/// Make a new function object, add it to the given object
template <class R, class... Args>
Pointer<BasePointer<FunctionBase> > AddFunction(
    Object &root, R (*Fun)(Args...), const Label &L = KAI_UNNAMED_FUNCTION_NAME,
    const char *D = 0) {
    auto fun = MakeFunction(Fun, L, D);
    auto ptr = root.GetRegistry()->New<BasePointer<FunctionBase> >(fun);
    root.SetChild(L, ptr);
    return ptr;
}

KAI_END
