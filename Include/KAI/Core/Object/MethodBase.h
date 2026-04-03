#pragma once

#include <KAI/Core/BuiltinTypes/Stack.h>
#include <KAI/Core/Config/Base.h>

#include "KAI/Core/CallableBase.h.notused"
#include "KAI/Core/Object/Constness.h"

KAI_BEGIN

/// Common for all methods_ that return void or not, and are const or not
class MethodBase : public CallableBase<MethodBase> {
   public:
    Type::Number class_type;
    Constness constness;
    MethodBase(Constness C, const Label &N)
        : constness(C), CallableBase<MethodBase>(N) {}

    virtual ~MethodBase() {}

    Type::Number GetClassType() const { return class_type; }
    Constness GetConstness() const { return constness; }

    void Invoke(Object const &Q, Stack &stack) {
        if (Q.IsConst())
            ConstInvoke(Q, stack);
        else
            NonConstInvoke(Q, stack);
    }

    virtual void NonConstInvoke(const Object &servant, Stack &stack) = 0;
    virtual void ConstInvoke(const Object &servant, Stack &stack) = 0;

    Object GetArgumentTypesArray() const;
    String ToString() const;
    static void Register(Registry &);
};

template <class Method>
struct ConstMethodBase : MethodBase {
    typedef Method MethodType;
    Method method;
    ConstMethodBase(Method M, const Label &N)
        : method(M), MethodBase(Constness::Const, N) {}

    void NonConstInvoke(const Object &Q, Stack &S) { ConstInvoke(Q, S); }
    ConstMethodBase(Method M, const Label &N, Constness C)
        : method(M), MethodBase(C, N) {}
};

template <class Method>
struct MutatingMethodBase : ConstMethodBase<Method> {
    MutatingMethodBase(Method M, const Label &N)
        : ConstMethodBase<Method>(M, N, Constness::Mutable) {}

    void ConstInvoke(const Object &, Stack &) {
        KAI_THROW_1(ConstError, "Mutating method");
    }
};

StringStream &operator<<(StringStream &, const BasePointer<MethodBase> &);

KAI_TYPE_TRAITS(BasePointer<MethodBase>, Number::Method,
                Properties::StringStreamInsert | Properties::Reflected);

KAI_END
