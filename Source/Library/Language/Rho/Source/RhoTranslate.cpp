#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>
#include <KAI/Language/Pi/PiTranslator.h>

// Use C++20/23 headers
#include <concepts>
#include <format>
#include <iostream>
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
        return Object();
    }

    trace = 5;  // Increase trace level for debugging
    KAI_TRACE() << "RhoTranslator::Translate called with text: " << text;

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

    // Rho always transpiles to Pi - simple and direct
    std::string piCode = TranspileToPi(parse->GetRoot());
    
    std::cout << "\n=== TRANSPILATION DEBUG ===\n";
    std::cout << "Input Rho: " << text << "\n";
    std::cout << "Generated Pi: '" << piCode << "'\n";
    std::cout << "Failed: " << (Failed ? "true" : "false") << "\n";
    std::cout << "=== END DEBUG ===\n\n";
    
    if (!piCode.empty() && !Failed) {
        KAI_TRACE() << "Using Pi transpilation: " << piCode;
        
        // Use PiTranslator to convert Pi code to continuation
        PiTranslator piTranslator(*reg_);
        piTranslator.trace = trace;
        
        auto result = piTranslator.Translate(piCode.c_str(), st);
        
        if (!piTranslator.Failed) {
            return result;
        }
        
        KAI_TRACE() << "Pi transpilation failed";
        Fail("Pi transpilation failed");
        return Object();
    }
    
    // If we get here, Pi code was empty or Failed was set
    Fail("Unable to transpile Rho to Pi");
    return Object();
}

/// Transpile Rho AST to Pi code string
std::string RhoTranslator::TranspileToPi(AstNodePtr node) {
    if (!node) {
        Fail("TranspileToPi: NULL node");
        return "";
    }
    
    KAI_TRACE() << "Transpiling node to Pi: " << node->ToString();
    return TranspileNodeToPi(node);
}

/// Recursively transpile AST nodes to Pi code
std::string RhoTranslator::TranspileNodeToPi(AstNodePtr node) {
    if (!node) {
        return "";
    }

    KAI_TRACE() << "TranspileNodeToPi: Processing node type " << RhoAstNodeEnumType::ToString(node->GetType());
    
    // Debug: print node structure
    std::cout << "  Node type: " << RhoAstNodeEnumType::ToString(node->GetType()) << ", Children: " << node->GetChildren().size();
    if (node->GetType() == AstNodeEnum::TokenType) {
        std::cout << ", Token: " << RhoTokenEnumType::ToString(node->GetToken().type);
    }
    std::cout << "\n";

    switch (node->GetType()) {
        case AstNodeEnum::Program: {
            std::string result;
            for (const auto& child : node->GetChildren()) {
                std::string childCode = TranspileNodeToPi(child);
                if (!childCode.empty()) {
                    if (!result.empty()) result += " ";
                    result += childCode;
                }
            }
            return result;
        }
        
        case AstNodeEnum::TokenType: {
            auto token = node->GetToken();
            std::cout << "    Processing TokenType: " << RhoTokenEnumType::ToString(token.type) << "\n";
            switch (token.type) {
                case RhoTokenEnumType::Int:
                case RhoTokenEnumType::Float:
                    return token.Text();
                    
                case RhoTokenEnumType::String:
                    return "\"" + token.Text() + "\"";
                    
                case RhoTokenEnumType::True:
                    return "true";
                    
                case RhoTokenEnumType::False:
                    return "false";
                    
                case RhoTokenEnumType::Label:
                    // Variables are retrieved by name in Pi
                    return token.Text();
                    
                case RhoTokenEnumType::Return:
                    // In Pi, the last value on stack is automatically returned
                    // So we just process the return value and leave it on stack
                    if (!node->GetChildren().empty()) {
                        return TranspileNodeToPi(node->GetChild(0));
                    }
                    return "";

                case RhoTokenEnumType::Break:
                    // Break resumes the previous continuation (exits loop)
                    // Just resume without pushing to stack
                    return "resume";

                case RhoTokenEnumType::Continue:
                    // Continue pushes itself to stack first, then resumes
                    // This allows the loop to continue as a subroutine
                    return "self resume";

                // Handle assignment
                case RhoTokenEnumType::Assign: {
                    // Rho: var = value  ->  Pi: value 'var #
                    if (node->GetChildren().size() >= 2) {
                        std::string target = node->GetChild(1)->GetTokenText(); // variable name
                        std::string value = TranspileNodeToPi(node->GetChild(0));
                        std::cout << "    Assignment: '" << value << " '" << target << " #'\n";
                        return value + " '" + target + " #";
                    }
                    return "";
                }
                
                // Handle binary operators
                case RhoTokenEnumType::Plus:
                case RhoTokenEnumType::Minus:
                case RhoTokenEnumType::Mul:
                case RhoTokenEnumType::Divide:
                case RhoTokenEnumType::Less:
                case RhoTokenEnumType::Greater:
                case RhoTokenEnumType::LessEquiv:
                case RhoTokenEnumType::GreaterEquiv:
                case RhoTokenEnumType::Equiv: {
                    std::string op = "";
                    switch (token.type) {
                        case RhoTokenEnumType::Plus: op = "+"; break;
                        case RhoTokenEnumType::Minus: op = "-"; break;
                        case RhoTokenEnumType::Mul: op = "*"; break;
                        case RhoTokenEnumType::Divide: op = "/"; break;
                        case RhoTokenEnumType::Less: op = "<"; break;
                        case RhoTokenEnumType::Greater: op = ">"; break;
                        case RhoTokenEnumType::LessEquiv: op = "<="; break;
                        case RhoTokenEnumType::GreaterEquiv: op = ">="; break;
                        case RhoTokenEnumType::Equiv: op = "=="; break;
                        default: break;
                    }
                    
                    if (!op.empty() && node->GetChildren().size() >= 2) {
                        // Binary operation: left right op
                        std::string left = TranspileNodeToPi(node->GetChild(0));
                        std::string right = TranspileNodeToPi(node->GetChild(1));
                        std::cout << "    Binary op: left='" << left << "', right='" << right << "', op='" << op << "'\n";
                        return left + " " + right + " " + op;
                    }
                    break;
                }
                
                default:
                    std::cout << "    TokenType default case: " << RhoTokenEnumType::ToString(token.type) << ", children: " << node->GetChildren().size() << "\n";
                    return token.Text();
            }
        }
        
        case AstNodeEnum::Assignment: {
            // Rho: var = value  ->  Pi: value 'var # (# is the store operator)
            if (node->GetChildren().size() >= 2) {
                std::string target = node->GetChild(1)->GetTokenText(); // variable name
                std::string value = TranspileNodeToPi(node->GetChild(0));
                return value + " '" + target + " #";
            }
            return "";
        }
        
        case AstNodeEnum::Function: {
            // Transpile all functions to Pi
            if (node->GetChildren().size() >= 3) {
                std::string name = node->GetChild(0)->GetTokenText();
                auto argsNode = node->GetChild(1);
                auto bodyNode = node->GetChild(2);
                
                std::string piCode = "{ ";
                
                // Generate parameter storage code (reverse order for stack)
                // Stack is LIFO: args are pushed left-to-right, so rightmost is on top
                auto args = argsNode->GetChildren();
                for (int i = args.size() - 1; i >= 0; i--) {
                    std::string paramName = args[i]->GetTokenText();
                    piCode += "'" + paramName + " # ";
                }
                
                // Generate body code
                std::string bodyCode = TranspileNodeToPi(bodyNode);
                piCode += bodyCode + " }";
                
                // Store the function (# is the store operator)
                return piCode + " '" + name + " #";
            }
            return "";
        }
        
        case AstNodeEnum::Call: {
            // Rho: func(args)  ->  Pi: args... 'func retrieve suspend
            if (!node->GetChildren().empty()) {
                std::string funcName = node->GetChild(0)->GetTokenText();
                std::string piCode;
                
                // Add arguments
                if (node->GetChildren().size() > 1) {
                    auto argsNode = node->GetChild(1);
                    if (argsNode->GetType() == AstNodeEnum::ArgList) {
                        for (const auto& arg : argsNode->GetChildren()) {
                            if (!piCode.empty()) piCode += " ";
                            piCode += TranspileNodeToPi(arg);
                        }
                    }
                }
                
                // Add function call (retrieve function and execute with &)
                if (!piCode.empty()) piCode += " ";
                piCode += funcName + " &";
                
                return piCode;
            }
            return "";
        }
        
        case AstNodeEnum::Block: {
            std::string result;
            for (const auto& child : node->GetChildren()) {
                std::string childCode = TranspileNodeToPi(child);
                if (!childCode.empty()) {
                    if (!result.empty()) result += " ";
                    result += childCode;
                }
            }
            return result;
        }
        
        case AstNodeEnum::DoWhile: {
            // Transpile do-while loops to Pi
            // Pi doesn't have do-while, so simulate: body { condition body } while
            // This executes body once, then while(condition) { body }
            if (node->GetChildren().size() >= 2) {
                std::string bodyCode = TranspileNodeToPi(node->GetChild(0));
                std::string conditionCode = TranspileNodeToPi(node->GetChild(1));

                if (bodyCode.empty() || conditionCode.empty()) {
                    return "";
                }

                // Execute body once, then while loop with condition and body
                return bodyCode + " { " + conditionCode + " } { " + bodyCode + " } while";
            }
            return "";
        }

        case AstNodeEnum::While: {
            // Try to transpile simple while loops to Pi
            // For complex cases, this will return empty and trigger fallback
            if (node->GetChildren().size() >= 2) {
                std::string condition = "{ " + TranspileNodeToPi(node->GetChild(0)) + " }";
                std::string body = "{ " + TranspileNodeToPi(node->GetChild(1)) + " }";

                // If either condition or body couldn't be transpiled, return empty for fallback
                if (condition == "{ }" || body == "{ }") {
                    return "";
                }

                return condition + " " + body + " while";
            }
            return "";
        }

        case AstNodeEnum::For: {
            // Transpile for loops to Pi
            // Rho: for (init; cond; step) { body }
            // Pi: init { cond } { body step } while
            if (node->GetChildren().size() >= 4) {
                std::string init = TranspileNodeToPi(node->GetChild(0));
                std::string condition = "{ " + TranspileNodeToPi(node->GetChild(1)) + " }";
                std::string step = TranspileNodeToPi(node->GetChild(2));
                std::string body = TranspileNodeToPi(node->GetChild(3));

                // Combine body and step
                std::string bodyWithStep = "{ " + body;
                if (!step.empty()) {
                    bodyWithStep += " " + step;
                }
                bodyWithStep += " }";

                std::string result;
                if (!init.empty()) {
                    result = init + " ";
                }
                result += condition + " " + bodyWithStep + " while";

                return result;
            }
            return "";
        }

        case AstNodeEnum::ForEach: {
            // Transpile foreach loops to Pi
            // Rho: foreach (item in collection) { body }
            // Pi: collection { 'item # body } foreach
            if (node->GetChildren().size() >= 3) {
                std::string varName = node->GetChild(0)->GetTokenText();
                std::string collection = TranspileNodeToPi(node->GetChild(1));
                std::string body = TranspileNodeToPi(node->GetChild(2));

                std::string piCode = collection + " { '" + varName + " # " + body + " } foreach";
                return piCode;
            }
            return "";
        }

        case AstNodeEnum::Conditional: {
            // Transpile if-else to Pi
            // Pi syntax: condition '{ then-block } '{ else-block } ife
            // or: condition '{ then-block } if
            // The blocks need to be quoted so they are continuations that inherit scope
            if (node->GetChildren().size() >= 2) {
                std::string condition = TranspileNodeToPi(node->GetChild(0));
                std::string thenBlock = "'{ " + TranspileNodeToPi(node->GetChild(1)) + " }";

                if (node->GetChildren().size() > 2) {
                    // Has else block
                    std::string elseBlock = "'{ " + TranspileNodeToPi(node->GetChild(2)) + " }";
                    return condition + " " + thenBlock + " " + elseBlock + " ife";
                } else {
                    // No else block
                    return condition + " " + thenBlock + " if";
                }
            }
            return "";
        }

        default: {
            
            // For unhandled nodes, try to process children
            std::string result;
            for (const auto& child : node->GetChildren()) {
                std::string childCode = TranspileNodeToPi(child);
                if (!childCode.empty()) {
                    if (!result.empty()) result += " ";
                    result += childCode;
                }
            }
            return result;
        }
    }
}


KAI_END