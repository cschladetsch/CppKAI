#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>

// Use C++20/23 headers
#include <concepts>
#include <format>
#include <optional>
#include <ranges>
#include <string_view>

KAI_BEGIN

/// Implementation of the TranslateNode method that processes AST nodes
void RhoTranslator::TranslateNode(AstNodePtr node) {
    if (!node) {
        KAI_TRACE_ERROR() << "NULL node in TranslateNode";
        return;
    }

    KAI_TRACE() << "Processing node: " << node->ToString();

    // Only handle the minimal necessary cases for the basic functionality
    switch (node->GetType()) {
        case AstNodeEnum::None:
            KAI_TRACE() << "Empty node encountered, skipping";
            break;

        case AstNodeEnum::Program:
            KAI_TRACE() << "Processing Program node";
            for (const auto& child : node->GetChildren()) {
                TranslateNode(child);
            }
            break;

        case AstNodeEnum::TokenType:
            KAI_TRACE() << "Processing token node: "
                        << RhoTokenEnumType::ToString(node->GetToken().type);
            TranslateToken(node);
            break;

        case AstNodeEnum::DoWhile:
            KAI_TRACE() << "Processing DoWhile node";
            TranslateDoWhile(node);
            break;

        case AstNodeEnum::While:
            KAI_TRACE() << "Processing While node";
            TranslateWhile(node);
            break;

        case AstNodeEnum::For:
            KAI_TRACE() << "Processing For node";
            TranslateFor(node);
            break;

        case AstNodeEnum::ForEach:
            KAI_TRACE() << "Processing ForEach node";
            TranslateForEach(node);
            break;

        case AstNodeEnum::Conditional:
            KAI_TRACE() << "Processing If node";
            TranslateIf(node);
            break;

        case AstNodeEnum::List:
            KAI_TRACE() << "Processing List node";
            TranslateList(node);
            break;

        case AstNodeEnum::IndexOp:
            KAI_TRACE() << "Processing IndexOp node";
            TranslateIndex(node);
            break;

        case AstNodeEnum::Map:
            KAI_TRACE() << "Processing Map node";
            TranslateMap(node);
            break;

        case AstNodeEnum::ToPiLang:
            KAI_TRACE() << "Processing ToPiLang node";
            TranslatePiBlock(node);
            break;

        case AstNodeEnum::Function:
            KAI_TRACE() << "Processing Function node";
            TranslateFunction(node);
            // Don't process children - they're handled by TranslateFunction
            return;

        case AstNodeEnum::Call:
            KAI_TRACE() << "Processing Call node";
            TranslateCall(node);
            break;

        case AstNodeEnum::Block:
            KAI_TRACE() << "Processing Block node";
            TranslateBlock(node);
            break;

        case AstNodeEnum::Assignment:
            KAI_TRACE() << "Processing Assignment node";
            TranslateNode(node->GetChild(1));  // Value
            TranslateNode(node->GetChild(0));  // Target
            AppendDirectOperation(Operation::Store);
            break;

        case AstNodeEnum::Ident:
            KAI_TRACE() << "Processing Ident node";
            TranslateToken(node);
            break;

        case AstNodeEnum::GetMember:
            KAI_TRACE() << "Processing GetMember node";
            // GetMember nodes have: object and member name
            if (node->GetChildren().size() >= 2) {
                TranslateNode(node->GetChild(0));  // Object
                TranslateNode(node->GetChild(1));  // Member name
                AppendDirectOperation(Operation::GetChild);
            }
            break;

        case AstNodeEnum::ArgList:
            KAI_TRACE() << "Processing ArgList node";
            // ArgList is just a container for arguments, process all children
            for (const auto& child : node->GetChildren()) {
                TranslateNode(child);
            }
            break;

        default:
            // Log warning about unhandled node type but continue
            KAI_TRACE() << "Node type not fully implemented: "
                        << RhoAstNodeEnumType::ToString(node->GetType());

            // Recursively process child nodes so we don't completely halt
            for (const auto& child : node->GetChildren()) {
                TranslateNode(child);
            }
            break;
    }
}

/// Implementation of the Translate method using C++20/23 features.
/// This method parses Rho language text and converts it to a Continuation
/// object.
Pointer<Continuation> RhoTranslator::Translate(const char* text, Structure st) {
    // Validate input using modern C++ idioms
    if (text == nullptr || *text == '\0') {
        KAI_TRACE_WARN_1("No input text provided");
        return Object();  // Return empty Object for consistency with base
                          // implementation
    }

    trace = 5;  // Increase trace level for debugging

    KAI_TRACE() << "Executing text: " << text;

    // Use modern shared_ptr for lexical analysis
    auto lex = std::make_shared<Lexer>(text, *reg_);
    lex->Process();

    // Check if lexer produced any tokens
    if (lex->GetTokens().empty()) {
        KAI_TRACE_WARN_1("No tokens were generated by lexer");
        return Object();
    }

    if (lex->Failed) {
        KAI_TRACE_WARN_1(lex->Error);
        Fail(lex->Error);
        return Object();
    }

    // Print tokens if trace is enabled
    if (trace > 0) {
        KAI_TRACE_1(lex->Print());
    }

    // Parse the tokens into an AST
    auto parse = std::make_shared<Parser>(*reg_);
    parse->Process(lex, st);

    if (parse->Failed) {
        if (trace > 1) {
            KAI_TRACE_1(parse->PrintTree());
        }
        Fail(parse->Error);
        return Object();
    }

    // Print parse tree if trace level > 1
    if (trace > 1) {
        KAI_TRACE_1(parse->PrintTree());
    }

    // Create a new continuation to hold the translated code
    PushNew();

    // Translate the AST to a continuation
    KAI_TRACE() << "Starting to process tokens";
    TranslateNode(parse->GetRoot());

    // Check for errors in translation
    if (stack.empty()) {
        KAI_TRACE_ERROR() << "RhoTranslator::Translate: Stack is empty";
        return Object();
    }

    // Get the resulting continuation
    auto cont = Pop();

    // Analyze the continuation if it exists
    if (cont.Exists() && cont->GetCode().Exists()) {
        KAI_TRACE() << std::format("Rho translated code has {} elements",
                                   static_cast<int>(cont->GetCode()->Size()));
    }

    return cont;
}

KAI_END