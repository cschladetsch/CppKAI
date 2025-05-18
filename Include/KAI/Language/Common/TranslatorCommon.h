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
        Append(reg_->New<T>(val));
    }

    // Append an operation to the current continuation
    void AppendOp(Operation::Type op);

    // Add operations directly to the continuation without wrapping in another
    // continuation Used by Rho language to avoid unnecessary nesting and allow
    // proper evaluation
    void AppendDirectOperation(Operation::Type op);

    // Mark the current continuation as a Rho expression
    // This helps the executor identify and properly evaluate Rho expressions
    void MarkAsRhoExpression();

    // Add literal values directly to the code array
    // This helps avoid unnecessary wrapping in Rho expressions
    template <typename T>
    void AppendLiteral(const T &value) {
        // Use explicit type parameter for New to ensure proper type
        // identification
        Object obj = reg_->New<T>(value);

        // Add debug logging to track type information
        KAI_TRACE() << "AppendLiteral: Created value of type: "
                    << obj.GetClass()->GetName().ToString()
                    << ", value: " << obj.ToString();

        Append(obj);
    }

    struct Exception {};
    struct Unsupported : Exception {};

   protected:
    std::vector<Pointer<Continuation> > stack;
    using ProcessCommon::reg_;
};

KAI_END
