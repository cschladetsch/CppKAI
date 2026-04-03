#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/Number.h>

KAI_BEGIN

namespace function_detail {
template <int N, class... Args>
struct AddArgType;

template <int N, class Head, class... Tail>
struct AddArgType<N, Head, Tail...> {
    static void Add(std::vector<Type::Number> &args) {
        args.push_back(Type::Traits<Head>::Number);
        AddArgType<N - 1, Tail...>::Add(args);
    }
};

template <class Last>
struct AddArgType<1, Last> {
    static void Add(std::vector<Type::Number> &args) {
        args.push_back(Type::Traits<Last>::Number);
    }
};

template <>
struct AddArgType<0> {
    static void Add(std::vector<Type::Number> &) {}
};
}  // namespace function_detail

KAI_END
