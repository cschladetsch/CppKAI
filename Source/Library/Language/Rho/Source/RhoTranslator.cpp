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
                // Create a properly typed integer value
                auto intObj = reg_->New<int>(value);
                
                // Append the integer directly
                Append(intObj);
                
                KAI_TRACE() << "Translated integer: " << intObj.ToString();
            } catch (const std::exception& e) {
                Fail(std::format("Failed to parse integer: {}", e.what()));
            }
            return;
        }

        case TokenEnum::Float: {
            KAI_TRACE() << std::format("Translating Float: {}", node->GetTokenText());
            try {
                const auto value = std::stof(node->GetTokenText());
                // Create a properly typed float value
                auto floatObj = reg_->New<float>(value);
                
                // Append the float directly
                Append(floatObj);
                
                KAI_TRACE() << "Translated float: " << floatObj.ToString();
            } catch (const std::exception& e) {
                Fail(std::format("Failed to parse float: {}", e.what()));
            }
            return;
        }

        case TokenEnum::String: {
            KAI_TRACE() << std::format("Translating String: {}", node->Text());
            // Create a properly typed string
            auto strObj = reg_->New<String>(String(node->Text()));
            
            // Append the string directly
            Append(strObj);
            
            KAI_TRACE() << "Translated string: " << strObj.ToString();
            return;
        }

        case TokenEnum::Ident: {
            KAI_TRACE() << std::format("Translating Ident: {}", node->Text());
            // Create a properly typed label
            auto labelObj = reg_->New<Label>(Label(node->Text()));
            
            // Append the label directly
            Append(labelObj);
            
            KAI_TRACE() << "Translated identifier: " << labelObj.ToString();
            return;
        }

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
            
            // Attempt to optimize simple Pi expressions by evaluating them directly
            bool canOptimize = false;
            
            // Look at the children nodes to see if this is a simple expression
            // like "2 3 + " that we can evaluate directly
            if (node->GetChildren().size() == 3) {
                auto it = node->GetChildren().begin();
                auto first = *it++;
                auto second = *it++;
                auto op = *it;
                
                // Check if first and second are integer literals
                if (first->GetType() == AstEnum::TokenType && second->GetType() == AstEnum::TokenType &&
                    op->GetType() == AstEnum::TokenType) {
                    
                    if (first->GetToken().type == TokenEnum::Int && 
                        second->GetToken().type == TokenEnum::Int &&
                        (op->GetToken().type == TokenEnum::Plus || 
                         op->GetToken().type == TokenEnum::Minus || 
                         op->GetToken().type == TokenEnum::Mul || 
                         op->GetToken().type == TokenEnum::Divide)) {
                        
                        try {
                            // Parse the integer literals
                            int firstValue = std::stoi(first->GetTokenText());
                            int secondValue = std::stoi(second->GetTokenText());
                            int result = 0;
                            
                            // Calculate the result based on the operation
                            switch (op->GetToken().type) {
                                case TokenEnum::Plus:
                                    result = firstValue + secondValue;
                                    break;
                                case TokenEnum::Minus:
                                    result = firstValue - secondValue;
                                    break;
                                case TokenEnum::Mul:
                                    result = firstValue * secondValue;
                                    break;
                                case TokenEnum::Divide:
                                    if (secondValue != 0) {
                                        result = firstValue / secondValue;
                                    }
                                    else {
                                        // Division by zero - revert to normal handling
                                        canOptimize = false;
                                    }
                                    break;
                                default:
                                    // Should not get here
                                    canOptimize = false;
                                    break;
                            }
                            
                            // Direct evaluation successful - create integer result
                            if (canOptimize) {
                                KAI_TRACE() << "Direct evaluation of Pi expression: result = " << result;
                                // Create a properly typed integer object
                                Object resultObj = reg_->New<int>(result);
                                // Append directly to the parent
                                Append(resultObj);
                                return;
                            }
                        }
                        catch (const std::exception& e) {
                            // Parse error - fallback to normal handling
                            KAI_TRACE_ERROR() << "Error optimizing Pi expression: " << e.what();
                            canOptimize = false;
                        }
                    }
                }
            }
            
            // If optimization failed, use normal approach: create a continuation
            PushNew();
            
            // Translate all the children nodes
            for (const auto& child : node->GetChildren()) {
                TranslateNode(child);
            }
            
            // Pop the continuation
            auto piCont = Pop();
            
            // Mark it with a special flag to indicate it's a Pi continuation
            // that should be automatically resolved when evaluated
            // This will be used by the PerformBinaryOp and friends in Executor
            piCont->SetSpecialHandling(true);
            
            // Add the continuation to the parent
            KAI_TRACE() << "Using continuation for Pi sequence (with special handling)";
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
    
    // Get the left and right children for any operation
    auto leftChild = node->GetChild(0);
    auto rightChild = node->GetChild(1);
    
    if (!leftChild || !rightChild) {
        KAI_TRACE_ERROR() << "Binary operation missing children";
        Fail("Binary operation missing children");
        return;
    }
    
    // Create temporary objects to capture the results of translating the children
    // We'll need to translate first to handle variable references, expressions, etc.
    
    // Create new code array to capture left operand translation
    // This will let us execute and extract the result at translation time
    PushNew();
    TranslateNode(leftChild);
    Pointer<Continuation> leftCont = Pop();
    
    // Create new code array to capture right operand translation
    PushNew();
    TranslateNode(rightChild);
    Pointer<Continuation> rightCont = Pop();
    
    // Now evaluate the left and right continuations to get their actual values
    // Create an executor to evaluate the continuations
    Pointer<Executor> executor = reg_->New<Executor>();
    executor->Create();
    
    // Set up a temporary data stack for evaluation
    Pointer<Stack> tempStack = reg_->New<Stack>();
    executor->SetDataStack(tempStack);
    
    // Execute left continuation and get result
    executor->Continue(leftCont);
    Object leftObj;
    if (!tempStack->Empty()) {
        leftObj = tempStack->Pop();
    }
    
    // Execute right continuation and get result
    tempStack->Clear();
    executor->Continue(rightCont);
    Object rightObj;
    if (!tempStack->Empty()) {
        rightObj = tempStack->Pop();
    }
    
    // If we have both operands, we can evaluate the operation directly
    if (leftObj.Exists() && rightObj.Exists()) {
        // Create a clean executor for the binary operation
        Pointer<Executor> opExecutor = reg_->New<Executor>();
        opExecutor->Create();
        
        // Use the PerformBinaryOp helper to handle the operation
        Object result = opExecutor->PerformBinaryOp(leftObj, rightObj, op);
        
        if (result.Exists()) {
            // Log the direct evaluation
            KAI_TRACE() << "Direct evaluation of binary op: " << leftObj.ToString() << " " 
                        << Operation::ToString(op) << " " << rightObj.ToString() << " = " << result.ToString();
            
            // Append the result directly - this is key for proper type preservation
            Append(result);
            return;
        }
    }
    
    // Fallback to creating a continuation with the operation
    // This should rarely be needed since we're evaluating at translation time
    KAI_TRACE() << "Fallback to continuation-based binary operation";
    
    // Since we've already translated and evaluated the operands above,
    // we need to translate them again to add them to the code stream
    
    // First translate the left operand
    TranslateNode(leftChild);
    
    // Then translate the right operand
    TranslateNode(rightChild);
    
    // Finally, add the operation directly
    AppendDirectOperation(op);
    
    KAI_TRACE() << "Binary operation translated to Pi stream";
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