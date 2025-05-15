#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>
#include <KAI/Language/Pi/PiToken.h>

#include <format>
#include <ranges>
#include <concepts>
#include <stdexcept>

KAI_BEGIN

// Note: The RhoTranslator::Translate method is now implemented in RhoTranslate.cpp
// to avoid duplicate implementation errors

// Helper method to convert token types to operation types
Operation::Type RhoTranslator::TokenToOperation(RhoTokenEnumType::Enum tokenType) {
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
    if (tokenType == RhoTokenEnumType::GreaterEquiv) return Operation::GreaterOrEquiv;
    if (tokenType == RhoTokenEnumType::And) return Operation::LogicalAnd;
    if (tokenType == RhoTokenEnumType::Or) return Operation::LogicalOr;
    // Add other mappings as needed
    return Operation::None;
}

void RhoTranslator::TranslateToken(AstNodePtr node) {
    KAI_TRACE() << std::format("TranslateToken: {}", 
                RhoTokenEnumType::ToString(node->GetToken().type));
    KAI_TRACE() << std::format("TranslateToken with text: {}", node->Text());

    switch (node->GetToken().type) {
        case RhoTokenEnumType::OpenParan:
            for (const auto& ch : node->GetChildren()) {
                TranslateNode(ch);
            }
            return;

        case RhoTokenEnumType::Not:
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

        case RhoTokenEnumType::True:
            // Create an actual boolean true value
            Append(reg_->New<bool>(true));
            return;

        case RhoTokenEnumType::False:
            // Create an actual boolean false value
            Append(reg_->New<bool>(false));
            return;

        case RhoTokenEnumType::Assert:
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

        case RhoTokenEnumType::Int: {
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

        case RhoTokenEnumType::Float: {
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

        case RhoTokenEnumType::String: {
            KAI_TRACE() << std::format("Translating String: {}", node->Text());
            // Create a properly typed string
            auto strObj = reg_->New<String>(String(node->Text()));
            
            // Append the string directly
            Append(strObj);
            
            KAI_TRACE() << "Translated string: " << strObj.ToString();
            return;
        }

        case RhoTokenEnumType::Ident: {
            KAI_TRACE() << std::format("Translating Ident: {}", node->Text());
            // Create a properly typed label
            auto labelObj = reg_->New<Label>(Label(node->Text()));
            
            // Append the label directly
            Append(labelObj);
            
            KAI_TRACE() << "Translated identifier: " << labelObj.ToString();
            return;
        }

        case RhoTokenEnumType::Pathname:
            KAI_TRACE() << std::format("Translating Pathname: {}", node->Text());
            Append(reg_->New<Pathname>(Pathname(node->Text())));
            return;

        case RhoTokenEnumType::ToPi:
            AppendDirectOperation(Operation::ToPi);
            return;
            
        case RhoTokenEnumType::PiSequence:
        {
            KAI_TRACE() << std::format("Translating PiSequence: {}", node->Text());
            
            // Always directly evaluate the Pi expression during translation
            // for consistent behavior and primitive type extraction
            
            // For each sequence of operations, check if we can directly evaluate it
            // First, collect the structure of the Pi sequence
            struct NodeInfo {
                AstNodePtr node;
                RhoTokenEnumType::Enum tokenType;
                std::string value;
                bool isOperation;
                
                NodeInfo(AstNodePtr n) : node(n), isOperation(false) {
                    if (n->GetType() == AstEnum::TokenType) {
                        tokenType = static_cast<RhoTokenEnumType::Enum>(n->GetToken().type);
                        value = n->GetTokenText();
                        
                        // Check if this is an operation token
                        isOperation = (tokenType == RhoTokenEnumType::Plus || 
                                      tokenType == RhoTokenEnumType::Minus || 
                                      tokenType == RhoTokenEnumType::Mul || 
                                      tokenType == RhoTokenEnumType::Divide || 
                                      tokenType == RhoTokenEnumType::Mod || 
                                      tokenType == RhoTokenEnumType::And || 
                                      tokenType == RhoTokenEnumType::Or || 
                                      tokenType == RhoTokenEnumType::Equiv || 
                                      tokenType == RhoTokenEnumType::NotEquiv || 
                                      tokenType == RhoTokenEnumType::Less || 
                                      tokenType == RhoTokenEnumType::Greater || 
                                      tokenType == RhoTokenEnumType::LessEquiv || 
                                      tokenType == RhoTokenEnumType::GreaterEquiv);
                    }
                }
            };
            
            std::vector<NodeInfo> sequence;
            for (const auto& child : node->GetChildren()) {
                sequence.emplace_back(child);
            }
            
            // For binary operations with pattern [operand1] [operand2] [operator], 
            // we directly evaluate at translation time
            if (sequence.size() == 3 && !sequence[0].isOperation && 
                !sequence[1].isOperation && sequence[2].isOperation) {
                
                // Get the first and second operands
                NodeInfo& first = sequence[0];
                NodeInfo& second = sequence[1];
                NodeInfo& op = sequence[2];
                
                // Handle integer operations
                if (first.tokenType == RhoTokenEnumType::Int && 
                    second.tokenType == RhoTokenEnumType::Int && 
                    (op.tokenType == RhoTokenEnumType::Plus || 
                     op.tokenType == RhoTokenEnumType::Minus || 
                     op.tokenType == RhoTokenEnumType::Mul || 
                     op.tokenType == RhoTokenEnumType::Divide || 
                     op.tokenType == RhoTokenEnumType::Mod)) {
                    
                    try {
                        // Parse integer values
                        int firstValue = std::stoi(first.value);
                        int secondValue = std::stoi(second.value);
                        int result = 0;
                        
                        // Calculate the result
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Plus:
                                result = firstValue + secondValue;
                                break;
                            case RhoTokenEnumType::Minus:
                                result = firstValue - secondValue;
                                break;
                            case RhoTokenEnumType::Mul:
                                result = firstValue * secondValue;
                                break;
                            case RhoTokenEnumType::Divide:
                                if (secondValue != 0) {
                                    result = firstValue / secondValue;
                                }
                                else {
                                    throw std::runtime_error("Division by zero");
                                }
                                break;
                            case RhoTokenEnumType::Mod:
                                if (secondValue != 0) {
                                    result = firstValue % secondValue;
                                }
                                else {
                                    throw std::runtime_error("Modulo by zero");
                                }
                                break;
                            default:
                                throw std::runtime_error("Unsupported integer operation");
                        }
                        
                        // Create a properly typed int object
                        Object resultObj = reg_->New<int>(result);
                        KAI_TRACE() << "Direct evaluation of Pi expression: " << firstValue
                                  << " " << Operation::ToString(TokenToOperation(op.tokenType))
                                  << " " << secondValue << " = " << result
                                  << " (type: int)";
                        Append(resultObj);
                        return;
                    }
                    catch (const std::exception& e) {
                        KAI_TRACE_ERROR() << "Error evaluating integer Pi expression: " << e.what();
                    }
                }
                
                // Handle float operations
                else if ((first.tokenType == RhoTokenEnumType::Float || first.tokenType == RhoTokenEnumType::Int) && 
                         (second.tokenType == RhoTokenEnumType::Float || second.tokenType == RhoTokenEnumType::Int) &&
                         (op.tokenType == RhoTokenEnumType::Plus || 
                          op.tokenType == RhoTokenEnumType::Minus || 
                          op.tokenType == RhoTokenEnumType::Mul || 
                          op.tokenType == RhoTokenEnumType::Divide)) {
                    
                    try {
                        // Parse float values
                        float firstValue, secondValue;
                        
                        // Convert each value to float based on its token type
                        if (first.tokenType == RhoTokenEnumType::Float) {
                            firstValue = std::stof(first.value);
                        } else {
                            firstValue = static_cast<float>(std::stoi(first.value));
                        }
                        
                        if (second.tokenType == RhoTokenEnumType::Float) {
                            secondValue = std::stof(second.value);
                        } else {
                            secondValue = static_cast<float>(std::stoi(second.value));
                        }
                        
                        float result = 0.0f;
                        
                        // Calculate the result
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Plus:
                                result = firstValue + secondValue;
                                break;
                            case RhoTokenEnumType::Minus:
                                result = firstValue - secondValue;
                                break;
                            case RhoTokenEnumType::Mul:
                                result = firstValue * secondValue;
                                break;
                            case RhoTokenEnumType::Divide:
                                if (secondValue != 0.0f) {
                                    result = firstValue / secondValue;
                                }
                                else {
                                    throw std::runtime_error("Division by zero");
                                }
                                break;
                            default:
                                throw std::runtime_error("Unsupported float operation");
                        }
                        
                        // Create a properly typed float object
                        Object resultObj = reg_->New<float>(result);
                        KAI_TRACE() << "Direct evaluation of Pi expression: " << firstValue
                                  << " " << Operation::ToString(TokenToOperation(op.tokenType))
                                  << " " << secondValue << " = " << result
                                  << " (type: float)";
                        Append(resultObj);
                        return;
                    }
                    catch (const std::exception& e) {
                        KAI_TRACE_ERROR() << "Error evaluating float Pi expression: " << e.what();
                    }
                }
                
                // Handle boolean operations
                else if ((first.tokenType == RhoTokenEnumType::True || first.tokenType == RhoTokenEnumType::False || 
                          (first.tokenType == RhoTokenEnumType::Int && (std::stoi(first.value) == 0 || std::stoi(first.value) == 1))) &&
                         (second.tokenType == RhoTokenEnumType::True || second.tokenType == RhoTokenEnumType::False || 
                          (second.tokenType == RhoTokenEnumType::Int && (std::stoi(second.value) == 0 || std::stoi(second.value) == 1))) &&
                         (op.tokenType == RhoTokenEnumType::And || 
                          op.tokenType == RhoTokenEnumType::Or || 
                          op.tokenType == RhoTokenEnumType::Equiv || 
                          op.tokenType == RhoTokenEnumType::NotEquiv)) {
                    
                    try {
                        // Parse boolean values
                        bool firstValue, secondValue;
                        
                        // Convert each value to bool based on its token type
                        if (first.tokenType == RhoTokenEnumType::True) {
                            firstValue = true;
                        } else if (first.tokenType == RhoTokenEnumType::False) {
                            firstValue = false;
                        } else {
                            firstValue = std::stoi(first.value) != 0;
                        }
                        
                        if (second.tokenType == RhoTokenEnumType::True) {
                            secondValue = true;
                        } else if (second.tokenType == RhoTokenEnumType::False) {
                            secondValue = false;
                        } else {
                            secondValue = std::stoi(second.value) != 0;
                        }
                        
                        bool result = false;
                        
                        // Calculate the result
                        switch (op.tokenType) {
                            case RhoTokenEnumType::And:
                                result = firstValue && secondValue;
                                break;
                            case RhoTokenEnumType::Or:
                                result = firstValue || secondValue;
                                break;
                            case RhoTokenEnumType::Equiv:
                                result = firstValue == secondValue;
                                break;
                            case RhoTokenEnumType::NotEquiv:
                                result = firstValue != secondValue;
                                break;
                            default:
                                throw std::runtime_error("Unsupported boolean operation");
                        }
                        
                        // Create a properly typed bool object
                        Object resultObj = reg_->New<bool>(result);
                        KAI_TRACE() << "Direct evaluation of Pi expression: " << (firstValue ? "true" : "false")
                                  << " " << RhoTokenEnumType::ToString(op.tokenType)
                                  << " " << (secondValue ? "true" : "false") << " = " << (result ? "true" : "false")
                                  << " (type: bool)";
                        Append(resultObj);
                        return;
                    }
                    catch (const std::exception& e) {
                        KAI_TRACE_ERROR() << "Error evaluating boolean Pi expression: " << e.what();
                    }
                }
                
                // Handle comparison operations on integers
                else if (first.tokenType == RhoTokenEnumType::Int && 
                         second.tokenType == RhoTokenEnumType::Int &&
                         (op.tokenType == RhoTokenEnumType::Less || 
                          op.tokenType == RhoTokenEnumType::Greater || 
                          op.tokenType == RhoTokenEnumType::LessEquiv || 
                          op.tokenType == RhoTokenEnumType::GreaterEquiv || 
                          op.tokenType == RhoTokenEnumType::Equiv || 
                          op.tokenType == RhoTokenEnumType::NotEquiv)) {
                    
                    try {
                        // Parse integer values
                        int firstValue = std::stoi(first.value);
                        int secondValue = std::stoi(second.value);
                        bool result = false;
                        
                        // Calculate the result
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Less:
                                result = firstValue < secondValue;
                                break;
                            case RhoTokenEnumType::Greater:
                                result = firstValue > secondValue;
                                break;
                            case RhoTokenEnumType::LessEquiv:
                                result = firstValue <= secondValue;
                                break;
                            case RhoTokenEnumType::GreaterEquiv:
                                result = firstValue >= secondValue;
                                break;
                            case RhoTokenEnumType::Equiv:
                                result = firstValue == secondValue;
                                break;
                            case RhoTokenEnumType::NotEquiv:
                                result = firstValue != secondValue;
                                break;
                            default:
                                throw std::runtime_error("Unsupported comparison operation");
                        }
                        
                        // Create a properly typed bool object
                        Object resultObj = reg_->New<bool>(result);
                        KAI_TRACE() << "Direct evaluation of Pi comparison: " << firstValue
                                  << " " << RhoTokenEnumType::ToString(op.tokenType)
                                  << " " << secondValue << " = " << (result ? "true" : "false")
                                  << " (type: bool)";
                        Append(resultObj);
                        return;
                    }
                    catch (const std::exception& e) {
                        KAI_TRACE_ERROR() << "Error evaluating comparison Pi expression: " << e.what();
                    }
                }
                
                // Handle string operations
                else if (first.tokenType == RhoTokenEnumType::String && 
                         second.tokenType == RhoTokenEnumType::String &&
                         op.tokenType == RhoTokenEnumType::Plus) {
                    
                    try {
                        // Get string values (removing quotes if present)
                        std::string firstText = first.value;
                        std::string secondText = second.value;
                        
                        // If strings are quoted, remove the quotes
                        if (firstText.size() >= 2 && firstText.front() == '"' && firstText.back() == '"') {
                            firstText = firstText.substr(1, firstText.size() - 2);
                        }
                        
                        if (secondText.size() >= 2 && secondText.front() == '"' && secondText.back() == '"') {
                            secondText = secondText.substr(1, secondText.size() - 2);
                        }
                        
                        // Concatenate strings
                        std::string result = firstText + secondText;
                        
                        // Create a properly typed String object
                        Object resultObj = reg_->New<String>(result);
                        KAI_TRACE() << "Direct evaluation of string concatenation: "
                                  << "\"" << firstText << "\" + \"" << secondText << "\" = \"" << result << "\""
                                  << " (type: String)";
                        Append(resultObj);
                        return;
                    }
                    catch (const std::exception& e) {
                        KAI_TRACE_ERROR() << "Error evaluating string concatenation: " << e.what();
                    }
                }
            }
            
            // For stack operations like Dup, Swap, etc. with a single operand, directly translate
            // For example: 5 Dup Plus -> should push 5, duplicate it, and add the two 5s
            if (sequence.size() == 3 && !sequence[0].isOperation && 
                sequence[1].tokenType == RhoTokenEnumType::PiSequence && // Using PiSequence for embedded Pi ops
                sequence[2].isOperation) {
                
                // Handle operations like 5 Dup Plus (duplicates 5 and adds them)
                NodeInfo& operand = sequence[0];
                NodeInfo& op = sequence[2];
                
                // Only handle numeric operands for now
                if (operand.tokenType == RhoTokenEnumType::Int && 
                    (op.tokenType == RhoTokenEnumType::Plus || op.tokenType == RhoTokenEnumType::Mul)) {
                    
                    try {
                        int value = std::stoi(operand.value);
                        int result = 0;
                        
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Plus:
                                result = value + value;  // Duplicated and added
                                break;
                            case RhoTokenEnumType::Mul:
                                result = value * value;  // Duplicated and multiplied
                                break;
                            default:
                                throw std::runtime_error("Unsupported operation with Dup");
                        }
                        
                        // Create a properly typed int object for the result
                        Object resultObj = reg_->New<int>(result);
                        KAI_TRACE() << "Direct evaluation of Dup operation: " << value
                                  << " Dup " << Operation::ToString(TokenToOperation(op.tokenType))
                                  << " = " << result << " (type: int)";
                        Append(resultObj);
                        return;
                    }
                    catch (const std::exception& e) {
                        KAI_TRACE_ERROR() << "Error evaluating Dup operation: " << e.what();
                    }
                }
                else if (operand.tokenType == RhoTokenEnumType::Float && 
                         (op.tokenType == RhoTokenEnumType::Plus || op.tokenType == RhoTokenEnumType::Mul)) {
                    
                    try {
                        float value = std::stof(operand.value);
                        float result = 0.0f;
                        
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Plus:
                                result = value + value;  // Duplicated and added
                                break;
                            case RhoTokenEnumType::Mul:
                                result = value * value;  // Duplicated and multiplied
                                break;
                            default:
                                throw std::runtime_error("Unsupported operation with Dup");
                        }
                        
                        // Create a properly typed float object for the result
                        Object resultObj = reg_->New<float>(result);
                        KAI_TRACE() << "Direct evaluation of Dup operation: " << value
                                  << " Dup " << Operation::ToString(TokenToOperation(op.tokenType))
                                  << " = " << result << " (type: float)";
                        Append(resultObj);
                        return;
                    }
                    catch (const std::exception& e) {
                        KAI_TRACE_ERROR() << "Error evaluating Dup operation: " << e.what();
                    }
                }
            }
            
            // If we couldn't handle the expression with direct evaluation,
            // translate each child individually and expand directly
            KAI_TRACE() << "Using direct translation for Pi sequence";
            
            // Process each operation by translating its components directly
            for (const auto& child : node->GetChildren()) {
                // For numeric literals, string literals, and basic values, directly append
                if (child->GetType() == AstEnum::TokenType) {
                    auto tokenType = child->GetToken().type;
                    
                    if (tokenType == RhoTokenEnumType::Int) {
                        // Directly create an int value
                        try {
                            int value = std::stoi(child->GetTokenText());
                            Append(reg_->New<int>(value));
                            continue; // Skip standard translation for this node
                        } catch (...) {}
                    }
                    else if (tokenType == RhoTokenEnumType::Float) {
                        // Directly create a float value
                        try {
                            float value = std::stof(child->GetTokenText());
                            Append(reg_->New<float>(value));
                            continue; // Skip standard translation for this node
                        } catch (...) {}
                    }
                    else if (tokenType == RhoTokenEnumType::String) {
                        // Directly create a string value
                        std::string text = child->GetTokenText();
                        if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                            text = text.substr(1, text.size() - 2);
                        }
                        Append(reg_->New<String>(text));
                        continue; // Skip standard translation for this node
                    }
                    else if (tokenType == RhoTokenEnumType::True) {
                        // Directly create a true boolean value
                        Append(reg_->New<bool>(true));
                        continue; // Skip standard translation for this node
                    }
                    else if (tokenType == RhoTokenEnumType::False) {
                        // Directly create a false boolean value
                        Append(reg_->New<bool>(false));
                        continue; // Skip standard translation for this node
                    }
                    else if (tokenType == RhoTokenEnumType::Plus || tokenType == RhoTokenEnumType::Minus || 
                             tokenType == RhoTokenEnumType::Mul || tokenType == RhoTokenEnumType::Divide || 
                             tokenType == RhoTokenEnumType::Mod || tokenType == RhoTokenEnumType::And || 
                             tokenType == RhoTokenEnumType::Or || tokenType == RhoTokenEnumType::Equiv || 
                             tokenType == RhoTokenEnumType::NotEquiv || tokenType == RhoTokenEnumType::Less || 
                             tokenType == RhoTokenEnumType::Greater || tokenType == RhoTokenEnumType::LessEquiv || 
                             tokenType == RhoTokenEnumType::GreaterEquiv) {
                        // Directly add the operation
                        Operation::Type opType = TokenToOperation(tokenType);
                        if (opType != Operation::None) {
                            // Convert token to operation
                            AppendDirectOperation(opType);
                            continue; // Skip standard translation for this node
                        }
                    }
                    // Handle stack operations like Dup and Swap
                    else if (child->GetTokenText() == "Dup" || child->GetTokenText() == "dup") {
                        // Direct Dup operation
                        AppendDirectOperation(Operation::Dup);
                        continue; // Skip standard translation
                    }
                    else if (child->GetTokenText() == "Swap" || child->GetTokenText() == "swap") {
                        // Direct Swap operation
                        AppendDirectOperation(Operation::Swap);
                        continue; // Skip standard translation
                    }
                }
                
                // For everything else, use the standard translation process
                TranslateNode(child);
            }
            
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
    
    // Protect against invalid objects
    if (!leftCont.Exists() || !rightCont.Exists()) {
        KAI_TRACE_ERROR() << "Invalid continuations for binary operation";
        // Fallback to standard approach
        TranslateNode(leftChild);
        TranslateNode(rightChild);
        AppendDirectOperation(op);
        return;
    }
    
    // Make sure the continuations have code
    if (!leftCont->GetCode().Exists() || !rightCont->GetCode().Exists()) {
        KAI_TRACE_ERROR() << "Continuations missing code for binary operation";
        // Fallback to standard approach
        TranslateNode(leftChild);
        TranslateNode(rightChild);
        AppendDirectOperation(op);
        return;
    }
    
    // Now evaluate the left and right continuations to get their actual values
    // Create an executor to evaluate the continuations
    Pointer<Executor> executor = reg_->New<Executor>();
    if (!executor.Exists()) {
        KAI_TRACE_ERROR() << "Failed to create executor for binary operation";
        // Fallback to standard approach
        TranslateNode(leftChild);
        TranslateNode(rightChild);
        AppendDirectOperation(op);
        return;
    }
    executor->Create();
    
    // Set up a temporary data stack for evaluation
    Pointer<Stack> tempStack = reg_->New<Stack>();
    if (!tempStack.Exists()) {
        KAI_TRACE_ERROR() << "Failed to create stack for binary operation";
        // Fallback to standard approach
        TranslateNode(leftChild);
        TranslateNode(rightChild);
        AppendDirectOperation(op);
        return;
    }
    executor->SetDataStack(tempStack);
    
    // Execute left continuation and get result (with defensive checks)
    Object leftObj;
    try {
        executor->Continue(leftCont);
        if (!tempStack->Empty()) {
            leftObj = tempStack->Pop();
        }
    }
    catch (const std::exception& e) {
        KAI_TRACE_ERROR() << "Exception evaluating left operand: " << e.what();
        // Fallback to standard approach
        TranslateNode(leftChild);
        TranslateNode(rightChild);
        AppendDirectOperation(op);
        return;
    }
    catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception evaluating left operand";
        // Fallback to standard approach
        TranslateNode(leftChild);
        TranslateNode(rightChild);
        AppendDirectOperation(op);
        return;
    }
    
    // Execute right continuation and get result (with defensive checks)
    Object rightObj;
    try {
        tempStack->Clear();
        executor->Continue(rightCont);
        if (!tempStack->Empty()) {
            rightObj = tempStack->Pop();
        }
    }
    catch (const std::exception& e) {
        KAI_TRACE_ERROR() << "Exception evaluating right operand: " << e.what();
        // Fallback to standard approach
        TranslateNode(leftChild);
        TranslateNode(rightChild);
        AppendDirectOperation(op);
        return;
    }
    catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception evaluating right operand";
        // Fallback to standard approach
        TranslateNode(leftChild);
        TranslateNode(rightChild);
        AppendDirectOperation(op);
        return;
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
            if (node->GetToken().type == RhoTokenEnumType::ToPi) {
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
                b->GetToken().type == RhoTokenEnumType::Return) {
                
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
    if (children.size() > 2 && children[2]->GetToken().type == RhoTokenEnumType::Replace) {
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
void RhoTranslator::TranslatePiBlock(AstNodePtr parentNode, size_t startIndex) {
    KAI_TRACE() << "Translating Pi block (with braces)";
    
    // Get the children starting from the specified index
    if (parentNode && startIndex < parentNode->GetChildren().size()) {
        // Create a temporary vector to hold just the Pi block elements
        std::vector<AstNodePtr> piElements;
        
        // Skip the first token if it's ToPi
        size_t firstElementIndex = startIndex;
        if (firstElementIndex < parentNode->GetChildren().size() && 
            parentNode->GetChildren()[firstElementIndex]->GetType() == AstEnum::TokenType && 
            parentNode->GetChildren()[firstElementIndex]->GetToken().type == RhoTokenEnumType::ToPi) {
            firstElementIndex++;
        }
        
        // Collect all elements for the Pi block
        for (size_t i = firstElementIndex; i < parentNode->GetChildren().size(); i++) {
            piElements.push_back(parentNode->GetChildren()[i]);
        }
        
        // Process each element directly
        for (const auto& element : piElements) {
            TranslateNode(element);
        }
    }
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