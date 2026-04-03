#pragma once

#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Config/Base.h>

#include <utility>

#include "KAI/Core/Object/Handle.h"
#include "KAI/Core/Object/Object.h"
#include "KAI/Core/Object/StorageBase.h"
#include "KAI/Core/ObjectColor.h"

KAI_BEGIN

template <class Cont, class Fun>
Fun ForEach(Cont &container, Fun fun) {
    for (auto &A : container) {
        if (!fun(A)) break;
    }
    return fun;
}

template <class T>
struct Deleter  // : Function<void (*)(T *)
{
    bool operator()(T *P) {
        delete P;
        return true;
    }
    bool operator()(const T *P) {
        delete const_cast<T *>(P);
        return true;
    }
    template <class A, class B>
    bool operator()(const std::pair<A, B *> &pair) {
        delete pair.second;
        return true;
    }
};

template <class T>
struct IteratedFunctionBase {
    // return false to cease the iteration
    virtual bool Invoke(Object const &Q) = 0;

    bool operator()(Object const &Q) {
        if (!Q.Exists()) return true;
        return Invoke(Q);
    }
    bool operator()(Object &Q) {
        if (!Q.Exists()) return true;
        return Invoke(Q);
    }
    bool operator()(std::pair<Object, Object> &M) {
        return Invoke(M.first) && Invoke(M.second);
    }
    bool operator()(std::pair<const Object, Object> &M) {
        return Invoke(const_cast<Object &>(M.first)) && Invoke(M.second);
    }
    bool operator()(std::pair<const String, Object> &M) {
        return Invoke(M.second);
    }
    template <class T2>
    bool operator()(std::pair<const String, Pointer<T2> > &M) {
        return Invoke(M.second);
    }
};

template <class T>
struct SetSwitch : IteratedFunctionBase<T> {
    int val;
    bool on;
    SetSwitch(int V, bool Q) : val(V), on(Q) {}
    bool Invoke(Object const &object) {
        object.SetSwitch(val, on);
        return true;
    }
};

template <class T>
struct SetMarked : IteratedFunctionBase<T> {
    bool mark;
    SetMarked(bool Q) : mark(Q) {}
    bool Invoke(Object const &M) {
        MarkObjectAndChildren(M, mark);
        return true;
    }
};

template <class T>
struct CanBlackenFun : IteratedFunctionBase<T> {
    bool can_make_black;
    CanBlackenFun() : can_make_black(true) {}
    bool Invoke(Object const &Q) {
        StorageBase *base = Q.GetStorageBase(Q.GetHandle());
        return can_make_black = can_make_black && base && !base->IsWhite();
    }
};

template <class T>
struct SetObjectColor : IteratedFunctionBase<T> {
    ObjectColor::Color color;
    SetObjectColor(ObjectColor::Color C) : color(C) {}
    bool Invoke(Object const &Q) {
        Object(Q).SetColor(color);
        return true;
    }
};

template <class T>
struct SetObjectColorRecursive : IteratedFunctionBase<T> {
    ObjectColor::Color color;
    HandleSet *handles;
    SetObjectColorRecursive(ObjectColor::Color C, HandleSet &H)
        : color(C), handles(&H) {}

    bool Invoke(Object const &Q) {
        Object(Q).SetColorRecursive(color, *handles);
        return true;
    }
};

template <class T>
struct MakeReachableGreyFun : IteratedFunctionBase<T> {
    bool Invoke(Object const &Q) {
        StorageBase *base = Q.GetStorageBase(Q.GetHandle());
        if (base != 0 && base->IsWhite()) base->SetColor(ObjectColor::Grey);
        return true;
    }
};

template <class T, class C>
struct AddContainedFun : IteratedFunctionBase<T> {
    typedef C OutputContainer;
    OutputContainer *output;
    AddContainedFun(OutputContainer &L) : output(&L) {}
    bool Invoke(Object const &Q) {
        if (!Q.Valid()) return true;
        StorageBase *base = Q.GetStorageBase(Q.GetHandle());
        if (base != 0) output->push_back(*base);
        return true;
    }
};

KAI_END
