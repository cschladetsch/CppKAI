#pragma once

#include <KAI/Core/Object/ClassBase.h>
#include <KAI/Core/Object/GetStorageBase.h>
#include <KAI/Core/Type/Traits.h>

KAI_BEGIN

/// Common base for ConstPointer<T> and Pointer<T>
/// To dereference a Pointer<T> requires a hash-table lookup.
template <class T>
class PointerBase {
   public:
    typedef Type::Traits<T> Traits;
    typedef typename Traits::Reference Reference;
    typedef typename Traits::ConstReference ConstReference;
    typedef typename Traits::Pointer PointerType;
    typedef typename Traits::ConstPointer ConstPointerType;

   protected:
    bool CanAssign(const Object &Q) const {
        if (!Q.Exists()) return true;
        return CanAssign(Q.GetStorageBase());
    }

    bool CanAssign(const StorageBase &P) const {
        Type::Number type = P.GetTypeNumber();
        if (type == Type::Number::None) return true;
        if (type == Type::Traits<Object>::Number) return true;
        if (type != Traits::Number)
            KAI_THROW_2(TypeMismatch, Traits::Number, type.ToInt());
        return true;
    }

#ifdef KAI_POINTER_HAS_STORAGEBASE
    mutable ConstPointerType pointer;
#endif
};

template <>
struct PointerBase<Object> {
    typedef Type::Traits<Object>::ConstPointer ConstPointerType;

    bool CanAssign(const StorageBase &) const { return true; }

#ifdef KAI_POINTER_HAS_STORAGEBASE
    mutable ConstPointerType pointer;
#endif
};

/// Provides const-only access to the underlying StorageBase
template <class T>
struct ConstPointer : PointerBase<T>, Object {
    typedef typename PointerBase<T>::ConstReference ConstReference;
    typedef typename PointerBase<T>::ConstPointerType ConstPointerType;
    using PointerBase<T>::CanAssign;
#ifdef KAI_POINTER_HAS_STORAGEBASE
    using PointerBase<T>::pointer;
#endif
    // typedef typename PointerBase<T>::PointerType;

   protected:
    ConstPointer() {}

    ConstPointer(const Object &Q) {
        if (PointerBase<T>::CanAssign(Q)) Object::operator=(Q);
    }

    ConstPointer(const StorageBase *Q) {
        if (Q == 0) return;
        if (PointerBase<T>::CanAssign(*Q)) Object::operator=(*Q);
    }

   public:
    ConstPointer<T> &operator=(const ConstPointer<T> &X) {
        Object::operator=(X);
        return *this;
    }

    ConstReference operator*() const {
        ConstReference ref = ConstDeref<T>(*this);
#ifdef KAI_POINTER_HAS_STORAGEBASE
        pointer = &ref;
#endif
        return ref;
    }
    ConstPointerType operator->() const { return &**this; }

    ConstReference GetConstReference() const { return **this; }
};

/// Provides mutable access to the underlying storage
template <class T>
struct Pointer : PointerBase<T>, Object {
#ifdef KAI_POINTER_HAS_STORAGEBASE
    using PointerBase<T>::pointer;
#endif

    Pointer() {
#ifdef KAI_POINTER_HAS_STORAGEBASE
        pointer = 0;
#endif
    }
    Pointer(const Object &Q) { Pointer<T>::Assign(Q); }

    explicit Pointer(StorageBase *Q) {
        if (Q == 0) return;
        if (Q->IsConst()) KAI_THROW_0(ConstError);
        if (PointerBase<T>::CanAssign(*Q)) Assign(*Q);
    }
    Pointer<T> &operator=(const Pointer<T> &P) {
        Object::operator=(P);
#ifdef KAI_POINTER_HAS_STORAGEBASE
        pointer = 0;
        if (Exists()) GetConstReference();
#endif
        return *this;
    }

    typedef typename PointerBase<T>::Reference Reference;
    typedef typename PointerBase<T>::ConstReference ConstReference;
    typedef typename PointerBase<T>::PointerType PointerType;
    typedef typename PointerBase<T>::ConstPointerType ConstPointerType;

    Reference operator*() { return GetReference(); }
    PointerType operator->() { return &**this; }

    ConstReference operator*() const { return GetConstReference(); }
    ConstPointerType operator->() const { return &**this; }

    ConstReference GetConstReference() const {
        ConstReference ref = ConstDeref<T>(*this);
#ifdef KAI_POINTER_HAS_STORAGEBASE
        pointer = &ref;
#endif
        return ref;
    }
    Reference GetReference() const {
        Reference ref = Deref<T>(*this);
#ifdef KAI_POINTER_HAS_STORAGEBASE
        pointer = &ref;
#endif
        return ref;
    }

   protected:
    void Assign(const Object &Q) {
        if (!PointerBase<T>::CanAssign(Q)) return;
        if (Q.GetHandle() != Handle(0) && Q.Exists() && Q.IsConst())
            KAI_THROW_0(ConstError);
        Object::operator=(Q);
#ifdef KAI_POINTER_HAS_STORAGEBASE
        if (Exists()) GetConstReference();
#endif
    }

   private:
    // do *not* define these
    Pointer(const Pointer<const T> &);
};

/// Special case to allow const pointers to be defined by type
template <class T>
struct Pointer<const T> : ConstPointer<T> {
    Pointer() {}
    Pointer(Object const &Q) : ConstPointer<T>(Q) {}
    Pointer(const StorageBase *Q) : ConstPointer<T>(Q) {}
};

/// Special case to avoid type recursion
template <>
struct Pointer<Object> : Object {
    Pointer() {}
    Pointer(Object const &Q) : Object(Q) {}
    Pointer(const StorageBase *Q) {
        if (Q == 0) return;
        Object::operator=(*Q);
    }
    Object &operator*() { return *this; }
    Object *operator->() { return this; }
};

template <class T>
struct Pointer<Pointer<T> > : Pointer<T> {
    Pointer() {}
    Pointer(Object const &Q) : Pointer<T>(Q) {}
    Pointer(const StorageBase *Q) : Pointer<T>(Q) {}
};

template <class T>
struct PointerType {
    typedef Pointer<T> Type;
};

template <class T>
struct PointerType<Pointer<T> > {
    typedef Pointer<T> Type;
};

template <class T>
struct PointerType<T &> {
    typedef Pointer<T> Type;
};

template <class T>
struct PointerType<const T &> {
    typedef Pointer<const T> Type;
};

template <class T>
struct ArgType {
    static T &From(Object const &P) { return Deref<T>(P); }
};

template <class T>
struct ArgType<const T> {
    static const T From(Object const &P) { return ConstDeref<T>(P); }
};

template <class T>
struct ArgType<Pointer<T> > {
    static Pointer<T> const &From(Pointer<T> const &P) { return P; }
};

template <class T>
struct DerefType<Pointer<T> > : DerefType<T> {};

template <class T>
typename DerefType<T>::Reference CleanDeref(StorageBase &base);

namespace Type {
// treat any use of Type::Traits<Pointer<T> > like Type::Traits<Object>
template <class T>
struct Traits<Pointer<T> > : Traits<Object> {};
}  // namespace Type

KAI_END
