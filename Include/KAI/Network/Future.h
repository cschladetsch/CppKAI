#pragma once

#include "KAI/Network/Config.h"
#include "KAI/Network/DateTime.h"
#include "KAI/Network/NetHandle.h"
#include "KAI/Network/ResponseType.h"

#include <optional>
#include <stdexcept>
#include <string>

KAI_NET_BEGIN

// Simplified Future class that doesn't have template issues
template <class T = void>
struct Future {
    // Id that is unique to the creator of the Future
    int Id = 0;

    // the response type
    ResponseType Response = ResponseType::None;

    // if true, this future has been completed one way or another
    bool Complete = false;

    // Simplified - no need for DateTime which has dependencies
    // DateTime When;

    // Simplified - no need for Pointer which causes circular dependency
    // Pointer<T> Result;

    std::optional<T> Value;
    std::string ErrorMessage;

    bool Succeeded() const {
        return Complete && Response == ResponseType::Returned;
    }

    const T &GetValue() const {
        if (!Value) {
            throw std::runtime_error("Future does not contain a value");
        }
        return *Value;
    }
};

template <>
struct Future<void> {
    int Id = 0;
    ResponseType Response = ResponseType::None;
    bool Complete = false;
    std::string ErrorMessage;

    bool Succeeded() const {
        return Complete && Response == ResponseType::Returned;
    }
};

KAI_NET_END
