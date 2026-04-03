#pragma once

#include <KAI/Core/BuiltinTypes/Stack.h>

#include "KAI/KAI.h"

KAI_BEGIN

// namespace detail
//{
//     // TODO: make this redundant with CallableableBase.h
//     template <int N>
//     struct Add
//     {
//         template <class... Args>
//         static void Arg(Stack &input, std::tuple<Args...> &args)
//         {
//             typedef decltype(std::get<N>(args)) Ty;
//             Object back = input.Top();
//             input.Pop();
//             std::get<N>(args) = Deref<Ty>(back);
//             Add<N-1>::Arg(input, args);
//         }
//     };
//
//     template <>
//     struct Add<0>
//     {
//         template <class...Args>
//         static void Arg(Stack &input, std::tuple<Args...> &args)
//         {
//             typedef decltype(std::get<0>(args)) Ty;
//             Object back = input.Top();
//             input.Pop();
//             std::get<0>(args) = Deref<Ty>(back);
//         }
//     };
// }

KAI_END
