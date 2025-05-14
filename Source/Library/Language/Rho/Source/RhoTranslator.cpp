#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <format>
#include <ranges>
#include <concepts>
#include <stdexcept>

KAI_BEGIN

// Note: The RhoTranslator::Translate method is now implemented in RhoTranslate.cpp
// to avoid duplicate implementation errors

void RhoTranslator::TranslateToken(AstNodePtr node) {
    KAI_TRACE() << std::format("TranslateToken: {}", 
                RhoTokenEnumType::ToString(node->GetToken().type));
    KAI_TRACE() << std::format("TranslateToken with text: {}", node->Text());

    switch (node->GetToken().type) {
        case TokenEnum::OpenParan:
            for (const auto& ch : node->GetChildren()) {
                TranslateNode(ch);
            }
            return;

        case TokenEnum::Not:
            // Create a continuation for this unary operation
            {
                // Create a new continuation block
                PushNew();
                
                // Translate the operand
                TranslateNode(node->GetChild(0));
                
                // Add the operation
                AppendDirectOperation(Operation::LogicalNot);
                
                // Pop and append to parent
                Append(Pop());
            }
            return;

        case TokenEnum::True:
            // Create an actual boolean true value
            Append(reg_->New<bool>(true));
            return;

        case TokenEnum::False:
            // Create an actual boolean false value
            Append(reg_->New<bool>(false));
            return;

        case TokenEnum::Assert:
            // Create a continuation for assert operation
            {
                // Create a new continuation block
                PushNew();
                
                // Translate the assertion condition
                TranslateNode(node->GetChild(0));
                
                // Add the operation
                AppendDirectOperation(Operation::Assert);
                
                // Pop and append to parent
                Append(Pop());
            }
            return;

        case TokenEnum::DivAssign:
            TranslateBinaryOp(node, Operation::DivEquals);
            return;

        case TokenEnum::MulAssign:
            TranslateBinaryOp(node, Operation::MulEquals);
            return;

        case TokenEnum::MinusAssign:
            TranslateBinaryOp(node, Operation::MinusEquals);
            return;

        case TokenEnum::PlusAssign:
            TranslateBinaryOp(node, Operation::PlusEquals);
            return;

        case TokenEnum::Assign:
            TranslateBinaryOp(node, Operation::Store);
            return;

        case TokenEnum::Lookup:
            AppendDirectOperation(Operation::Lookup);
            return;

        case TokenEnum::Self:
            AppendDirectOperation(Operation::This);
            return;

        case TokenEnum::NotEquiv:
            TranslateBinaryOp(node, Operation::NotEquiv);
            return;

        case TokenEnum::Equiv:
            TranslateBinaryOp(node, Operation::Equiv);
            return;

        case TokenEnum::Less:
            TranslateBinaryOp(node, Operation::Less);
            return;

        case TokenEnum::Greater:
            TranslateBinaryOp(node, Operation::Greater);
            return;

        case TokenEnum::GreaterEquiv:
            TranslateBinaryOp(node, Operation::GreaterOrEquiv);
            return;

        case TokenEnum::LessEquiv:
            TranslateBinaryOp(node, Operation::LessOrEquiv);
            return;

        case TokenEnum::Minus:
            TranslateBinaryOp(node, Operation::Minus);
            return;

        case TokenEnum::Plus:
            KAI_TRACE() << "Translating Plus operation";
            TranslateBinaryOp(node, Operation::Plus);
            return;

        case TokenEnum::Mul:
            TranslateBinaryOp(node, Operation::Multiply);
            return;

        case TokenEnum::Divide:
            TranslateBinaryOp(node, Operation::Divide);
            return;

        case TokenEnum::Mod:
            TranslateBinaryOp(node, Operation::Modulo);
            return;

        case TokenEnum::Or:
            TranslateBinaryOp(node, Operation::LogicalOr);
            return;

        case TokenEnum::And:
            TranslateBinaryOp(node, Operation::LogicalAnd);
            return;

        case TokenEnum::Int: {
            KAI_TRACE() << std::format("Translating Int: {}", node->GetTokenText());
            try {
                const auto value = std::stoi(node->GetTokenText());
                Append(reg_->New<int>(value));
            } catch (const std::exception& e) {
                Fail(std::format("Failed to parse integer: {}", e.what()));
            }
            return;
        }

        case TokenEnum::Float: {
            KAI_TRACE() << std::format("Translating Float: {}", node->GetTokenText());
            try {
                const auto value = std::stof(node->GetTokenText());
                Append(reg_->New<float>(value));
            } catch (const std::exception& e) {
                Fail(std::format("Failed to parse float: {}", e.what()));
            }
            return;
        }

        case TokenEnum::String:
            KAI_TRACE() << std::format("Translating String: {}", node->Text());
            Append(reg_->New<String>(String(node->Text())));
            KAI_TRACE() << "String translation complete";
            return;

        case TokenEnum::Ident:
            KAI_TRACE() << std::format("Translating Ident: {}", node->Text());
            Append(reg_->New<Label>(Label(node->Text())));
            KAI_TRACE() << "Ident translation complete";
            return;

        case TokenEnum::Pathname:
            KAI_TRACE() << std::format("Translating Pathname: {}", node->Text());
            Append(reg_->New<Pathname>(Pathname(node->Text())));
            return;

        case TokenEnum::ToPi:
            AppendDirectOperation(Operation::ToPi);
            return;
            
        case TokenEnum::PiSequence:
        {
            KAI_TRACE() << std::format("Translating PiSequence: {}", node->Text());
            // Create a continuation for the Pi code block
            PushNew();
            
            // Translate all the children nodes
            for (const auto& child : node->GetChildren()) {
                TranslateNode(child);
            }
            
            // Get the continuation and convert it to Pi
            auto piCont = Pop();
            // Note: We're not setting a Language property anymore since it's not registered
            // in the Continuation class. Language context is now handled in Console.cpp.
            
            // Add the continuation to the parent
            Append(piCont);
            return;
        }

        case TokenEnum::Yield:
            // Modern implementation would use coroutines with co_yield
            KAI_NOT_IMPLEMENTED();
            return;

        case TokenEnum::Return:
            for (const auto& ch : node->GetChildren()) {
                TranslateNode(ch);
            }
            AppendDirectOperation(Operation::Return);
            return;
    }

    Fail(std::format("Unsupported node {}", node->ToString()));
    KAI_TRACE_ERROR() << "Error: " << Error;
    KAI_NOT_IMPLEMENTED();
}

void RhoTranslator::TranslateBinaryOp(AstNodePtr node, Operation::Type op) {
    KAI_TRACE() << std::format("TranslateBinaryOp: Operation={}", 
                 Operation::ToString(op));

    // For binary operations, we need to ensure we're translating directly to Pi streams
    // without introducing unnecessary continuations
    
    // First, check if both children are simple integer literals
    // In such case, we could evaluate the operation at translation time
    bool canEvaluateNow = false;
    int leftValue = 0, rightValue = 0;
    
    // Use structured bindings for cleaner code
    if (const auto [leftChild, rightChild] = std::make_tuple(node->GetChild(0), node->GetChild(1)); 
        leftChild->GetType() == AstNodeEnum::TokenType && 
        rightChild->GetType() == AstNodeEnum::TokenType) {
        
        const auto& leftToken = leftChild->GetToken();
        const auto& rightToken = rightChild->GetToken();
        
        // Check if both tokens are integers
        if (leftToken.type == TokenEnum::Int && rightToken.type == TokenEnum::Int) {
            try {
                leftValue = std::stoi(leftChild->GetTokenText());
                rightValue = std::stoi(rightChild->GetTokenText());
                canEvaluateNow = true;
            } catch (...) {
                canEvaluateNow = false;
            }
        }
    }
    
    // If we can evaluate the operation now, do it directly to avoid wrapping
    if (canEvaluateNow) {
        int result = 0;
        bool validOp = true;
        
        // Evaluate the operation
        switch (op) {
            case Operation::Plus:
                result = leftValue + rightValue;
                break;
            case Operation::Minus:
                result = leftValue - rightValue;
                break;
            case Operation::Multiply:
                result = leftValue * rightValue;
                break;
            case Operation::Divide:
                if (rightValue != 0) {
                    result = leftValue / rightValue;
                } else {
                    validOp = false;  // Cannot divide by zero
                }
                break;
            case Operation::Modulo:
                if (rightValue != 0) {
                    result = leftValue % rightValue;
                } else {
                    validOp = false;  // Cannot modulo by zero
                }
                break;
            default:
                // For other operations, we'll use the general approach
                validOp = false;
                break;
        }
        
        if (validOp) {
            // Push the result directly as an integer
            KAI_TRACE() << std::format("Direct evaluation of binary op: {} {} {} = {}", 
                         leftValue, Operation::ToString(op), rightValue, result);
            Append(reg_->New<int>(result));
            return;
        }
    }
    
    // Translate directly to Pi stream without creating a continuation
    // This is the key change - we want to directly translate binary operations to Pi
    // First translate the left operand
    TranslateNode(node->GetChild(0));
    
    // Then translate the right operand
    TranslateNode(node->GetChild(1));
    
    // Finally, add the operation directly
    AppendDirectOperation(op);

    KAI_TRACE() << "Binary operation successfully translated to Pi stream";
}

void RhoTranslator::TranslateNode(AstNodePtr node) {
    if (!node) {
        Failed = true;
        return;
    }

    KAI_TRACE() << std::format("TranslateNode: Type={} Text={}",
                RhoAstNodeEnumType::ToString(node->GetType()),
                node->Text());

    switch (node->GetType()) {
        case AstEnum::Pathname:
            TranslateToken(node);
            return;

        case AstEnum::IndexOp:
            TranslateBinaryOp(node, Operation::Index);
            return;

        case AstEnum::GetMember:
            // Translate directly to Pi stream without creating a continuation
            // First translate the object (pushes it onto the stack)
            TranslateNode(node->GetChild(0));
            
            // Then translate the property name (pushes it onto the stack)
            TranslateNode(node->GetChild(1));
            
            // Add the GetProperty operation directly
            AppendDirectOperation(Operation::GetProperty);
            return;

        case AstEnum::TokenType: {
            // Special handling for Pi with braces (handle the case where pi + { are separate tokens)
            if (node->GetToken().type == TokenEnum::ToPi) {
                // Since we can't access the parent directly, we'll handle Pi blocks
                // by looking at the next token in the parse tree as needed
                
                // For Pi blocks, we'll rely on the token sequence during parsing
                // where we should have already created the appropriate structure using
                // the PiSequence token type that's generated by the parser.
                
                // Just translate the ToPi token as normal
                AppendDirectOperation(Operation::ToPi);
            }
            
            // Regular token handling
            TranslateToken(node);
            return;
        }

        case AstEnum::Assignment:
            KAI_TRACE() << "Translating Assignment";
            try {
                // Basic validation
                if (node->GetChildren().size() < 2) {
                    KAI_TRACE_ERROR() << "Assignment node has fewer than 2 children";
                    Fail("Assignment node has fewer than 2 children");
                    return;
                }

                // Value first (right side)
                TranslateNode(node->GetChild(1));

                // Then target (left side)
                TranslateNode(node->GetChild(0));

                // Add the store operation directly
                AppendDirectOperation(Operation::Store);

                KAI_TRACE() << "Completed assignment translation without Continuations";
            } catch (const std::exception &e) {
                KAI_TRACE_ERROR() << std::format("Exception in assignment: {}", e.what());
                Fail(std::format("Assignment failed: {}", e.what()));
            } catch (...) {
                KAI_TRACE_ERROR() << "Unknown exception in assignment";
                Fail("Assignment failed with unknown exception");
            }
            return;

        case AstEnum::Call:
            TranslateCall(node);
            return;

        case AstEnum::Conditional:
            TranslateIf(node);
            return;

        case AstEnum::While:
            TranslateWhile(node);
            return;

        case AstEnum::Block:
            // Use C++20 ranges for cleaner iteration
            for (const auto& st : node->GetChildren() | std::views::all) {
                TranslateNode(st);
            }
            return;

        case AstEnum::List:
            TranslateList(node);
            return;

        case AstEnum::Function:
            TranslateFunction(node);
            return;

        case AstEnum::Program:
            // Use C++20 ranges for cleaner iteration
            for (const auto& e : node->GetChildren() | std::views::all) {
                TranslateNode(e);
            }
            return;
    }

    Fail(std::format("Unsupported node {}", node->ToString()));
    KAI_NOT_IMPLEMENTED();
}

void RhoTranslator::TranslateBlock(AstNodePtr node) {
    // Use C++20 ranges for cleaner iteration
    for (const auto& st : node->GetChildren() | std::views::all) {
        TranslateNode(st);
    }
}

void RhoTranslator::TranslateFunction(AstNodePtr node) {
    KAI_TRACE() << "TranslateFunction: Creating function continuation";
    
    // child 0: ident
    // child 1: args
    // child 2: block
    const auto& ch = node->GetChildren();

    // Create a continuation for the function body
    PushNew();
    
    // Get the current continuation (top of stack)
    Pointer<Continuation> cont = Top();
    
    // Process the function body (the block)
    if (ch.size() > 2) {
        KAI_TRACE() << std::format("Processing function body with {} statements", 
                     static_cast<int>(ch[2]->GetChildren().size()));
                    
        // Process each statement in the block
        for (const auto& b : ch[2]->GetChildren()) {
            // Special handling for return statements
            if (b->GetType() == AstEnum::TokenType &&
                b->GetToken().type == TokenEnum::Return) {
                
                // Process the return value if it exists
                if (b->GetChildren().size() > 0) {
                    TranslateNode(b->GetChild(0));
                }

                // Add the Return operation directly
                AppendDirectOperation(Operation::Return);
            } else {
                TranslateNode(b);
            }
        }
    }
    
    // Pop the continuation
    cont = Pop();
    
    // Add the arguments to the function using Deref to get a reference
    Continuation& contRef = Deref<Continuation>(cont);
    for (const auto& a : ch[1]->GetChildren()) {
        contRef.AddArg(Label(a->GetTokenText()));
    }

    // Store the function: push function object, push function name, store
    Append(cont);
    Append(New<Label>(Label(ch[0]->Text())));
    AppendDirectOperation(Operation::Store);
    
    KAI_TRACE() << "Function translation complete";
}

void RhoTranslator::TranslateCall(AstNodePtr node) {
    KAI_TRACE() << "Translating call";

    const auto& children = node->GetChildren();
    
    // Use C++20 ranges for cleaner iteration
    for (const auto& a : children[1]->GetChildren() | std::views::all) {
        TranslateNode(a);
    }

    TranslateNode(children[0]);

    // Use C++20 if constexpr for more readable code
    Operation::Type callOp;
    if (children.size() > 2 && children[2]->GetToken().type == TokenEnum::Replace) {
        callOp = Operation::Replace;
    } else {
        callOp = Operation::Suspend;
    }

    AppendDirectOperation(callOp);

    KAI_TRACE() << "Completed call translation";
}

void RhoTranslator::TranslateIf(AstNodePtr node) {
    KAI_TRACE() << "Translating if statement";

    const auto& ch = node->GetChildren();
    const bool hasElse = ch.size() > 2;

    // First translate the condition
    TranslateNode(ch[0]);

    // Create and translate the 'then' branch
    PushNew();
    TranslateNode(ch[1]);
    auto thenCont = Pop();

    // Handle optional else branch using C++20 idioms
    Pointer<Continuation> elseCont;
    if (hasElse) {
        PushNew();
        TranslateNode(ch[2]);
        elseCont = Pop();
    }

    // Add continuations and the appropriate if operation
    Append(thenCont);

    // Use conditional to determine the correct if operation
    Operation::Type ifOp;
    if (hasElse) {
        Append(elseCont);
        ifOp = Operation::IfThenSuspendElseSuspend;
    } else {
        ifOp = Operation::IfThenSuspend;
    }

    AppendDirectOperation(ifOp);

    KAI_TRACE() << "Completed if statement translation";
}

void RhoTranslator::TranslateWhile(AstNodePtr node) {
    try {
        KAI_TRACE() << "Translating while loop";

        // Verify we have enough children
        if (node->GetChildren().empty()) {
            KAI_TRACE_ERROR() << "Not enough children in While node";
            Fail("While node must have at least a condition child");
            return;
        }

        // Get condition and body nodes
        const auto condition = node->GetChild(0);

        // Define the body node - either use the existing one or create an empty block
        AstNodePtr bodyNode;
        if (node->GetChildren().size() > 1) {
            bodyNode = node->GetChild(1);
        } else {
            // Create an empty body block node using the proper AstNodePtr type
            // We need to ensure type compatibility
            auto emptyBlock = std::make_shared<RhoAstNode>(RhoAstNodeEnumType::Block);
            bodyNode = std::static_pointer_cast<AstNode>(emptyBlock);
        }

        // First translate the condition
        TranslateNode(condition);

        // Then translate the body
        TranslateNode(bodyNode);

        // Add WhileLoop operation directly
        AppendDirectOperation(Operation::WhileLoop);

        KAI_TRACE() << "While loop translation complete with direct Pi operations";
    } catch (const std::exception& e) {
        KAI_TRACE_ERROR() << std::format("Exception in TranslateWhile: {}", e.what());
        Fail(std::format("Exception in TranslateWhile: {}", e.what()));
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslateWhile";
        Fail("Unknown exception in TranslateWhile");
    }
}

// DoWhile functionality has been removed to simplify the language implementation
// and focus on core binary operations that work correctly

// Handle Pi block with braces: pi { ... }
void RhoTranslator::TranslatePiBlock(AstNodePtr /*parentNode*/, size_t /*startIndex*/) {
    KAI_TRACE() << "Translating Pi block (with braces)";
    
    // This method is kept as a placeholder for future implementation
    // using C++20 coroutines for more natural flow control
}

void RhoTranslator::TranslateList(AstNodePtr node) {
    KAI_TRACE() << std::format("Translating List (array): {}", node->ToString());

    // Get the number of elements
    const auto numElements = static_cast<int>(node->GetChildren().size());
    
    // First translate all elements onto the stack using C++20 ranges
    for (const auto& element : node->GetChildren() | std::views::all) {
        TranslateNode(element);
    }
    
    // Create a new integer representing the count
    AppendLiteral(numElements);
    
    // Now create an array from the top N elements on the stack
    AppendDirectOperation(Operation::ToArray);

    KAI_TRACE() << std::format("List translation complete with {} elements", numElements);
}

KAI_END