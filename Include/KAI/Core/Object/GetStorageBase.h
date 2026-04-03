#pragma once

#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Type/Deref.h>
#include <KAI/Core/Type/Number.h>

#include "KAI/Core/Object/Object.h"
#include "KAI/Core/Object/Storage.h"
#include "KAI/Core/Object/StorageBase.h"

KAI_BEGIN

StorageBase &GetStorageBase(Object const &Q);

Type::Number GetTypeNumber(Object const &Q);

template <class T>
Storage<typename Type::Traits<T>::Store> &GetStorage(Pointer<T> const &P) {
    return GetStorage<T>(GetStorageBase(P));
}

template <class T>
Storage<typename Type::Traits<T>::Store> &GetStorage(StorageBase &base) {
    if (base.GetTypeNumber().GetValue() != Type::Traits<T>::Number)
        KAI_THROW_2(TypeMismatch, base.GetTypeNumber().ToInt(),
                    Type::Traits<T>::Number);
    return static_cast<Storage<typename Type::Traits<T>::Store> &>(base);
}

template <class T>
typename DerefType<T>::Reference Deref(StorageBase &base) {
    if (base.IsConst()) KAI_THROW_0(ConstError);
    return GetStorage<typename DerefType<T>::Value>(base).GetReference();
}

template <class T>
typename DerefType<T>::Reference CleanDeref(StorageBase &base) {
    return GetStorage<typename DerefType<T>::Value>(base).GetCleanReference();
}

template <class T>
typename DerefType<T>::Reference Deref(Object const &Q) {
    return Deref<T>(GetStorageBase(Q));
}

template <class T>
ConstStorage<T> const &GetConstStorage(StorageBase const &base) {
    if (base.GetTypeNumber() != Type::Traits<T>::Number)
        KAI_THROW_2(TypeMismatch, base.GetTypeNumber().ToInt(),
                    Type::Traits<T>::Number);
    return static_cast<ConstStorage<T> const &>(base);
}

template <class T>
typename DerefType<T>::ConstReference ConstDeref(StorageBase const &base) {
    return GetConstStorage<typename DerefType<T>::Value>(base)
        .GetConstReference();
}

template <class T>
typename DerefType<T>::ConstReference ConstDeref(const Object &Q) {
    return ConstDeref<T>(GetStorageBase(Q));
}

template <>
inline Object &Deref<Object>(Object const &Q) {
    return const_cast<Object &>(Q);
}

template <>
inline const Object &ConstDeref<Object>(const Object &Q) {
    return Q;
}

StorageBase &GetStorageBase(Object const &Q);

KAI_END
