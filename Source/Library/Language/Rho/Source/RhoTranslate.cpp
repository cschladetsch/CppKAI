#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;

KAI_BEGIN

// Override the Translate method to handle Rho language specific processing
Pointer<Continuation> RhoTranslator::Translate(const char *text, Structure st) {
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

    // This part is the crucial difference - we translate the AST node
    // and ensure it's not unnecessarily wrapped in continuations
    TranslateNode(parse->GetRoot());

    if (stack.empty()) {
        KAI_TRACE_ERROR() << "RhoTranslator::Translate: Stack is empty";
        return Object(); // Return empty object instead of throwing
    }

    auto cont = Pop();
    
    // For Rho, we want to make sure the cont we return has the type
    // information that indicates it's a Rho source, not a generic continuation
    cont->SetProperty("Language", "Rho");
    
    return cont;
}

KAI_END