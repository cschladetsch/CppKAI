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

    virtual Pointer<Continuation> Translate(const char *text, Structure st) override {
        if (text == 0 || text[0] == 0) {
            KAI_TRACE_WARN_1("No input");
            return Object();
        }

        trace = 0;

        auto lex = std::make_shared<Lexer>(text, *reg_);
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

        auto parse = std::make_shared<Parser>(*reg_);
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

        // If the continuation contains a single value that's not a complex type,
        // we can return it directly for efficiency, but only if the value is valid
        if (cont.Exists() && cont->GetCode().Exists() && cont->GetCode()->Size() == 1) {
            Object value = cont->GetCode()->At(0);
            
            // Check if the value is valid and not a complex type
            if (value.Valid() && value.Exists() && 
                value.GetTypeNumber() != Type::Number::Continuation &&
                value.GetTypeNumber() != Type::Number::Operation) {
                
                KAI_TRACE() << "TranslatorBase: Returning direct single value: " 
                          << value.ToString() << " (type: " << value.GetClass()->GetName() << ")";
                
                return value;
            }
        }

        // For more complex cases, return the continuation for evaluation
        return cont;
    }
    
    // Helper method for loop-related continuation creation
    [[nodiscard]] Pointer<Continuation> CreateContinuationAndTranslate(AstNodePtr node) {
        // Create a new continuation for the code block
        PushNew();
        
        // Translate the node into the continuation
        TranslateNode(node);
        
        // Get the resulting continuation
        return Pop();
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
