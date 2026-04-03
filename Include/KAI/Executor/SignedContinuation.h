#pragma once

#include <KAI/Core/BuiltinTypes/Stack.h>
#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Object/Label.h>
#include <KAI/Core/Type/Number.h>
#include <KAI/Executor/Continuation.h>

#include <vector>

KAI_BEGIN

// A SignedContinuation is just like a normal Continuation, except is knows what
// to expect on the stack when it is executed.
struct SignedContinuation {
    struct FormalParameter {
        Type::Number type;
        Label label;  // pass by ref if quoted
        FormalParameter() {}
        FormalParameter(Type::Number T, Label const &L) : type(T), label(L) {}
    };

    typedef std::vector<FormalParameter> FormalParameters;
    typedef std::vector<Type::Number> ReturnTuple;

    Pointer<Continuation> cont;
    FormalParameters params;
    ReturnTuple return_tuple;

    void Create(Pointer<Array> args, Pointer<Array> rtypes,
                Pointer<Continuation> cont, Pointer<Executor> exec);
    Object GetContinuation() const { return cont; }
    void Enter(Stack &);
    void Leave(Stack &);
    static void Register(Registry &R);
};

StringStream &operator<<(StringStream &, SignedContinuation const &);

KAI_TYPE_TRAITS(SignedContinuation, Number::SignedContinuation,
                Properties::StringStreamInsert);

KAI_END
