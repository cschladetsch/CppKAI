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

   protected:
    virtual void TranslateNode(AstNodePtr node) override;
    using Parent::_reg;

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
    void TranslateForEachNetwork(AstNodePtr node);
    void TranslateWhile(AstNodePtr node);
};

KAI_END
