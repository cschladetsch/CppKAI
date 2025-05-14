#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;

KAI_BEGIN

// Note: The RhoTranslator::Translate method is now implemented in RhoTranslate.cpp
// to avoid duplicate implementation errors


void RhoTranslator::TranslateToken(AstNodePtr node) {
    KAI_TRACE() << "TranslateToken: "
                << RhoTokenEnumType::ToString(node->GetToken().type);
    KAI_TRACE() << "TranslateToken with text: " << node->Text();

    switch (node->GetToken().type) {
        case TokenEnum::OpenParan:
            for (auto ch : node->GetChildren()) TranslateNode(ch);
            return;

        case TokenEnum::Not:
            // Create a continuation for this unary operation
            {
                Pointer<Continuation> opCont = reg_->New<Continuation>();
                opCont->SetCode(reg_->New<Array>());
                stack.push_back(opCont);
                
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
                Pointer<Continuation> opCont = reg_->New<Continuation>();
                opCont->SetCode(reg_->New<Array>());
                stack.push_back(opCont);
                
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

        case TokenEnum::Int:
            KAI_TRACE() << "Translating Int: " << node->GetTokenText();
            // Use explicit type parameter for New to ensure proper type identification
            Append(reg_->New<int>(boost::lexical_cast<int>(node->GetTokenText())));
            return;

        case TokenEnum::Float:
            KAI_TRACE() << "Translating Float: " << node->GetTokenText();
            // Use explicit type parameter for New to ensure proper type identification
            Append(reg_->New<float>(boost::lexical_cast<float>(node->GetTokenText())));
            return;

        case TokenEnum::String:
            KAI_TRACE() << "Translating String: " << node->Text();
            // Use explicit type parameter for New to ensure proper type identification
            Append(reg_->New<String>(String(node->Text())));
            KAI_TRACE() << "String translation complete";
            return;

        case TokenEnum::Ident:
            KAI_TRACE() << "Translating Ident: " << node->Text();
            // Use explicit type parameter for New to ensure proper type identification
            Append(reg_->New<Label>(Label(node->Text())));
            KAI_TRACE() << "Ident translation complete";
            return;

        case TokenEnum::Pathname:
            KAI_TRACE() << "Translating Pathname: " << node->Text();
            // Use explicit type parameter for New to ensure proper type identification
            Append(reg_->New<Pathname>(Pathname(node->Text())));
            return;

        case TokenEnum::ToPi:
            AppendDirectOperation(Operation::ToPi);
            return;
            
        case TokenEnum::PiSequence:
        {
            KAI_TRACE() << "Translating PiSequence: " << node->Text();
            // Create a continuation for the Pi code block
            PushNew();
            
            // Translate all the children nodes
            for (auto child : node->GetChildren()) {
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
            // for (auto ch : node->Children)
            //     Translate(ch);
            // AppendNewOp(Operation::PushContext);
            KAI_NOT_IMPLEMENTED();
            return;

        case TokenEnum::Return:
            for (auto ch : node->GetChildren()) TranslateNode(ch);
            AppendDirectOperation(Operation::Return);
            return;
    }

    Fail("Unsupported node %s", node->ToString().c_str());
    KAI_TRACE_ERROR() << "Error: " << Error;
    KAI_NOT_IMPLEMENTED();
}

void RhoTranslator::TranslateBinaryOp(AstNodePtr node, Operation::Type op) {
    KAI_TRACE() << "TranslateBinaryOp: Operation=" << Operation::ToString(op);

    // For binary operations, we need to ensure we're not just pushing Continuations
    // but actually getting the result of evaluating expressions
    
    // First, check if both children are simple integer literals
    // In such case, we could evaluate the operation at translation time
    bool canEvaluateNow = false;
    int leftValue = 0, rightValue = 0;
    
    if (node->GetChild(0)->GetType() == AstNodeEnum::TokenType && 
        node->GetChild(1)->GetType() == AstNodeEnum::TokenType) {
        
        TokenNode leftToken = node->GetChild(0)->GetToken();
        TokenNode rightToken = node->GetChild(1)->GetToken();
        
        // Check if both tokens are integers
        if (leftToken.type == TokenEnum::Int && rightToken.type == TokenEnum::Int) {
            try {
                leftValue = boost::lexical_cast<int>(node->GetChild(0)->GetTokenText());
                rightValue = boost::lexical_cast<int>(node->GetChild(1)->GetTokenText());
                canEvaluateNow = true;
            }
            catch (...) {
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
            KAI_TRACE() << "Direct evaluation of binary op: " << leftValue << " " 
                        << Operation::ToString(op) << " " << rightValue << " = " << result;
            Append(reg_->New<int>(result));
            return;
        }
    }
    
    // Create a new continuation for this binary operation
    // This ensures it gets properly wrapped and typed for the executor
    Pointer<Continuation> opCont = reg_->New<Continuation>();
    opCont->SetCode(reg_->New<Array>());
    
    // Push this new continuation to become the current context
    stack.push_back(opCont);
    
    // Translate left and right operands into this continuation
    TranslateNode(node->GetChild(0));
    TranslateNode(node->GetChild(1));
    
    // Add the operation to the continuation
    AppendDirectOperation(op);
    
    // Pop the continuation from the stack
    Pointer<Continuation> completedOp = Pop();
    
    // Add the completed operation continuation to the parent
    Append(completedOp);

    KAI_TRACE() << "Binary operation successfully translated";
}

// void RhoTranslator::TranslatePathname(AstNodePtr node)
//{
//     Pathname::Elements elements;
//     typedef Pathname::Element El;
//
//     for (auto ch : node->GetChildren())
//     {
//         switch (ch->GetToken().type)
//         {
//         case RhoTokenEnumType::Quote:
//             elements.push_back(El::Quote);
//             break;
//         case RhoTokenEnumType::Sep:
//             elements.push_back(El::Separator);
//             break;
//         case RhoTokenEnumType::Ident:
//             elements.push_back(Label(ch->GetTokenText()));
//             break;
//         }
//     }
//
//     AppendNew(Pathname(move(elements)));
// }

void RhoTranslator::TranslateNode(AstNodePtr node) {
    if (!node) {
        Failed = true;
        return;
    }

    KAI_TRACE() << "TranslateNode: Type="
                << RhoAstNodeEnumType::ToString(node->GetType())
                << " Text=" << node->Text();

    switch (node->GetType()) {
        case AstEnum::Pathname:
            TranslateToken(node);
            return;

        case AstEnum::IndexOp:
            TranslateBinaryOp(node, Operation::Index);
            return;

        case AstEnum::GetMember:
            TranslateBinaryOp(node, Operation::GetProperty);
            return;

        case AstEnum::TokenType: {
            // Special handling for Pi with braces (handle the case where pi + { are separate tokens)
            if (node->GetToken().type == TokenEnum::ToPi) {
                // Since we can't access the parent directly, we'll handle Pi blocks
                // by looking at the next token in the parse tree as needed
                
                // For Pi blocks, we'll rely on the token sequence during parsing
                // where we should have already created the appropriate structure using
                // the PiSequence token type that's generated by the parser.
                // The raw 'pi' token followed by '{' is only needed if manually parsing
                // with the lower-level API.
                
                // Just translate the ToPi token as normal
                AppendDirectOperation(Operation::ToPi);
            }
            
            // Regular token handling
            TranslateToken(node);
            return;
        }

        case AstEnum::Assignment:
            KAI_TRACE() << "Translating Assignment";
            // Put this in a try-catch block to help diagnose issues
            try {
                // Basic validation
                if (node->GetChildren().size() < 2) {
                    KAI_TRACE_ERROR()
                        << "Assignment node has fewer than 2 children";
                    Fail("Assignment node has fewer than 2 children");
                    return;
                }

                // Value first (right side)
                TranslateNode(node->GetChild(1));

                // Then target (left side)
                TranslateNode(node->GetChild(0));

                // Add the store operation directly
                AppendDirectOperation(Operation::Store);

                KAI_TRACE()
                    << "Completed assignment translation without Continuations";
            } catch (const std::exception &e) {
                KAI_TRACE_ERROR() << "Exception in assignment: " << e.what();
                Fail(std::string("Assignment failed: ") + e.what());
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

        case AstEnum::DoWhile:
            TranslateDoWhile(node);
            return;

        case AstEnum::Block:
            for (auto st : node->GetChildren()) TranslateNode(st);
            return;

        case AstEnum::List:
            TranslateList(node);
            return;

        case AstEnum::Function:
            TranslateFunction(node);
            return;

        case AstEnum::Program:
            for (auto e : node->GetChildren()) TranslateNode(e);
            return;
    }

    Fail("Unsupported node %s", node->ToString().c_str());
    KAI_NOT_IMPLEMENTED();
}

void RhoTranslator::TranslateBlock(AstNodePtr node) {
    for (auto st : node->GetChildren()) TranslateNode(st);
}

void RhoTranslator::TranslateFunction(AstNodePtr node) {
    KAI_TRACE() << "TranslateFunction: Creating function continuation";
    
    // child 0: ident
    // child 1: args
    // child 2: block
    AstNode::ChildrenType const &ch = node->GetChildren();

    // Create a Continuation for the function body
    Pointer<Continuation> cont = reg_->New<Continuation>();
    if (!cont.Exists()) {
        KAI_TRACE_ERROR() << "Failed to create function continuation";
        Fail("Failed to create function continuation");
        return;
    }

    // Set its code array
    cont->SetCode(reg_->New<Array>());
    if (!cont->GetCode().Exists()) {
        KAI_TRACE_ERROR() << "Failed to create function code array";
        Fail("Failed to create function code array");
        return;
    }
    
    // Note: We're not setting Language properties anymore since they're not registered
    // in the Continuation class. Language context is now handled in Console.cpp.
    // cont.SetPropertyValue(Label("Language"), _reg->New<String>("Rho"));
    // cont.SetPropertyValue(Label("RhoFunction"), _reg->New<bool>(true));

    // Write the body into the continuation's code array
    stack.push_back(cont);
    
    // Process the function body (the block)
    if (ch.size() > 2) {
        KAI_TRACE() << "Processing function body with " 
                    << static_cast<int>(ch[2]->GetChildren().size()) << " statements";
                    
        // Process each statement in the block
        for (auto b : ch[2]->GetChildren()) {
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
    
    stack.pop_back();

    // Add the args to the function
    for (auto a : ch[1]->GetChildren()) {
        cont->AddArg(Label(a->GetTokenText()));
    }

    // Store the function: push function object, push function name, store
    Append(cont);
    Append(New<Label>(Label(ch[0]->Text())));
    AppendDirectOperation(Operation::Store);
    
    KAI_TRACE() << "Function translation complete";
}

void RhoTranslator::TranslateCall(AstNodePtr node) {
    KAI_TRACE() << "Translating call";

    typename AstNode::ChildrenType const &children = node->GetChildren();
    for (auto a : children[1]->GetChildren()) {
        TranslateNode(a);
    }

    TranslateNode(children[0]);

    Operation::Type callOp;
    if (children.size() > 2 &&
        children[2]->GetToken().type == TokenEnum::Replace) {
        callOp = Operation::Replace;
    } else {
        callOp = Operation::Suspend;
    }

    AppendDirectOperation(callOp);

    KAI_TRACE() << "Completed call translation";
}

void RhoTranslator::TranslateIf(AstNodePtr node) {
    KAI_TRACE() << "Translating if statement";

    typename AstNode::ChildrenType const &ch = node->GetChildren();
    bool hasElse = ch.size() > 2;

    // For if statements in Pi, we need to create continuations for then and
    // else blocks
    Pointer<Continuation> thenCont = reg_->New<Continuation>();
    thenCont->SetCode(reg_->New<Array>());

    Pointer<Continuation> elseCont;
    if (hasElse) {
        elseCont = reg_->New<Continuation>();
        elseCont->SetCode(reg_->New<Array>());
    }

    // First translate the condition
    TranslateNode(ch[0]);

    // Translate then branch into its continuation
    stack.push_back(thenCont);
    TranslateNode(ch[1]);
    stack.pop_back();

    // Translate else branch if it exists
    if (hasElse) {
        stack.push_back(elseCont);
        TranslateNode(ch[2]);
        stack.pop_back();
    }

    // Add continuations and if operation
    Append(thenCont);

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
        if (node->GetChildren().size() < 1) {
            KAI_TRACE_ERROR() << "Not enough children in While node";
            Fail("While node must have at least a condition child");
            return;
        }

        // Get condition and body nodes
        AstNodePtr condition = node->GetChild(0);

        AstNodePtr body = nullptr;
        if (node->GetChildren().size() > 1) {
            body = node->GetChild(1);
        } else {
            // Create an empty body block node
            body = std::make_shared<RhoAstNode>(RhoAstNodeEnumType::Block);
        }

        // For while loops in Pi, we need continuations for condition and body
        // First create condition continuation
        Pointer<Continuation> condCont = reg_->New<Continuation>();
        condCont->SetCode(reg_->New<Array>());

        // Then create body continuation
        Pointer<Continuation> bodyCont = reg_->New<Continuation>();
        bodyCont->SetCode(reg_->New<Array>());

        // Translate condition into condition continuation
        stack.push_back(condCont);
        TranslateNode(condition);
        stack.pop_back();

        // Translate body into body continuation
        stack.push_back(bodyCont);
        TranslateNode(body);
        stack.pop_back();

        // Add continuations to code in correct order for WhileLoop operation
        Append(condCont);
        Append(bodyCont);

        // Add WhileLoop operation directly
        AppendDirectOperation(Operation::WhileLoop);

        KAI_TRACE() << "While loop translation complete";
    } catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslateWhile: " << e.what();
        Fail(std::string("Exception in TranslateWhile: ") + e.what());
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslateWhile";
        Fail("Unknown exception in TranslateWhile");
    }
}

void RhoTranslator::TranslateDoWhile(AstNodePtr node) {
    try {
        KAI_TRACE() << "Translating do-while loop";

        // Verify we have enough children
        if (node->GetChildren().size() < 2) {
            KAI_TRACE_ERROR() << "Not enough children in DoWhile node";
            Fail("DoWhile node must have both body and condition children");
            return;
        }

        // Get body and condition nodes (order is different from while loop)
        AstNodePtr body = node->GetChild(0);
        AstNodePtr condition = node->GetChild(1);

        // For do-while loops in Pi, we need continuations for condition and
        // body First create condition continuation
        Pointer<Continuation> condCont = reg_->New<Continuation>();
        condCont->SetCode(reg_->New<Array>());

        // Then create body continuation
        Pointer<Continuation> bodyCont = reg_->New<Continuation>();
        bodyCont->SetCode(reg_->New<Array>());

        // Translate body into body continuation
        stack.push_back(bodyCont);
        TranslateNode(body);
        stack.pop_back();

        // Translate condition into condition continuation
        stack.push_back(condCont);
        TranslateNode(condition);
        stack.pop_back();

        // Add continuations to code in correct order for DoLoop operation
        // Note: Order is different for do-while compared to while
        Append(condCont);
        Append(bodyCont);

        // Add DoLoop operation directly
        AppendDirectOperation(Operation::DoLoop);

        KAI_TRACE() << "Do-while loop translation complete";
    } catch (kai::Exception::Base &e) {
        KAI_TRACE_ERROR() << "KAI Exception in TranslateDoWhile: "
                          << e.ToString();
        Fail(std::string("Exception in TranslateDoWhile: ") +
             e.ToString().c_str());
    } catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslateDoWhile: " << e.what();
        Fail(std::string("Exception in TranslateDoWhile: ") + e.what());
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslateDoWhile";
        Fail("Unknown exception in TranslateDoWhile");
    }
}

// Handle Pi block with braces: pi { ... }
void RhoTranslator::TranslatePiBlock(AstNodePtr parentNode, size_t startIndex) {
    KAI_TRACE() << "Translating Pi block (with braces)";
    
    // This method is not currently used since we commented out the parent-child
    // navigation code in TranslateNode. This will need to be revisited
    // when parent-child navigation is added to the AST.
    
    // For now, this is kept as a placeholder for future implementation.
    // We'll add a proper implementation that doesn't rely on GetParent() in a future update
    
    // The original implementation would:
    // 1. Find all nodes between opening and closing braces
    // 2. Create a Pi language continuation
    // 3. Process all nodes as Pi code
    // 4. Add the continuation to the current code array
}

void RhoTranslator::TranslateList(AstNodePtr node) {
    KAI_TRACE() << "Translating List (array): " << node->ToString();

    // First push all elements onto the stack
    for (auto element : node->GetChildren()) {
        TranslateNode(element);
    }

    // Get the number of elements
    int numElements = static_cast<int>(node->GetChildren().size());
    
    // Create a new integer representing the count
    AppendLiteral(numElements);
    
    // Now create an array from the top N elements on the stack
    AppendDirectOperation(Operation::ToArray);

    KAI_TRACE() << "List translation complete with " << numElements << " elements";
}

KAI_END