#pragma once

#include <KAI/Core/Meta/Base.h>

// This file is maintained for backward compatibility
// The main implementation has been moved to Core/Event.h using variadic
// templates

namespace event_detail {
using namespace meta;

// Null type for default template arguments
struct Null {};

// Arity helper for backward compatibility
template <class T0 = Null, class T1 = Null, class T2 = Null>
struct Arity {
    // 0, 1, 2, or 3 arguments
    enum {
        Value = std::is_same_v<T0, Null>   ? 0
                : std::is_same_v<T1, Null> ? 1
                : std::is_same_v<T2, Null> ? 2
                                           : 3
    };
};

// EventType for backward compatibility with old Event system
template <int>
struct EventType {
    template <class T0, class T1, class T2>
    struct Create {
        // Implementation has been moved to the main Event.h
    };
};
}  // namespace event_detail

// EOF