#include "KAI/Executor/SignedContinuation.h"

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "KAI/Core/Object/ClassBuilder.h"
#include "KAI/Executor/Executor.h"

KAI_BEGIN

void SignedContinuation::Create(Pointer<Array> args, Pointer<Array> returnTypes,
                                Pointer<Continuation> continuation,
                                Pointer<Executor> executor) {
    if (args.Exists()) {
        if ((args->Size() & 1) != 0)
            KAI_THROW_1(Base,
                        "SignedContinuation argument list must be a list of "
                        "(type, label)");

        // Add the formal params.
        Array::const_iterator it = args->Begin(), end = args->End();
        while (it != end) {
            Object typeIdent = *it++;
            Pointer<Label> label = *it++;
            Pointer<const ClassBase *> klass = executor->Resolve(typeIdent);
            params.push_back(
                FormalParameter((*klass)->GetTypeNumber(), *label));
        }
    }

    // set the return types
    if (returnTypes.Exists()) {
        // TODO
    }
}

void SignedContinuation::Enter(Stack &stack) { KAI_UNUSED_1(stack); }

void SignedContinuation::Leave(Stack &) {
    // TODO check the return types
}

StringStream &operator<<(StringStream &stream,
                         SignedContinuation const &continuation) {
    stream << "SignedContinuation: ";
    String separator = "";
    SignedContinuation::FormalParameters::const_iterator
        it = continuation.params.begin(),
        end = continuation.params.end();
    for (; it != end; ++it) {
        stream << separator << it->type.TypeNumber << String(" ") << it->label;
        separator = ", ";
    }

    stream << " -> ";
    separator = "";
    SignedContinuation::ReturnTuple::const_iterator
        returnIt = continuation.return_tuple.begin(),
        returnEnd = continuation.return_tuple.end();
    for (; returnIt != returnEnd; ++returnIt) {
        stream << separator << returnIt->GetValue();
        separator = ", ";
    }

    return stream << "\nContinuation: " << continuation.cont;
}

void SignedContinuation::Register(Registry &registry) {
    ClassBuilder<SignedContinuation>(registry, "SignedContinuation");
}

KAI_END
