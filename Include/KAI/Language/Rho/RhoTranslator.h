#pragma once

#include <KAI/Language/Common/TranslatorBase.h>
#include <KAI/Language/Rho/RhoParser.h>

KAI_BEGIN

class RhoTranslator : public TranslatorBase<RhoParser> {
   public:
    typedef TranslatorBase<RhoParser> Parent;
    typedef typename Parent::Parser Parser;
    typedef typename Parent::TokenNode TokenNode;
    typedef typename Parent::AstNode AstNode;
    typedef typename Parent::TokenEnum TokenEnum;
    typedef typename Parent::AstEnum AstNodeEnum;
    typedef typename Parent::AstNodePtr AstNodePtr;

    RhoTranslator(const RhoTranslator &) = delete;
    RhoTranslator(Registry &r) : Parent(r) {}
    
    // Override to make Rho handle direct evaluation of expressions
    // This addresses the issue where expressions are unnecessarily wrapped in continuations
    Pointer<Continuation> Translate(const char *text, Structure st) override;

   protected:
    virtual void TranslateNode(AstNodePtr node) override;
    using Parent::_reg;

    // Helper method for loop-related continuation creation
    Pointer<Continuation> CreateContinuationAndTranslate(AstNodePtr node) {
        // Use Pointer<Continuation> instead of Object
        Pointer<Continuation> cont = _reg->New<Continuation>();
        cont->SetCode(_reg->New<Array>());
        stack.push_back(cont);  // Use direct stack manipulation
        TranslateNode(node);
        stack.pop_back();
        return cont;
    }

   private:
    void TranslateToken(AstNodePtr node);
    void TranslateFunction(AstNodePtr node);
    void TranslateBlock(AstNodePtr node);
    void TranslateBinaryOp(AstNodePtr node, Operation::Type);
    void TranslateCall(AstNodePtr node);
    void TranslatePathname(AstNodePtr node);
    void TranslateIndex(AstNodePtr node);
    void TranslateIf(AstNodePtr node);
    void TranslateWhile(AstNodePtr node);
    void TranslateDoWhile(AstNodePtr node);
};

KAI_END
