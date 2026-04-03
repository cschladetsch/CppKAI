#pragma once

#include "KAI/Network/NetObject.h"

KAI_NET_BEGIN

template <class T>
struct NetPointer : Pointer<T> {
   private:
    NetHandle netHandle_;
    Domain *domain_;

   public:
    NetHandle GetNetHandle() const;
};

KAI_NET_END
