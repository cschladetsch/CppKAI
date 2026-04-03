#pragma once

#include "KAI/Core/Object/ConstStorage.h"

KAI_BEGIN

template <class T>
class Storage : public ConstStorage<T> {
   public:
    Storage(const ObjectConstructParams &P) : ConstStorage<T>(P) {}

    typedef typename Type::Traits<T> Tr;

    typename Tr::Reference GetReference() {
        StorageBase::SetDirty();
        return ConstStorage<T>::stored;
    }

    typename Tr::Reference operator*() /*const*/ { return GetReference(); }
    typename Tr::Pointer operator->() /*const*/ { return &GetReference(); }

    typename Tr::Reference GetCleanReference() {
        return ConstStorage<T>::stored;
    }
};

template <class T>
class Storage<const T> : ConstStorage<T> {
    Storage(const ObjectConstructParams &P) : ConstStorage<T>(P) {}
    typedef typename ConstStorage<T>::Traits Tr;

    typename Tr::Reference GetReference() { KAI_THROW_0(ConstError); }
};

KAI_END
