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
    
    // Handle the special case where we have a simple binary operation
    // that has been translated into literal values followed by an operation
    // For example: "2 + 3" -> [2, 3, Plus]
    if (cont->GetCode().Exists() && (cont->GetCode()->Size() == 3)) {
        Object first = cont->GetCode()->At(0);
        Object second = cont->GetCode()->At(1);
        Object third = cont->GetCode()->At(2);
        
        // Check if this is a binary operation pattern: value value operation
        if (third.GetTypeNumber() == Type::Number::Operation) {
            // Check if the values are direct numbers (not complex expressions)
            if ((first.GetTypeNumber() == Type::Number::Signed32 || 
                 first.GetTypeNumber() == Type::Number::Single) &&
                (second.GetTypeNumber() == Type::Number::Signed32 || 
                 second.GetTypeNumber() == Type::Number::Single)) {
                
                // We have a simple binary operation with direct values
                // This is already in the correct format for direct execution
                // No need to modify it
                KAI_TRACE() << "Detected simple binary operation pattern in Rho translation";
            }
        }
    }
    
    return cont;
}

KAI_END