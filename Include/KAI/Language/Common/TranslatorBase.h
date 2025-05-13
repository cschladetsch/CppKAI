#pragma once

#include <KAI/Language/Common/TranslatorCommon.h>

KAI_BEGIN

template <class EParser>
struct TranslatorBase : TranslatorCommon {
    typedef EParser Parser;
    typedef typename Parser::TokenNode TokenNode;
    typedef typename TokenNode::Enum TokenEnum;
    typedef typename Parser::Lexer Lexer;
    typedef typename Parser::AstNode AstNode;
    typedef typename AstNode::Enum AstEnum;
    typedef typename Parser::AstNodePtr AstNodePtr;

    TranslatorBase(const TranslatorBase &) = delete;
    TranslatorBase(Registry &reg) : TranslatorCommon(reg) {}

    Pointer<Continuation> Translate(const char *text, Structure st) override {
        if (text == 0 || text[0] == 0) {
            KAI_TRACE_WARN_1("No input");
            return Object();
        }

        trace = 0;

        auto lex = std::make_shared<Lexer>(text, *_reg);
        lex->Process();
        if (lex->GetTokens().empty()) {
            KAI_TRACE_WARN_1("No tokens");
            return Object();
        }

        if (lex->Failed) {
            KAI_TRACE_WARN_1(lex->Error);
            Fail(lex->Error);
            return Object();
        }

        if (trace > 0) KAI_TRACE_1(lex->Print());

        auto parse = std::make_shared<Parser>(*_reg);
        parse->Process(lex, st);
        if (parse->Failed) {
            if (trace > 1) KAI_TRACE_1(parse->PrintTree());

            Fail(parse->Error);
            return Object();
        }

        if (trace > 1) KAI_TRACE_1(parse->PrintTree());

        PushNew();

        TranslateNode(parse->GetRoot());

        if (stack.empty()) KAI_THROW_0(EmptyStack);

        auto cont = Pop();

        // Return the full continuation
        // The Console.Execute method is now enhanced to handle both Pi and Rho languages
        // It will properly evaluate operations and results according to the language
        return cont;
    }

   protected:
    virtual void TranslateNode(AstNodePtr node) = 0;

    void Run(std::shared_ptr<Parser> p) {
        PushNew();

        try {
            TranslateNode(p);
        } catch (Exception &) {
            if (!Failed) Fail("Failed");
        }
    }
};

KAI_END
