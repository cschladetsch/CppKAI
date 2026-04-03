#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Detail/AddArgType.h>
#include <KAI/Core/Detail/Arity.h>
#include <KAI/Core/Meta/Base.h>

#include <concepts>
#include <functional>
#include <type_traits>

#include "KAI/Core/BuiltinTypes/Void.h"
#include "KAI/Core/Detail/CallableBase.h"
#include "KAI/Core/FunctionBase.h"
#include "KAI/Core/Type.h"

KAI_BEGIN

namespace function_detail {
using namespace detail;

template <class... Args>
struct VoidFun : FunctionBase {
    // Modern version using std::function instead of raw function pointer
    using Func = std::function<void(Args...)>;
    Func fun;
    using FunctionBase::arguments;
    using FunctionBase::return_type;

    static constexpr int arity = sizeof...(Args);

    VoidFun(Func f, const Label &N) : fun(std::move(f)), FunctionBase(N) {
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
    // Modern version using std::function
    using Func = std::function<Ret(Args...)>;
    Func fun;
    std::tuple<Args...> args_;
    static constexpr int arity = sizeof...(Args);
    using FunctionBase::arguments;
    using FunctionBase::return_type;

    NonVoidFun(Func f, const Label &N) : fun(std::move(f)), FunctionBase(N) {
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
    // C++23 uses std::is_same_v instead of the old SameType
    static constexpr bool VoidRet = std::is_same_v<Ret, void>;
    using Type =
        std::conditional_t<VoidRet, VoidFun<Args...>, NonVoidFun<Ret, Args...>>;
};
}  // namespace function_detail

template <class R, class... Args>
struct Function : function_detail::FunctionSelect<R, Args...>::Type {
    using Parent = typename function_detail::FunctionSelect<R, Args...>::Type;

    // Support both traditional function pointers and std::function
    Function(R (*fp)(Args...), const Label &N)
        : Parent(std::function<R(Args...)>(fp), N) {}

    Function(std::function<R(Args...)> f, const Label &N)
        : Parent(std::move(f), N) {}
};

#ifndef KAI_UNNAMED_FUNCTION_NAME
#define KAI_UNNAMED_FUNCTION_NAME Label("UnnamedFunction")
#endif

inline FunctionBase *AddDescription(FunctionBase *F, const char *D) {
    if (D) F->Description = D;
    return F;
}

// Support both traditional function pointers and std::function
template <class R, class... Args>
FunctionBase *MakeFunction(R (*Fun)(Args...),
                           const Label &L = KAI_UNNAMED_FUNCTION_NAME,
                           const char *D = nullptr) {
    return AddDescription(new Function<R, Args...>(Fun, L), D);
}

template <class R, class... Args>
FunctionBase *MakeFunction(std::function<R(Args...)> Fun,
                           const Label &L = KAI_UNNAMED_FUNCTION_NAME,
                           const char *D = nullptr) {
    return AddDescription(new Function<R, Args...>(std::move(Fun), L), D);
}

/// Make a new function object, add it to the given object
template <class R, class... Args>
Pointer<BasePointer<FunctionBase>> AddFunction(
    Object &root, R (*Fun)(Args...), const Label &L = KAI_UNNAMED_FUNCTION_NAME,
    const char *D = nullptr) {
    auto fun = MakeFunction(Fun, L, D);
    auto ptr = root.GetRegistry()->template New<BasePointer<FunctionBase>>(fun);
    root.SetChild(L, ptr);
    return ptr;
}

// Support for std::function
template <class R, class... Args>
Pointer<BasePointer<FunctionBase>> AddFunction(
    Object &root, std::function<R(Args...)> Fun,
    const Label &L = KAI_UNNAMED_FUNCTION_NAME, const char *D = nullptr) {
    auto fun = MakeFunction(std::move(Fun), L, D);
    auto ptr = root.GetRegistry()->template New<BasePointer<FunctionBase>>(fun);
    root.SetChild(L, ptr);
    return ptr;
}

// Support for lambdas and other callable objects
template <typename Callable,
          typename R = std::invoke_result_t<Callable, Object &, Stack &>,
          typename = std::enable_if_t<!std::is_function_v<
              std::remove_pointer_t<std::decay_t<Callable>>>>,
          typename = std::enable_if_t<!std::is_same_v<
              std::decay_t<Callable>, std::function<R(Object &, Stack &)>>>>
Pointer<BasePointer<FunctionBase>> AddFunction(
    Object &root, Callable &&callable,
    const Label &L = KAI_UNNAMED_FUNCTION_NAME, const char *D = nullptr) {
    std::function<R(Object &, Stack &)> func = std::forward<Callable>(callable);
    return AddFunction(root, std::move(func), L, D);
}

KAI_END
