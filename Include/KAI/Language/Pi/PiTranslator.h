#pragma once

#include <KAI/Executor/Executor.h>
#include <KAI/Language/Common/TranslatorBase.h>
#include <KAI/Language/Pi/PiAstNode.h>
#include <KAI/Language/Pi/PiLexer.h>
#include <KAI/Language/Pi/PiParser.h>
#include <KAI/Language/Pi/PiToken.h>

KAI_BEGIN

class PiTranslator : public TranslatorBase<PiParser> {
   public:
    typedef TranslatorBase<PiParser> Parent;
    typedef typename Parent::TokenNode TokenNode;
    typedef typename Parent::AstNode AstNode;
    typedef typename Parent::AstNodePtr AstNodePtr;

    PiTranslator() = delete;
    PiTranslator(Registry& r) : Parent(r) {}
    virtual ~PiTranslator();

    // Override base class methods to improve type handling
    virtual Pointer<Continuation> Result() override { return stack.front(); }

    // Override to make Pi handle direct evaluation of expressions
    // This addresses the issue where expressions lose type information
    [[nodiscard]] Pointer<Continuation> Translate(const char* text,
                                                  Structure st) override;

   protected:
    virtual void TranslateNode(AstNodePtr node) override;

   private:
    void AppendTokenised(const TokenNode& tok);
    void AppendDirectOperation(Pointer<Array> code, Operation::Type opType);
    bool isTranslatingRoot = false;
};

KAI_END
