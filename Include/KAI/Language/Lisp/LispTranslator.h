#pragma once

#include <KAI/Language/Common/TranslatorBase.h>
#include <KAI/Language/Lisp/LispParser.h>

KAI_BEGIN

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
    void TranslateToken(AstNodePtr node);
    void TranslateFunction(AstNodePtr node);
    void TranslateBlock(AstNodePtr node);
    void TranslateBinaryOp(AstNodePtr node, Operation::Type);
    void TranslateCall(AstNodePtr node);
    void TranslatePathname(AstNodePtr node);
    void TranslateIndex(AstNodePtr node);
    void TranslateIf(AstNodePtr node);
    void TranslateFor(AstNodePtr node);
    void TranslateWhile(AstNodePtr node);
};

KAI_END
