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

    // Create a new continuation to hold the translated code
    PushNew();

    // Translate the AST node
    KAI_TRACE() << "Starting to process tokens";
    TranslateNode(parse->GetRoot());

    if (stack.empty()) {
        KAI_TRACE_ERROR() << "RhoTranslator::Translate: Stack is empty";
        return Object(); // Return empty object instead of throwing
    }

    // Get the resulting continuation
    auto cont = Pop();
    
    // In the future, we may want to add auto-execution of simple expressions here
    // For now, just log what we've found to help with debugging
    if (cont.Exists() && cont->GetCode().Exists()) {
        int size = static_cast<int>(cont->GetCode()->Size());
        KAI_TRACE() << "Rho translated code has " << size << " elements";
        
        // Special detection of binary operations (pattern: value value operation)
        if (size == 3) {
            Object third = cont->GetCode()->At(2);
            if (third.GetTypeNumber() == Type::Number::Operation) {
                KAI_TRACE() << "Detected possible binary operation pattern in code array";
                
                // For now we're not modifying the continuation, just returning it
                // We could try to evaluate it here but that needs more work
            }
        }
    }
    
    return cont;
}

KAI_END