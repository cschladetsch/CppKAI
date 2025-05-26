#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Pi/PiToken.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <concepts>
#include <format>
#include <ranges>
#include <stdexcept>

KAI_BEGIN

// Note: The RhoTranslator::Translate method is now implemented in
// RhoTranslate.cpp to avoid duplicate implementation errors

// Helper method to convert token types to operation types
Operation::Type RhoTranslator::TokenToOperation(
    RhoTokenEnumType::Enum tokenType) {
    // Direct mapping from token enum to operation type
    if (tokenType == RhoTokenEnumType::Plus) return Operation::Plus;
    if (tokenType == RhoTokenEnumType::Minus) return Operation::Minus;
    if (tokenType == RhoTokenEnumType::Mul) return Operation::Multiply;
    if (tokenType == RhoTokenEnumType::Divide) return Operation::Divide;
    if (tokenType == RhoTokenEnumType::Mod) return Operation::Modulo;
    if (tokenType == RhoTokenEnumType::Equiv) return Operation::Equiv;
    if (tokenType == RhoTokenEnumType::NotEquiv) return Operation::NotEquiv;
    if (tokenType == RhoTokenEnumType::Less) return Operation::Less;
    if (tokenType == RhoTokenEnumType::Greater) return Operation::Greater;
    if (tokenType == RhoTokenEnumType::LessEquiv) return Operation::LessOrEquiv;
    if (tokenType == RhoTokenEnumType::GreaterEquiv)
        return Operation::GreaterOrEquiv;
    if (tokenType == RhoTokenEnumType::And) return Operation::LogicalAnd;
    if (tokenType == RhoTokenEnumType::Or) return Operation::LogicalOr;
    // Add other mappings as needed
    return Operation::None;
}

void RhoTranslator::TranslateToken(AstNodePtr node) {
    KAI_TRACE() << std::format(
        "TranslateToken: {}",
        RhoTokenEnumType::ToString(node->GetToken().type));
    KAI_TRACE() << std::format("TranslateToken with text: {}", node->Text());

    switch (node->GetToken().type) {
        case RhoTokenEnumType::OpenParan:
            for (const auto& ch : node->GetChildren()) {
                TranslateNode(ch);
            }
            return;

        case RhoTokenEnumType::Not:
            // Use a linear stream approach for the unary 'not' operation

            // Translate the operand
            TranslateNode(node->GetChild(0));

            // Add the operation directly
            AppendDirectOperation(Operation::LogicalNot);

            return;

        case RhoTokenEnumType::True:
            // Create an actual boolean true value
            Append(reg_->New<bool>(true));
            return;

        case RhoTokenEnumType::False:
            // Create an actual boolean false value
            Append(reg_->New<bool>(false));
            return;

        case RhoTokenEnumType::Assert:
            // Use a linear stream approach for the assert operation

            // Translate the assertion condition
            TranslateNode(node->GetChild(0));

            // Add the operation directly
            AppendDirectOperation(Operation::Assert);

            return;

        case RhoTokenEnumType::DivAssign:
            TranslateBinaryOp(node, Operation::DivEquals);
            return;

        case RhoTokenEnumType::MulAssign:
            TranslateBinaryOp(node, Operation::MulEquals);
            return;

        case RhoTokenEnumType::MinusAssign:
            TranslateBinaryOp(node, Operation::MinusEquals);
            return;

        case RhoTokenEnumType::PlusAssign:
            TranslateBinaryOp(node, Operation::PlusEquals);
            return;

        case RhoTokenEnumType::Assign:
            TranslateBinaryOp(node, Operation::Store);
            return;

        case RhoTokenEnumType::Lookup:
            AppendDirectOperation(Operation::Lookup);
            return;

        case RhoTokenEnumType::Self:
            AppendDirectOperation(Operation::This);
            return;

        case RhoTokenEnumType::NotEquiv:
            TranslateBinaryOp(node, Operation::NotEquiv);
            return;

        case RhoTokenEnumType::Equiv:
            TranslateBinaryOp(node, Operation::Equiv);
            return;

        case RhoTokenEnumType::Less:
            TranslateBinaryOp(node, Operation::Less);
            return;

        case RhoTokenEnumType::Greater:
            TranslateBinaryOp(node, Operation::Greater);
            return;

        case RhoTokenEnumType::GreaterEquiv:
            TranslateBinaryOp(node, Operation::GreaterOrEquiv);
            return;

        case RhoTokenEnumType::LessEquiv:
            TranslateBinaryOp(node, Operation::LessOrEquiv);
            return;

        case RhoTokenEnumType::Minus:
            TranslateBinaryOp(node, Operation::Minus);
            return;

        case RhoTokenEnumType::Plus:
            KAI_TRACE() << "Translating Plus operation";
            TranslateBinaryOp(node, Operation::Plus);
            return;

        case RhoTokenEnumType::Mul:
            TranslateBinaryOp(node, Operation::Multiply);
            return;

        case RhoTokenEnumType::Divide:
            TranslateBinaryOp(node, Operation::Divide);
            return;

        case RhoTokenEnumType::Mod:
            TranslateBinaryOp(node, Operation::Modulo);
            return;

        case RhoTokenEnumType::Or:
            TranslateBinaryOp(node, Operation::LogicalOr);
            return;

        case RhoTokenEnumType::And:
            TranslateBinaryOp(node, Operation::LogicalAnd);
            return;

        case RhoTokenEnumType::BitAnd:
            TranslateBinaryOp(node, Operation::BitwiseAnd);
            return;

        case RhoTokenEnumType::BitOr:
            TranslateBinaryOp(node, Operation::BitwiseOr);
            return;

        case RhoTokenEnumType::BitXor:
            TranslateBinaryOp(node, Operation::BitwiseXor);
            return;

        case RhoTokenEnumType::LeftShift:
            TranslateBinaryOp(node, Operation::LeftShift);
            return;

        case RhoTokenEnumType::RightShift:
            TranslateBinaryOp(node, Operation::RightShift);
            return;

        case RhoTokenEnumType::BitNot:
            // Use a linear stream approach for the unary bitwise not operation

            // Translate the operand
            TranslateNode(node->GetChild(0));

            // Add the operation directly
            AppendDirectOperation(Operation::BitwiseNot);

            return;

        case RhoTokenEnumType::Int: {
            KAI_TRACE() << std::format("Translating Int: {}",
                                       node->GetTokenText());
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

        case RhoTokenEnumType::Float: {
            KAI_TRACE() << std::format("Translating Float: {}",
                                       node->GetTokenText());
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

        case RhoTokenEnumType::String: {
            KAI_TRACE() << std::format("Translating String: {}", node->Text());
            // Create a properly typed string
            auto strObj = reg_->New<String>(String(node->Text()));

            // Append the string directly
            Append(strObj);

            KAI_TRACE() << "Translated string: " << strObj.ToString();
            return;
        }

        case RhoTokenEnumType::Label: {
            KAI_TRACE() << std::format("Translating Ident: {}", node->Text());
            // In Rho, identifiers in expressions need to be resolved to their values
            // For Pi-style execution, push the label and then a Retrieve operation
            auto labelObj = reg_->New<Label>(Label(node->Text()));

            // Append the label
            Append(labelObj);
            
            // Add Retrieve operation to resolve the label to its value
            AppendDirectOperation(Operation::Retreive);

            KAI_TRACE() << "Translated identifier as label with retrieve: " << labelObj.ToString();
            return;
        }

        case RhoTokenEnumType::Pathname:
            KAI_TRACE() << std::format("Translating Pathname: {}",
                                       node->Text());
            Append(reg_->New<Pathname>(Pathname(node->Text())));
            return;

        case RhoTokenEnumType::ToPi:
            AppendDirectOperation(Operation::ToPi);
            return;

        case RhoTokenEnumType::PiSequence: {
            KAI_TRACE() << std::format("Translating PiSequence: {}",
                                       node->Text());

            // IMPORTANT: Do NOT do direct evaluation at translation time!
            // The original implementation had extensive direct evaluation logic
            // that would compute results at translation time and append them
            // directly. This caused type mismatches because the executor expects
            // operations to execute, not pre-computed values.
            //
            // Instead, we translate each element of the Pi sequence into
            // operations that can be executed at runtime.
            
            // Simply translate each child node in the sequence
            for (const auto& child : node->GetChildren()) {
                TranslateNode(child);
            }
            
            KAI_TRACE() << "Pi sequence translation complete";
            return;
        }

        case RhoTokenEnumType::Yield:
            // Modern implementation would use coroutines with co_yield
            KAI_NOT_IMPLEMENTED();
            return;

        case RhoTokenEnumType::Return:
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

void RhoTranslator::TranslateWhile(AstNodePtr node) {
    KAI_TRACE() << "Translating while loop";

    // While loops need: condition and body
    if (node->GetChildren().size() < 2) {
        KAI_TRACE_ERROR() << "While node needs condition and body";
        return;
    }

    // Create continuation for condition
    PushNew();
    TranslateNode(node->GetChild(0));
    auto condCont = Pop();
    
    // Create continuation for body
    PushNew();
    TranslateNode(node->GetChild(1));
    auto bodyCont = Pop();
    
    // Push continuations on stack for WhileLoop operation
    Append(condCont);
    Append(bodyCont);
    AppendDirectOperation(Operation::WhileLoop);
}

void RhoTranslator::TranslateFor(AstNodePtr node) {
    KAI_TRACE() << "Translating for loop";
    
    // For loops have: init, condition, update, body
    if (node->GetChildren().size() < 4) {
        KAI_TRACE_ERROR() << "For node needs init, condition, update, and body";
        return;
    }
    
    // Translate initialization directly
    TranslateNode(node->GetChild(0));
    
    // Create continuation for condition
    PushNew();
    TranslateNode(node->GetChild(1));
    auto condCont = Pop();
    
    // Create continuation for body
    PushNew();
    TranslateNode(node->GetChild(3));
    auto bodyCont = Pop();
    
    // Create continuation for update
    PushNew();
    TranslateNode(node->GetChild(2));
    auto updateCont = Pop();
    
    // Push continuations on stack for ForLoop operation
    Append(condCont);
    Append(bodyCont);
    Append(updateCont);
    AppendDirectOperation(Operation::ForLoop);
}

void RhoTranslator::TranslateDoWhile(AstNodePtr node) {
    KAI_TRACE() << "Translating do-while loop";
    
    // Do-while loops need: body and condition
    if (node->GetChildren().size() < 2) {
        KAI_TRACE_ERROR() << "DoWhile node needs body and condition";
        return;
    }
    
    // Create continuation for body
    PushNew();
    TranslateNode(node->GetChild(0));
    auto bodyCont = Pop();
    
    // Create continuation for condition
    PushNew();
    TranslateNode(node->GetChild(1));
    auto condCont = Pop();
    
    // Push continuations on stack for DoLoop operation
    // Order: body first, then condition
    Append(bodyCont);
    Append(condCont);
    AppendDirectOperation(Operation::DoLoop);
}

void RhoTranslator::TranslateIf(AstNodePtr node) {
    KAI_TRACE() << "Translating if statement";
    
    // If statements need at least condition and then-block
    if (node->GetChildren().size() < 2) {
        KAI_TRACE_ERROR() << "If node needs at least condition and then block";
        return;
    }
    
    // Translate condition directly (not in a continuation)
    TranslateNode(node->GetChild(0));
    
    // Create continuation for then block
    PushNew();
    TranslateNode(node->GetChild(1));
    auto thenCont = Pop();
    
    // Check if there's an else block
    if (node->GetChildren().size() > 2) {
        // Create continuation for else block
        PushNew();
        TranslateNode(node->GetChild(2));
        auto elseCont = Pop();
        
        // Push continuations and IfElse operation
        Append(thenCont);
        Append(elseCont);
        AppendDirectOperation(Operation::IfElse);
    } else {
        // No else block - just then continuation and If operation
        Append(thenCont);
        AppendDirectOperation(Operation::If);
    }
}

void RhoTranslator::TranslateList(AstNodePtr node) {
    KAI_TRACE() << "TranslateList - Creating array literal";
    
    // Translate all list elements first
    for (const auto& child : node->GetChildren()) {
        TranslateNode(child);
    }
    
    // Create the array from the stack elements
    // The number of elements is the number of children
    size_t numElements = node->GetChildren().size();
    
    // Push the number of elements to create array from
    AppendNew<int>(numElements);
    // Use ToArray operation to create array from stack elements
    AppendDirectOperation(Operation::ToArray);
    
    KAI_TRACE() << std::format("Created array with {} elements", numElements);
}

void RhoTranslator::TranslateMap(AstNodePtr node) {
    KAI_TRACE() << "TranslateMap - Creating map literal";
    
    // For now, we only support empty maps
    // Push 0 to indicate no elements
    AppendNew<int>(0);
    
    // Use ToMap operation to create empty map
    AppendDirectOperation(Operation::ToMap);
    
    KAI_TRACE() << "Created empty map";
}

void RhoTranslator::TranslateIndex(AstNodePtr node) {
    KAI_TRACE() << "TranslateIndex - Array indexing operation";
    
    // IndexOp should have 2 children: the array and the index
    auto children = node->GetChildren();
    if (children.size() != 2) {
        KAI_TRACE_ERROR() << "IndexOp should have exactly 2 children, got " << static_cast<int>(children.size());
        return;
    }
    
    // Translate the array expression
    TranslateNode(children[0]);
    
    // Translate the index expression
    TranslateNode(children[1]);
    
    // Apply the Index operation
    AppendDirectOperation(Operation::Index);
    
    KAI_TRACE() << "Array indexing operation translated";
}

void RhoTranslator::TranslateBinaryOp(AstNodePtr node, Operation::Type op) {
    KAI_TRACE() << std::format("TranslateBinaryOp: Operation={}",
                               Operation::ToString(op));

    // For binary operations, we want a linear stream of operations
    // rather than creating nested continuations

    // IMPORTANT: We should NOT do direct evaluation at translation time!
    // The issue with the original implementation was that it was trying to
    // evaluate expressions at translation time and append the results directly.
    // This caused type mismatches because the executor expects operations
    // (continuations) to execute, not pre-computed values.
    //
    // The correct approach is to always translate the operands and append
    // the operation, letting the executor handle the actual computation
    // at runtime.

    // Special handling for assignment operations - they need identifier names, not values
    if (op == Operation::Store || 
        op == Operation::PlusEquals || 
        op == Operation::MinusEquals ||
        op == Operation::MulEquals ||
        op == Operation::DivEquals) {
        
        // For assignment operations: identifier = value (or +=, -=, etc.)
        // Stack needs: [value, identifier] for Store/assignment operations
        
        // The parser creates: Assign [value, identifier]
        // So child0 is the value and child1 is the identifier
        auto valueNode = node->GetChild(0);
        auto identNode = node->GetChild(1);
        
        // Translate the value first
        TranslateNode(valueNode);
        
        // Check if the left side is an array indexing operation
        if (identNode->GetType() == AstNodeEnum::IndexOp) {
            // Special handling for array element assignment: arr[index] = value
            KAI_TRACE() << "Handling indexed assignment";
            
            // For arr[2] = 99, we need to implement this differently
            // Since we don't have SetChild operation, we'll use a workaround
            
            // Get the array and index from IndexOp
            auto indexChildren = identNode->GetChildren();
            if (indexChildren.size() != 2) {
                KAI_TRACE_ERROR() << "IndexOp should have 2 children for assignment";
                return;
            }
            
            // Translate the array expression
            TranslateNode(indexChildren[0]);
            
            // Translate the index
            TranslateNode(indexChildren[1]);
            
            // The value is already on the stack from above
            // Stack now has: [value, array, index]
            
            // We need to rearrange to [array, index, value] for SetChild
            // Use stack manipulation operations: Rot rotates top 3 elements
            AppendDirectOperation(Operation::Rot);  // Now: [array, index, value]
            
            // Apply SetChild operation
            AppendDirectOperation(Operation::SetChild);
            
            // SetChild leaves the modified array on the stack
            // But Store expects [value, name], so we need to skip the Store operation
            return;  // Skip the Store operation below
            
        } else if (identNode->GetToken().type == RhoTokenEnumType::Label) {
            // Push the identifier name as a quoted Pathname for assignment
            KAI_TRACE() << "Appending pathname for assignment: " << identNode->Text();
            // Create a quoted pathname by prepending '
            String quotedPath = "'" + identNode->Text();
            auto pathObj = reg_->New<Pathname>(Pathname(quotedPath));
            KAI_TRACE() << "Created pathname object: " 
                        << (pathObj.Exists() ? "exists" : "null")
                        << ", type: " 
                        << (pathObj.GetClass() ? pathObj.GetClass()->GetName().ToString() : "<null>");
            Append(pathObj);
        } else {
            // If it's not a simple identifier or index op, translate it normally
            TranslateNode(identNode);
        }
    } else {
        // For other operations, use standard left-to-right order
        
        // Translate the left operand
        TranslateNode(node->GetChild(0));

        // Translate the right operand
        TranslateNode(node->GetChild(1));
    }

    // Add the operation directly to the current continuation
    AppendDirectOperation(op);

    KAI_TRACE() << "Binary operation successfully translated in linear stream";
}

KAI_END