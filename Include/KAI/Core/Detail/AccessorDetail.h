#pragma once

#include <KAI/Core/Object.h>
#include <KAI/Core/Object/AccessorBase.h>
#include <KAI/Core/Registry.h>
#include <KAI/Core/TypeInfo.h>

KAI_BEGIN

namespace property_detail {

// common functionality for accessors and mutators of either system or
// non-system types a `non-system` type is anything that is not convertible
// to an Object a `system type` is an Object, a Pointer<T>, a Pointer<const
// T>, or a Value<T> or Value<const T>
template <class Base, class C, class T, class F>
struct CommonBase : Base {
    typedef F Field;
    Field field;
    CommonBase(Field G, Label L, bool B, MemberCreateParams::Enum createParams)
        : Base(L, Type::Traits<C>::Number, Type::Traits<T>::Number, B,
               createParams),
          field(G) {}
};

// common functionality for accessors and mutators of non-system types.
template <class Base, class C, class T, class F>
struct NonSystemProperty : CommonBase<Base, C, T, F> {
    typedef CommonBase<Base, C, T, F> Parent;
    using typename Parent::Field;

    NonSystemProperty(Field G, Label const &L,
                      MemberCreateParams::Enum createParams)
        : CommonBase<Base, C, T, F>(G, L, false, createParams) {}

    // cannot get or set the property object for non-system types
    Object GetObject(Object const &) const { KAI_THROW_0(NoOperation); }
    void SetObject(Object const &, Object const &) const {
        KAI_THROW_0(NoOperation);
    }

    // non-system types cannot be marked
    void SetMarked(Object &, bool) {}
};

// common functionality for accessors and mutators of properties_ which are
// system types.
template <class Base, class C, class T, class F>
struct SystemProperty : CommonBase<Base, C, T, F> {
    typedef CommonBase<Base, C, T, F> Parent;
    using Parent::field;
    using typename Parent::Field;

    SystemProperty(Field field, Label const &L,
                   MemberCreateParams::Enum createParams)
        : CommonBase<Base, C, T, F>(field, L, true, createParams) {}

    Object GetObject(Object const &Q) const { return Deref<C>(Q).*field; }

    void SetObject(Object const &owner, Object const &value) const {
        Object &object = Deref<C>(owner).*field;
        if (object.Exists()) object.RemovedFromContainer(owner);

        if (!value.Valid())
            object = Object();
        else
            (object = value).AddedToContainer(owner);
    }

    void SetMarked(Object &Q, bool B) {
        MarkObjectAndChildren(Deref<C>(Q).*field, B);
    }
};

// accessor to non-system properties_
template <class Class, class C, bool, class T, class = T>
struct Accessor : NonSystemProperty<AccessorBase, C, T, T(Class::*)> {
    typedef NonSystemProperty<AccessorBase, C, T, T(Class::*)> Parent;
    using Parent::field;
    using typename Parent::Field;

    Accessor(Field F, Label const &L, MemberCreateParams::Enum createParams)
        : NonSystemProperty<AccessorBase, C, T, T(Class::*)>(F, L,
                                                             createParams) {}

    Object GetValue(Object const &object) const {
        return object.GetRegistry()->New(ConstDeref<C>(object).*field);
    }
};

// accessor to system properties_
template <class Class, class C, class T, class S>
struct Accessor<Class, C, true, T, S>
    : SystemProperty<AccessorBase, C, T, S(Class::*)> {
    typedef SystemProperty<AccessorBase, C, T, S(Class::*)> Parent;
    using Parent::field;
    using typename Parent::Field;

    Accessor(Field F, Label const &L, MemberCreateParams::Enum createParams)
        : SystemProperty<AccessorBase, C, T, S(Class::*)>(F, L, createParams) {}

    Object GetValue(Object const &object) const {
        return ConstDeref<C>(object).*field;
    }
};

// mutator for non-system types
template <class K, class Class, bool, class T, class S>
struct Mutator : NonSystemProperty<MutatorBase, K, T, T(Class::*)> {
    typedef NonSystemProperty<MutatorBase, K, T, T(Class::*)> Parent;
    using Parent::field;
    using typename Parent::Field;

    Mutator(Field F, Label const &L, MemberCreateParams::Enum createParams)
        : NonSystemProperty<MutatorBase, K, T, T(Class::*)>(F, L,
                                                            createParams) {}

    Object GetValue(Object const &object) const {
        return object.GetRegistry()->New(ConstDeref<K>(object).*field);
    }

    void SetValue(Object const &object, Object const &value) const {
        Deref<K>(object).*field = ConstDeref<T>(value);
    }
};

// mutator for system types
template <class K, class Class, class T, class S>
struct Mutator<K, Class, true, T, S>
    : SystemProperty<MutatorBase, K, T, S(Class::*)> {
    typedef SystemProperty<MutatorBase, K, T, S(Class::*)> Parent;
    typedef typename Parent::Field Field;
    using Parent::field;

    Mutator(Field field, Label const &label,
            MemberCreateParams::Enum createParams)
        : SystemProperty<MutatorBase, K, T, S(Class::*)>(field, label,
                                                         createParams) {}

    Object GetValue(Object const &Q) const {
        return ConstDeref<K>(Q).*
               field;  // just return the object for system types
    }

    void SetValue(Object const &Q, Object const &V) const {
        Object object = Deref<K>(Q).*field;  // get the containing object
        if (!object.Exists()) {
            Deref<K>(Q).*field = Q.GetRegistry()->New(ConstDeref<T>(V));
            return;
        }

        object.Assign(object.GetStorageBase(),
                      V.GetStorageBase());  // use the class system to perform
                                            // the assignment
    }
};

// make an accessor (read-only) property
// Class is the type of the containing structure
// Class is the type of the structure which defines the member. this may be the
// same as Class. T is the type of the property
template <class K, class C, class T>
struct MakeAccessor
    : Accessor<K, C, TypeInfo<T>::IsSytem, typename TypeInfo<T>::ValueType,
               typename TypeInfo<T>::StorageType> {
    typedef Accessor<K, C, TypeInfo<T>::IsSytem,
                     typename TypeInfo<T>::ValueType,
                     typename TypeInfo<T>::StorageType>
        Parent;
    typedef typename Parent::Field Field;

    MakeAccessor(Field F, Label const &L, MemberCreateParams::Enum createParams)
        : Accessor<K, C, TypeInfo<T>::IsSytem, typename TypeInfo<T>::ValueType,
                   typename TypeInfo<T>::StorageType>(F, L, createParams) {}
};

// make a mutator (read-write) property
template <class K, class C, class T>
struct MakeMutator
    : Mutator<K, C, TypeInfo<T>::IsSytem, typename TypeInfo<T>::ValueType,
              typename TypeInfo<T>::StorageType> {
    typedef Mutator<K, C, TypeInfo<T>::IsSytem, typename TypeInfo<T>::ValueType,
                    typename TypeInfo<T>::StorageType>
        Parent;
    typedef typename Parent::Field Field;

    MakeMutator(Field F, Label const &L, MemberCreateParams::Enum createParams)
        : Mutator<K, C, TypeInfo<T>::IsSytem, typename TypeInfo<T>::ValueType,
                  typename TypeInfo<T>::StorageType>(F, L, createParams) {}
};
}  // namespace property_detail

KAI_END
