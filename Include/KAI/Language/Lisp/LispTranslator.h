#pragma once

#include <KAI/Language/Common/TranslatorBase.h>
#include <KAI/Language/Lisp/LispParser.h>

#include <string>
#include <vector>

KAI_BEGIN

// Lowers a Lisp AST onto the same Executor operations Pi and Rho target.
//
// Nothing here is Lisp-specific at runtime: `(+ 1 2)` becomes exactly the
// operation stream Pi's `1 2 +` produces, and a lambda becomes a Continuation
// with named args, which is what Rho compiles its functions to. Lisp is a
// front-end, not a second runtime.
class LispTranslator : public TranslatorBase<LispParser> {
   public:
    typedef TranslatorBase<LispParser> Parent;
    typedef typename Parent::Parser Parser;
    typedef typename Parent::TokenNode TokenNode;
    typedef typename Parent::AstNode AstNode;
    typedef typename Parent::TokenEnum TokenEnum;
    typedef typename Parent::AstEnum AstNodeEnum;
    typedef typename Parent::AstNodePtr AstNodePtr;

    LispTranslator(const LispTranslator &) = delete;
    LispTranslator(Registry &r) : Parent(r) {}

   protected:
    virtual void TranslateNode(AstNodePtr node) override;
    using Parent::reg_;

   private:
    void TranslateProgram(AstNodePtr node);
    void TranslateAtom(AstNodePtr node);
    void TranslateList(AstNodePtr node);
    void TranslateQuote(AstNodePtr node);

    // Returns false if `head` names no special form, so the caller can go on
    // to try primitives and then a plain call.
    bool TranslateSpecialForm(std::string const &head, AstNodePtr list);
    bool TranslatePrimitive(std::string const &head, AstNodePtr list);
    void TranslateCall(AstNodePtr list);

    // Translates children [from..end) of `list` as a statement sequence.
    void TranslateBody(AstNodePtr list, size_t from);

    Pointer<Continuation> MakeLambda(std::vector<AstNodePtr> const &params,
                                     AstNodePtr body, size_t bodyFrom);

    // Left-associative fold, e.g. (- 10 2 3) => ((10-2)-3). `identity` is what
    // a zero-argument call yields; `unaryUsesIdentity` makes (- x) mean 0-x
    // and (/ x) mean 1/x rather than just x.
    void TranslateFold(AstNodePtr list, Operation::Type op, int identity,
                       bool unaryUsesIdentity);
    void TranslateCondFrom(AstNodePtr list, size_t index);

    void StoreTo(std::string const &name);
    bool ExpectArgs(AstNodePtr list, size_t count, const char *form);
};

KAI_END
