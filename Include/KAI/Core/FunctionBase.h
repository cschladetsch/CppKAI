#pragma once

#include <KAI/Core/BuiltinTypes/Stack.h>
#include <KAI/Core/Object/Label.h>
#include <KAI/Core/Type.h>

#include <KAI/Core/CallableBase.h.notused>

KAI_BEGIN

struct FunctionBase : CallableBase<FunctionBase> {
    FunctionBase(const Label &L) : CallableBase<FunctionBase>(L) {}

    virtual void Invoke(Registry &, Stack &stack) = 0;
    String ToString() const;
    static void Register(Registry &);
};

StringStream &operator<<(StringStream &, FunctionBase const &);

KAI_TYPE_TRAITS(BasePointer<FunctionBase>, Number::Function,
                Properties::StringStreamInsert);

KAI_END
