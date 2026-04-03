#pragma once

#include <KAI/Core/Type/Traits.h>

#include "KAI/Core/Object/ObjectConstructParams.h"
#include "KAI/Core/Object/StorageBase.h"

KAI_BEGIN

template <class T>
class ConstStorage : public StorageBase {
    typedef typename Type::Traits<T> Tr;
    typedef typename Tr::Store Store;

   protected:
    Store stored;

   public:
    ConstStorage(const ObjectConstructParams &P)
        : StorageBase(P) {}  //{ SetClean(); }

    typename Tr::ConstReference GetConstReference() const { return stored; }
    typename Tr::ConstReference operator*() const { return stored; }
    typename Tr::ConstPointer operator->() const { return &stored; }
};

KAI_END
