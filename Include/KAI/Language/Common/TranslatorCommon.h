#pragma once

#include <KAI/Executor/Continuation.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Common/ProcessCommon.h>
#include <KAI/Language/Common/Structure.h>

KAI_BEGIN

/// Common to all systems that translate from a text stream to a Continuation
struct TranslatorCommon : ProcessCommon {
   public:
    TranslatorCommon(Registry &r);

    virtual Pointer<Continuation> Translate(
        const char *text, Structure = Structure::Statement) = 0;

    std::string ToString() const;

    void PushNew();
    Pointer<Continuation>
    Pop();  // fault in system: only continuations can be pushed/popped
    Pointer<Continuation> Top();
    Pointer<const Continuation> Top() const;
    virtual Pointer<Continuation> Result() { return Top(); }

    void Append(Object const &ob);

    template <class T>
    void AppendNew(T val) {
        Append(_reg->New<T>(val));
    }

    void AppendOp(Operation::Type op);
    void AppendDirectOperation(Operation::Type op);

    struct Exception {};
    struct Unsupported : Exception {};

   protected:
    std::vector<Pointer<Continuation> > stack;
    using ProcessCommon::_reg;
};

KAI_END
