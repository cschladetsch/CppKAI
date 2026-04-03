#pragma once

#include "KAI/Core/Object/Handle.h"
#include "KAI/Network/Address.h"

KAI_NET_BEGIN

struct NetHandle {
    int value;

    NetHandle() : value(0) {}
    explicit NetHandle(int v) : value(v) {}

    friend bool operator==(NetHandle const &A, NetHandle const &B) {
        return A.value == B.value;
    }
};

struct HashNetHandle {
    size_t operator()(NetHandle const &h) const {
        return static_cast<size_t>(h.value);
    }
};

KAI_NET_END
