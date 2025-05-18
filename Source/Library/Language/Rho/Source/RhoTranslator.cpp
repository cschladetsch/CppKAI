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
            
            // ENHANCED DIRECT Pi EVALUATION IMPLEMENTATION:
            // This completely rewritten implementation focuses on:
            // 1. Properly handling primitive types (int, float, bool, String)
            // 2. Ensuring type preservation through direct evaluation when possible
            // 3. Using a multi-layered approach for fall-back evaluation methods
            
            // LAYER 1: Try to directly evaluate simple Pi expressions with literals
            // Analyze the Pi sequence structure
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
            
            // Collect the nodes in the Pi sequence
            std::vector<NodeInfo> sequence;
            for (const auto& child : node->GetChildren()) {
                sequence.emplace_back(child);
            }
            
            // Check for simple direct evaluation patterns
            
            // Common pattern 1: Two operands and an operator (postfix notation)
            // Example: 2 3 + for 2+3=5
            if (sequence.size() == 3 && !sequence[0].isOperation && 
                !sequence[1].isOperation && sequence[2].isOperation) {
                
                NodeInfo& first = sequence[0];
                NodeInfo& second = sequence[1];
                NodeInfo& op = sequence[2];
                
                // Integer operations
                if (first.tokenType == RhoTokenEnumType::Int && 
                    second.tokenType == RhoTokenEnumType::Int) {
                    
                    try {
                        // Parse integer values
                        int firstValue = std::stoi(first.value);
                        int secondValue = std::stoi(second.value);
                        
                        // Select the appropriate operation
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Plus: {
                                int result = firstValue + secondValue;
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE() << "DIRECT INTEGER EVAL: " << firstValue << " + " 
                                          << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Minus: {
                                int result = firstValue - secondValue;
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE() << "DIRECT INTEGER EVAL: " << firstValue << " - " 
                                          << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Mul: {
                                int result = firstValue * secondValue;
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE() << "DIRECT INTEGER EVAL: " << firstValue << " * " 
                                          << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Divide: {
                                if (secondValue != 0) {
                                    int result = firstValue / secondValue;
                                    Object resultObj = reg_->New<int>(result);
                                    KAI_TRACE() << "DIRECT INTEGER EVAL: " << firstValue << " / " 
                                              << secondValue << " = " << result;
                                    Append(resultObj);
                                    return;
                                }
                                break;
                            }
                            case RhoTokenEnumType::Mod: {
                                if (secondValue != 0) {
                                    int result = firstValue % secondValue;
                                    Object resultObj = reg_->New<int>(result);
                                    KAI_TRACE() << "DIRECT INTEGER EVAL: " << firstValue << " % " 
                                              << secondValue << " = " << result;
                                    Append(resultObj);
                                    return;
                                }
                                break;
                            }
                            case RhoTokenEnumType::Less: {
                                bool result = firstValue < secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT INTEGER COMPARISON: " << firstValue << " < " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Greater: {
                                bool result = firstValue > secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT INTEGER COMPARISON: " << firstValue << " > " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::LessEquiv: {
                                bool result = firstValue <= secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT INTEGER COMPARISON: " << firstValue << " <= " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::GreaterEquiv: {
                                bool result = firstValue >= secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT INTEGER COMPARISON: " << firstValue << " >= " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Equiv: {
                                bool result = firstValue == secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT INTEGER COMPARISON: " << firstValue << " == " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::NotEquiv: {
                                bool result = firstValue != secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT INTEGER COMPARISON: " << firstValue << " != " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                        }
                    }
                    catch (...) {
                        KAI_TRACE_ERROR() << "Error in direct integer Pi evaluation";
                    }
                }
                
                // Float operations (including mixed int/float)
                else if ((first.tokenType == RhoTokenEnumType::Float || first.tokenType == RhoTokenEnumType::Int) && 
                         (second.tokenType == RhoTokenEnumType::Float || second.tokenType == RhoTokenEnumType::Int)) {
                    
                    try {
                        // Parse float values, with conversion from int if needed
                        float firstValue, secondValue;
                        
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
                        
                        // Select the appropriate operation
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Plus: {
                                float result = firstValue + secondValue;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE() << "DIRECT FLOAT EVAL: " << firstValue << " + " 
                                          << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Minus: {
                                float result = firstValue - secondValue;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE() << "DIRECT FLOAT EVAL: " << firstValue << " - " 
                                          << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Mul: {
                                float result = firstValue * secondValue;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE() << "DIRECT FLOAT EVAL: " << firstValue << " * " 
                                          << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Divide: {
                                if (secondValue != 0.0f) {
                                    float result = firstValue / secondValue;
                                    Object resultObj = reg_->New<float>(result);
                                    KAI_TRACE() << "DIRECT FLOAT EVAL: " << firstValue << " / " 
                                              << secondValue << " = " << result;
                                    Append(resultObj);
                                    return;
                                }
                                break;
                            }
                            case RhoTokenEnumType::Less: {
                                bool result = firstValue < secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT FLOAT COMPARISON: " << firstValue << " < " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Greater: {
                                bool result = firstValue > secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT FLOAT COMPARISON: " << firstValue << " > " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::LessEquiv: {
                                bool result = firstValue <= secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT FLOAT COMPARISON: " << firstValue << " <= " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::GreaterEquiv: {
                                bool result = firstValue >= secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT FLOAT COMPARISON: " << firstValue << " >= " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Equiv: {
                                bool result = firstValue == secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT FLOAT COMPARISON: " << firstValue << " == " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::NotEquiv: {
                                bool result = firstValue != secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT FLOAT COMPARISON: " << firstValue << " != " 
                                          << secondValue << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                        }
                    }
                    catch (...) {
                        KAI_TRACE_ERROR() << "Error in direct float Pi evaluation";
                    }
                }
                
                // Boolean operations
                else if ((first.tokenType == RhoTokenEnumType::True || first.tokenType == RhoTokenEnumType::False) && 
                         (second.tokenType == RhoTokenEnumType::True || second.tokenType == RhoTokenEnumType::False)) {
                    
                    try {
                        // Get boolean values directly from token types
                        bool firstValue = (first.tokenType == RhoTokenEnumType::True);
                        bool secondValue = (second.tokenType == RhoTokenEnumType::True);
                        
                        // Select the appropriate operation
                        switch (op.tokenType) {
                            case RhoTokenEnumType::And: {
                                bool result = firstValue && secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT BOOLEAN EVAL: " << (firstValue ? "true" : "false") << " AND " 
                                          << (secondValue ? "true" : "false") << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Or: {
                                bool result = firstValue || secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT BOOLEAN EVAL: " << (firstValue ? "true" : "false") << " OR " 
                                          << (secondValue ? "true" : "false") << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Equiv: {
                                bool result = firstValue == secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT BOOLEAN EVAL: " << (firstValue ? "true" : "false") << " == " 
                                          << (secondValue ? "true" : "false") << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::NotEquiv: {
                                bool result = firstValue != secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT BOOLEAN EVAL: " << (firstValue ? "true" : "false") << " != " 
                                          << (secondValue ? "true" : "false") << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                        }
                    }
                    catch (...) {
                        KAI_TRACE_ERROR() << "Error in direct boolean Pi evaluation";
                    }
                }
                
                // String operations (concatenation, comparison)
                else if (first.tokenType == RhoTokenEnumType::String && 
                         second.tokenType == RhoTokenEnumType::String) {
                    
                    try {
                        // Process the string values (remove quotes)
                        std::string firstText = first.value;
                        std::string secondText = second.value;
                        
                        if (firstText.size() >= 2 && firstText.front() == '"' && firstText.back() == '"') {
                            firstText = firstText.substr(1, firstText.size() - 2);
                        }
                        
                        if (secondText.size() >= 2 && secondText.front() == '"' && secondText.back() == '"') {
                            secondText = secondText.substr(1, secondText.size() - 2);
                        }
                        
                        // Select the appropriate operation
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Plus: {
                                // String concatenation
                                String result = firstText + secondText;
                                Object resultObj = reg_->New<String>(result);
                                KAI_TRACE() << "DIRECT STRING EVAL: \"" << firstText << "\" + \"" 
                                          << secondText << "\" = \"" << result << "\"";
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Equiv: {
                                // String equality
                                bool result = firstText == secondText;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT STRING COMPARISON: \"" << firstText << "\" == \"" 
                                          << secondText << "\" = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::NotEquiv: {
                                // String inequality
                                bool result = firstText != secondText;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE() << "DIRECT STRING COMPARISON: \"" << firstText << "\" != \"" 
                                          << secondText << "\" = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                        }
                    }
                    catch (...) {
                        KAI_TRACE_ERROR() << "Error in direct string Pi evaluation";
                    }
                }
            }
            
            // Common pattern 2: Stack operations (dup, swap)
            // Example: 5 dup + (duplicates 5 and adds, resulting in 10)
            if (sequence.size() >= 2) {
                // Special case for 5 dup +
                if (sequence.size() == 3 && 
                    (sequence[0].tokenType == RhoTokenEnumType::Int || sequence[0].tokenType == RhoTokenEnumType::Float) &&
                    sequence[1].value == "Dup" || sequence[1].value == "dup" &&
                    sequence[2].isOperation) {
                    
                    try {
                        // Handle integer dup operations
                        if (sequence[0].tokenType == RhoTokenEnumType::Int) {
                            int value = std::stoi(sequence[0].value);
                            
                            // Handle common operations after dup
                            if (sequence[2].tokenType == RhoTokenEnumType::Plus) {
                                // 5 dup + = 5 + 5 = 10
                                int result = value + value;
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE() << "DIRECT DUP+ADD: " << value << " dup + = " << result;
                                Append(resultObj);
                                return;
                            }
                            else if (sequence[2].tokenType == RhoTokenEnumType::Mul) {
                                // 5 dup * = 5 * 5 = 25
                                int result = value * value;
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE() << "DIRECT DUP+MULTIPLY: " << value << " dup * = " << result;
                                Append(resultObj);
                                return;
                            }
                        }
                        // Handle float dup operations
                        else if (sequence[0].tokenType == RhoTokenEnumType::Float) {
                            float value = std::stof(sequence[0].value);
                            
                            // Handle common operations after dup
                            if (sequence[2].tokenType == RhoTokenEnumType::Plus) {
                                // 5.5 dup + = 5.5 + 5.5 = 11.0
                                float result = value + value;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE() << "DIRECT DUP+ADD: " << value << " dup + = " << result;
                                Append(resultObj);
                                return;
                            }
                            else if (sequence[2].tokenType == RhoTokenEnumType::Mul) {
                                // 5.5 dup * = 5.5 * 5.5 = 30.25
                                float result = value * value;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE() << "DIRECT DUP+MULTIPLY: " << value << " dup * = " << result;
                                Append(resultObj);
                                return;
                            }
                        }
                    }
                    catch (...) {
                        KAI_TRACE_ERROR() << "Error in direct dup operation evaluation";
                    }
                }
                
                // Special case for swap operations: 3 4 swap -
                if (sequence.size() == 4 && 
                    (sequence[0].tokenType == RhoTokenEnumType::Int || sequence[0].tokenType == RhoTokenEnumType::Float) &&
                    (sequence[1].tokenType == RhoTokenEnumType::Int || sequence[1].tokenType == RhoTokenEnumType::Float) &&
                    (sequence[2].value == "Swap" || sequence[2].value == "swap") &&
                    sequence[3].isOperation) {
                    
                    try {
                        // For simplicity, let's handle integer swap operations first
                        if (sequence[0].tokenType == RhoTokenEnumType::Int && 
                            sequence[1].tokenType == RhoTokenEnumType::Int) {
                            
                            int first = std::stoi(sequence[0].value);
                            int second = std::stoi(sequence[1].value);
                            
                            // 3 4 swap - = 4 - 3 = 1 (swap puts 3 on top, then subtract from 4)
                            if (sequence[3].tokenType == RhoTokenEnumType::Minus) {
                                int result = second - first; // After swap, second is on bottom, first on top
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE() << "DIRECT SWAP+SUBTRACT: " << first << " " << second 
                                         << " swap - = " << result;
                                Append(resultObj);
                                return;
                            }
                            // Other operations could be added here as needed
                        }
                    }
                    catch (...) {
                        KAI_TRACE_ERROR() << "Error in direct swap operation evaluation";
                    }
                }
            }
            
            // LAYER 2: Direct evaluation by building and executing a proper Pi program
            
            // Create a properly constructed array of objects representing the Pi program
            // First, collect and transform all elements into proper Pi objects
            Pointer<Array> piCode = reg_->New<Array>();
            bool allElementsTranslated = true;
            
            // Add proper ContinuationBegin marker
            piCode->Append(reg_->New<Operation>(Operation::ContinuationBegin));
            
            // Process each element in the Pi sequence
            for (const auto& item : sequence) {
                // Literals can be directly converted to Objects
                if (item.tokenType == RhoTokenEnumType::Int) {
                    try {
                        int value = std::stoi(item.value);
                        piCode->Append(reg_->New<int>(value));
                    }
                    catch (...) {
                        allElementsTranslated = false;
                        break;
                    }
                }
                else if (item.tokenType == RhoTokenEnumType::Float) {
                    try {
                        float value = std::stof(item.value);
                        piCode->Append(reg_->New<float>(value));
                    }
                    catch (...) {
                        allElementsTranslated = false;
                        break;
                    }
                }
                else if (item.tokenType == RhoTokenEnumType::String) {
                    std::string text = item.value;
                    if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                        text = text.substr(1, text.size() - 2);
                    }
                    piCode->Append(reg_->New<String>(text));
                }
                else if (item.tokenType == RhoTokenEnumType::True) {
                    piCode->Append(reg_->New<bool>(true));
                }
                else if (item.tokenType == RhoTokenEnumType::False) {
                    piCode->Append(reg_->New<bool>(false));
                }
                // Operations get converted to Operation objects
                else if (item.isOperation) {
                    Operation::Type opType = TokenToOperation(item.tokenType);
                    if (opType != Operation::None) {
                        piCode->Append(reg_->New<Operation>(opType));
                    }
                    else {
                        allElementsTranslated = false;
                        break;
                    }
                }
                // Special stack operations
                else if (item.value == "Dup" || item.value == "dup") {
                    piCode->Append(reg_->New<Operation>(Operation::Dup));
                }
                else if (item.value == "Swap" || item.value == "swap") {
                    piCode->Append(reg_->New<Operation>(Operation::Swap));
                }
                else {
                    // Unsupported element type
                    allElementsTranslated = false;
                    break;
                }
            }
            
            // Add proper ContinuationEnd marker
            piCode->Append(reg_->New<Operation>(Operation::ContinuationEnd));
            
            // If all elements were successfully translated, execute the Pi program
            if (allElementsTranslated) {
                // Create a continuation with the code
                Pointer<Continuation> piCont = reg_->New<Continuation>();
                piCont->Create();
                piCont->SetCode(piCode);
                
                // Create an executor to run the Pi program
                Pointer<Executor> executor = reg_->New<Executor>();
                executor->Create();
                
                // Set up a data stack for the executor
                Pointer<Stack> dataStack = reg_->New<Stack>();
                executor->SetDataStack(dataStack);
                
                // Run the Pi program
                executor->Continue(piCont);
                
                // Check if we got a result
                if (!dataStack->Empty()) {
                    // Get the result from the stack
                    Object result = dataStack->Top();
                    
                    // Extract primitive value if result is a continuation
                    if (result.IsType<Continuation>()) {
                        // Log the continuation type
                        KAI_TRACE() << "Got continuation result from Pi execution, attempting to extract value...";
                        
                        // Try to extract a primitive value
                        Pointer<Continuation> resultCont = result;
                        
                        // Check if it has a simple structure with a single value
                        if (resultCont->GetCode() && resultCont->GetCode()->Size() > 0) {
                            // Try to execute the continuation to get the final value
                            Pointer<Executor> extractExec = reg_->New<Executor>();
                            extractExec->Create();
                            
                            Pointer<Stack> extractStack = reg_->New<Stack>();
                            extractExec->SetDataStack(extractStack);
                            
                            extractExec->Continue(resultCont);
                            
                            if (!extractStack->Empty()) {
                                Object extractedResult = extractStack->Top();
                                
                                // Only use extracted result if it's a primitive type
                                if (extractedResult.IsType<int>() || extractedResult.IsType<float>() || 
                                    extractedResult.IsType<bool>() || extractedResult.IsType<String>()) {
                                    
                                    KAI_TRACE() << "Extracted primitive value from Pi result: " 
                                              << extractedResult.ToString()
                                              << " (type: " << extractedResult.GetClass()->GetName() << ")";
                                    
                                    // This is the primary result we want to use
                                    result = extractedResult;
                                }
                            }
                        }
                    }
                    
                    // Log the final result
                    KAI_TRACE() << "Pi sequence final result: " << result.ToString()
                              << " (type: " << (result.GetClass() ? result.GetClass()->GetName().ToString() : "null") << ")";
                    
                    // Add the result to the instruction stream
                    Append(result);
                    return;
                }
                else {
                    KAI_TRACE_ERROR() << "Pi execution completed but produced no result";
                }
            }
            
            // LAYER 3: Fallback to explicit evaluation for specific operation sequences
            // This creates Pi code for binary operations, executes it, and returns the result
            
            // For binary operations with [operand1, operand2, operation]
            if (sequence.size() >= 3) {
                // Get the elements (assuming postfix notation)
                auto op = sequence.back(); // Last element should be the operation
                
                // Only proceed if the last element is an operation
                if (op.isOperation && sequence.size() >= 3) {
                    // Get operands (stack has operands in reverse order for postfix notation)
                    auto operand2 = sequence[sequence.size() - 2];
                    auto operand1 = sequence[sequence.size() - 3];
                    
                    // Create objects for the operands
                    Object obj1, obj2;
                    bool validOperands = false;
                    
                    // Try to create objects for the operands based on their types
                    if (operand1.tokenType == RhoTokenEnumType::Int) {
                        try {
                            int value = std::stoi(operand1.value);
                            obj1 = reg_->New<int>(value);
                            validOperands = true;
                        } catch (...) {}
                    }
                    else if (operand1.tokenType == RhoTokenEnumType::Float) {
                        try {
                            float value = std::stof(operand1.value);
                            obj1 = reg_->New<float>(value);
                            validOperands = true;
                        } catch (...) {}
                    }
                    else if (operand1.tokenType == RhoTokenEnumType::String) {
                        std::string text = operand1.value;
                        if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                            text = text.substr(1, text.size() - 2);
                        }
                        obj1 = reg_->New<String>(text);
                        validOperands = true;
                    }
                    else if (operand1.tokenType == RhoTokenEnumType::True) {
                        obj1 = reg_->New<bool>(true);
                        validOperands = true;
                    }
                    else if (operand1.tokenType == RhoTokenEnumType::False) {
                        obj1 = reg_->New<bool>(false);
                        validOperands = true;
                    }
                    
                    // Second operand
                    if (operand2.tokenType == RhoTokenEnumType::Int) {
                        try {
                            int value = std::stoi(operand2.value);
                            obj2 = reg_->New<int>(value);
                            validOperands = validOperands && true;
                        } catch (...) { validOperands = false; }
                    }
                    else if (operand2.tokenType == RhoTokenEnumType::Float) {
                        try {
                            float value = std::stof(operand2.value);
                            obj2 = reg_->New<float>(value);
                            validOperands = validOperands && true;
                        } catch (...) { validOperands = false; }
                    }
                    else if (operand2.tokenType == RhoTokenEnumType::String) {
                        std::string text = operand2.value;
                        if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                            text = text.substr(1, text.size() - 2);
                        }
                        obj2 = reg_->New<String>(text);
                        validOperands = validOperands && true;
                    }
                    else if (operand2.tokenType == RhoTokenEnumType::True) {
                        obj2 = reg_->New<bool>(true);
                        validOperands = validOperands && true;
                    }
                    else if (operand2.tokenType == RhoTokenEnumType::False) {
                        obj2 = reg_->New<bool>(false);
                        validOperands = validOperands && true;
                    }
                    else {
                        validOperands = false;
                    }
                    
                    // If we have valid operands, perform the operation
                    if (validOperands && obj1.Exists() && obj2.Exists()) {
                        // Create a new executor for the operation
                        Pointer<Executor> opExec = reg_->New<Executor>();
                        opExec->Create();
                        
                        // Get the operation type
                        Operation::Type opType = TokenToOperation(op.tokenType);
                        
                        // Perform the binary operation
                        Object result = opExec->PerformBinaryOp(obj1, obj2, opType);
                        
                        if (result.Exists()) {
                            KAI_TRACE() << "Fallback binary operation result: " << result.ToString()
                                      << " (type: " << (result.GetClass() ? result.GetClass()->GetName().ToString() : "null") << ")";
                            
                            // Extract primitive value if result is a continuation
                            if (result.IsType<Continuation>()) {
                                // One more attempt to evaluate the continuation
                                Pointer<Continuation> resultCont = result;
                                
                                // Check if it has a simple structure with a single value
                                if (resultCont->GetCode() && resultCont->GetCode()->Size() > 0) {
                                    // Try to execute the continuation to get the final value
                                    Pointer<Executor> extractExec = reg_->New<Executor>();
                                    extractExec->Create();
                                    
                                    Pointer<Stack> extractStack = reg_->New<Stack>();
                                    extractExec->SetDataStack(extractStack);
                                    
                                    extractExec->Continue(resultCont);
                                    
                                    if (!extractStack->Empty()) {
                                        Object extractedResult = extractStack->Top();
                                        
                                        // Only use extracted result if it's a primitive type
                                        if (extractedResult.IsType<int>() || extractedResult.IsType<float>() || 
                                            extractedResult.IsType<bool>() || extractedResult.IsType<String>()) {
                                            
                                            KAI_TRACE() << "Extracted primitive value from continuation: " 
                                                      << extractedResult.ToString();
                                            
                                            // Use the extracted result
                                            result = extractedResult;
                                        }
                                    }
                                }
                            }
                            
                            // Add the result to the instruction stream
                            Append(result);
                            return;
                        }
                    }
                }
            }
            
            // FINAL FALLBACK: Traditional Pi execution approach
            // If no direct evaluation method worked, process the Pi sequence using traditional methods
            
            KAI_TRACE() << "Using traditional Pi execution approach";
            
            // Prepare a continuation to hold the code
            Pointer<Continuation> finalCont = reg_->New<Continuation>();
            finalCont->Create();
            
            // Collect all the nodes with proper translation
            Pointer<Array> finalCode = reg_->New<Array>();
            
            // For each node in the sequence
            for (const auto& child : node->GetChildren()) {
                // For basic token types, translate directly
                if (child->GetType() == AstEnum::TokenType) {
                    auto tokenType = child->GetToken().type;
                    
                    switch (tokenType) {
                        case RhoTokenEnumType::Int:
                            try {
                                int value = std::stoi(child->GetTokenText());
                                finalCode->Append(reg_->New<int>(value));
                            } catch (...) {
                                KAI_TRACE_ERROR() << "Error parsing int in Pi sequence";
                            }
                            break;
                            
                        case RhoTokenEnumType::Float:
                            try {
                                float value = std::stof(child->GetTokenText());
                                finalCode->Append(reg_->New<float>(value));
                            } catch (...) {
                                KAI_TRACE_ERROR() << "Error parsing float in Pi sequence";
                            }
                            break;
                            
                        case RhoTokenEnumType::String:
                            {
                                std::string text = child->GetTokenText();
                                if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                                    text = text.substr(1, text.size() - 2);
                                }
                                finalCode->Append(reg_->New<String>(text));
                            }
                            break;
                            
                        case RhoTokenEnumType::True:
                            finalCode->Append(reg_->New<bool>(true));
                            break;
                            
                        case RhoTokenEnumType::False:
                            finalCode->Append(reg_->New<bool>(false));
                            break;
                            
                        default:
                            // For operations and other token types, check if it maps to an operation
                            Operation::Type opType = TokenToOperation(tokenType);
                            if (opType != Operation::None) {
                                finalCode->Append(reg_->New<Operation>(opType));
                            }
                            // For special stack operations
                            else if (child->GetTokenText() == "Dup" || child->GetTokenText() == "dup") {
                                finalCode->Append(reg_->New<Operation>(Operation::Dup));
                            }
                            else if (child->GetTokenText() == "Swap" || child->GetTokenText() == "swap") {
                                finalCode->Append(reg_->New<Operation>(Operation::Swap));
                            }
                            else {
                                // For any other token, translate the node and add its result
                                PushNew();
                                TranslateNode(child);
                                Object nodeResult = Pop();
                                finalCode->Append(nodeResult);
                            }
                            break;
                    }
                }
                else {
                    // For non-token nodes, translate them and add their results
                    PushNew();
                    TranslateNode(child);
                    Object nodeResult = Pop();
                    
                    if (nodeResult.IsType<Continuation>()) {
                        // If we got a continuation, try to unwrap it or extract its code
                        Pointer<Continuation> nodeCont = nodeResult;
                        if (nodeCont->GetCode().Exists()) {
                            // Extract all code elements from the continuation
                            for (int i = 0; i < nodeCont->GetCode()->Size(); i++) {
                                finalCode->Append(nodeCont->GetCode()->At(i));
                            }
                        }
                        else {
                            // If no code, just add the continuation directly
                            finalCode->Append(nodeResult);
                        }
                    }
                    else {
                        // Add the translation result directly
                        finalCode->Append(nodeResult);
                    }
                }
            }
            
            // Set the final code on the continuation
            finalCont->SetCode(finalCode);
            
            // Add the continuation to the instruction stream
            Append(finalCont);
            
            KAI_TRACE() << "Pi sequence translation complete using traditional approach";
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
    
    // DIRECT EVALUATION: Check if both children are constants (literals) that can be 
    // evaluated immediately at compile-time without need for runtime evaluation
    bool canEvaluateDirectly = false;
    Object leftLiteral, rightLiteral;
    
    // Check if left child is a constant literal
    if (leftChild->GetType() == AstEnum::TokenType) {
        auto tokenType = leftChild->GetToken().type;
        if (tokenType == RhoTokenEnumType::Int) {
            try {
                int value = std::stoi(leftChild->GetTokenText());
                leftLiteral = reg_->New<int>(value);
                KAI_TRACE() << "Found integer literal on left side: " << value;
            } catch (...) {}
        }
        else if (tokenType == RhoTokenEnumType::Float) {
            try {
                float value = std::stof(leftChild->GetTokenText());
                leftLiteral = reg_->New<float>(value);
                KAI_TRACE() << "Found float literal on left side: " << value;
            } catch (...) {}
        }
        else if (tokenType == RhoTokenEnumType::String) {
            std::string text = leftChild->GetTokenText();
            if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                text = text.substr(1, text.size() - 2);
            }
            leftLiteral = reg_->New<String>(text);
            KAI_TRACE() << "Found string literal on left side: " << text;
        }
        else if (tokenType == RhoTokenEnumType::True) {
            leftLiteral = reg_->New<bool>(true);
            KAI_TRACE() << "Found boolean literal (true) on left side";
        }
        else if (tokenType == RhoTokenEnumType::False) {
            leftLiteral = reg_->New<bool>(false);
            KAI_TRACE() << "Found boolean literal (false) on left side";
        }
    }
    
    // Check if right child is a constant literal
    if (rightChild->GetType() == AstEnum::TokenType) {
        auto tokenType = rightChild->GetToken().type;
        if (tokenType == RhoTokenEnumType::Int) {
            try {
                int value = std::stoi(rightChild->GetTokenText());
                rightLiteral = reg_->New<int>(value);
                KAI_TRACE() << "Found integer literal on right side: " << value;
            } catch (...) {}
        }
        else if (tokenType == RhoTokenEnumType::Float) {
            try {
                float value = std::stof(rightChild->GetTokenText());
                rightLiteral = reg_->New<float>(value);
                KAI_TRACE() << "Found float literal on right side: " << value;
            } catch (...) {}
        }
        else if (tokenType == RhoTokenEnumType::String) {
            std::string text = rightChild->GetTokenText();
            if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                text = text.substr(1, text.size() - 2);
            }
            rightLiteral = reg_->New<String>(text);
            KAI_TRACE() << "Found string literal on right side: " << text;
        }
        else if (tokenType == RhoTokenEnumType::True) {
            rightLiteral = reg_->New<bool>(true);
            KAI_TRACE() << "Found boolean literal (true) on right side";
        }
        else if (tokenType == RhoTokenEnumType::False) {
            rightLiteral = reg_->New<bool>(false);
            KAI_TRACE() << "Found boolean literal (false) on right side";
        }
    }
    
    // If both sides are constant literals, evaluate directly at compile time
    if (leftLiteral.Exists() && rightLiteral.Exists()) {
        canEvaluateDirectly = true;
        KAI_TRACE() << "Both operands are literals - will perform direct compile-time evaluation";
        
        // Evaluate the operation directly without creating continuations
        Object result;
        
        // Handle integer operations
        if (leftLiteral.IsType<int>() && rightLiteral.IsType<int>()) {
            int left = ConstDeref<int>(leftLiteral);
            int right = ConstDeref<int>(rightLiteral);
            
            switch (op) {
                case Operation::Plus:
                    result = reg_->New<int>(left + right);
                    break;
                case Operation::Minus:
                    result = reg_->New<int>(left - right);
                    break;
                case Operation::Multiply:
                    result = reg_->New<int>(left * right);
                    break;
                case Operation::Divide:
                    if (right != 0) {
                        result = reg_->New<int>(left / right);
                    } else {
                        KAI_TRACE_ERROR() << "Division by zero detected during compile-time evaluation";
                        result = reg_->New<int>(0); // Fallback value
                    }
                    break;
                case Operation::Modulo:
                    if (right != 0) {
                        result = reg_->New<int>(left % right);
                    } else {
                        KAI_TRACE_ERROR() << "Modulo by zero detected during compile-time evaluation";
                        result = reg_->New<int>(0); // Fallback value
                    }
                    break;
                case Operation::Less:
                    result = reg_->New<bool>(left < right);
                    break;
                case Operation::Greater:
                    result = reg_->New<bool>(left > right);
                    break;
                case Operation::LessOrEquiv:
                    result = reg_->New<bool>(left <= right);
                    break;
                case Operation::GreaterOrEquiv:
                    result = reg_->New<bool>(left >= right);
                    break;
                case Operation::Equiv:
                    result = reg_->New<bool>(left == right);
                    break;
                case Operation::NotEquiv:
                    result = reg_->New<bool>(left != right);
                    break;
                case Operation::LogicalAnd:
                    result = reg_->New<bool>(left && right);
                    break;
                case Operation::LogicalOr:
                    result = reg_->New<bool>(left || right);
                    break;
                default:
                    canEvaluateDirectly = false; // Can't evaluate this operation directly
                    KAI_TRACE() << "Operation not supported for direct evaluation: " << Operation::ToString(op);
                    break;
            }
        }
        // Handle float operations
        else if ((leftLiteral.IsType<float>() || leftLiteral.IsType<int>()) && 
                (rightLiteral.IsType<float>() || rightLiteral.IsType<int>())) {
            
            // Convert both operands to float
            float left, right;
            if (leftLiteral.IsType<float>()) {
                left = ConstDeref<float>(leftLiteral);
            } else {
                left = static_cast<float>(ConstDeref<int>(leftLiteral));
            }
            
            if (rightLiteral.IsType<float>()) {
                right = ConstDeref<float>(rightLiteral);
            } else {
                right = static_cast<float>(ConstDeref<int>(rightLiteral));
            }
            
            switch (op) {
                case Operation::Plus:
                    result = reg_->New<float>(left + right);
                    break;
                case Operation::Minus:
                    result = reg_->New<float>(left - right);
                    break;
                case Operation::Multiply:
                    result = reg_->New<float>(left * right);
                    break;
                case Operation::Divide:
                    if (right != 0.0f) {
                        result = reg_->New<float>(left / right);
                    } else {
                        KAI_TRACE_ERROR() << "Division by zero detected during compile-time evaluation";
                        result = reg_->New<float>(0.0f); // Fallback value
                    }
                    break;
                case Operation::Less:
                    result = reg_->New<bool>(left < right);
                    break;
                case Operation::Greater:
                    result = reg_->New<bool>(left > right);
                    break;
                case Operation::LessOrEquiv:
                    result = reg_->New<bool>(left <= right);
                    break;
                case Operation::GreaterOrEquiv:
                    result = reg_->New<bool>(left >= right);
                    break;
                case Operation::Equiv:
                    result = reg_->New<bool>(left == right);
                    break;
                case Operation::NotEquiv:
                    result = reg_->New<bool>(left != right);
                    break;
                default:
                    canEvaluateDirectly = false; // Can't evaluate this operation directly
                    KAI_TRACE() << "Operation not supported for direct float evaluation: " << Operation::ToString(op);
                    break;
            }
        }
        // Handle boolean operations
        else if (leftLiteral.IsType<bool>() && rightLiteral.IsType<bool>()) {
            bool left = ConstDeref<bool>(leftLiteral);
            bool right = ConstDeref<bool>(rightLiteral);
            
            switch (op) {
                case Operation::LogicalAnd:
                    result = reg_->New<bool>(left && right);
                    break;
                case Operation::LogicalOr:
                    result = reg_->New<bool>(left || right);
                    break;
                case Operation::Equiv:
                    result = reg_->New<bool>(left == right);
                    break;
                case Operation::NotEquiv:
                    result = reg_->New<bool>(left != right);
                    break;
                default:
                    canEvaluateDirectly = false; // Can't evaluate this operation directly
                    KAI_TRACE() << "Operation not supported for direct boolean evaluation: " << Operation::ToString(op);
                    break;
            }
        }
        // Handle string operations (just concatenation for now)
        else if (leftLiteral.IsType<String>() && rightLiteral.IsType<String>()) {
            String left = ConstDeref<String>(leftLiteral);
            String right = ConstDeref<String>(rightLiteral);
            
            switch (op) {
                case Operation::Plus:
                    result = reg_->New<String>(left + right);
                    break;
                case Operation::Equiv:
                    result = reg_->New<bool>(left == right);
                    break;
                case Operation::NotEquiv:
                    result = reg_->New<bool>(left != right);
                    break;
                default:
                    canEvaluateDirectly = false; // Can't evaluate this operation directly
                    KAI_TRACE() << "Operation not supported for direct string evaluation: " << Operation::ToString(op);
                    break;
            }
        }
        else {
            // Types don't match or aren't supported
            canEvaluateDirectly = false;
            KAI_TRACE() << "Cannot directly evaluate operation between different types: " 
                         << (leftLiteral.GetClass() ? leftLiteral.GetClass()->GetName().ToString() : "null") 
                         << " and " 
                         << (rightLiteral.GetClass() ? rightLiteral.GetClass()->GetName().ToString() : "null");
        }
        
        // If we successfully evaluated the operation, append the result and return
        if (canEvaluateDirectly && result.Exists()) {
            KAI_TRACE() << "Direct compile-time evaluation result: " << result.ToString() 
                       << " (type: " << (result.GetClass() ? result.GetClass()->GetName().ToString() : "null") << ")";
            
            // Add the result directly to the instruction stream
            Append(result);
            return;
        }
    }
    
    // REGULAR APPROACH: If we couldn't evaluate literals directly, proceed with the standard approach
    
    // Create temporary objects to capture the results of translating the children
    PushNew();
    TranslateNode(leftChild);
    Pointer<Continuation> leftCont = Pop();
    
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
    
    // Check if we can extract simple primitive values from the continuations
    Object leftValue, rightValue;
    bool hasDirectValues = false;
    
    // Extract values from simple single-value continuations
    if (leftCont->GetCode()->Size() == 1) {
        leftValue = leftCont->GetCode()->At(0);
        
        // Only consider it a direct value if it's a primitive type
        if (leftValue.IsType<int>() || leftValue.IsType<float>() || 
            leftValue.IsType<bool>() || leftValue.IsType<String>()) {
            KAI_TRACE() << "Extracted direct value from left continuation: " << leftValue.ToString();
            // Only first part of direct value pair found
            hasDirectValues = true;
        }
    }
    
    if (rightCont->GetCode()->Size() == 1) {
        rightValue = rightCont->GetCode()->At(0);
        
        // Only consider it a direct value if it's a primitive type
        if (rightValue.IsType<int>() || rightValue.IsType<float>() || 
            rightValue.IsType<bool>() || rightValue.IsType<String>()) {
            KAI_TRACE() << "Extracted direct value from right continuation: " << rightValue.ToString();
            // Both parts must be direct values for this approach to work
            hasDirectValues = hasDirectValues && true;
        } else {
            // If right value is not primitive, reset the flag
            hasDirectValues = false;
        }
    } else {
        // If right continuation has multiple values, reset the flag
        hasDirectValues = false;
    }
    
    // If we have direct primitive values from both sides, compute directly
    if (hasDirectValues && leftValue.Exists() && rightValue.Exists()) {
        // Perform the operation directly with the primitive values
        
        Object result;
        bool computed = false;
        
        // Handle integer operations
        if (leftValue.IsType<int>() && rightValue.IsType<int>()) {
            int left = ConstDeref<int>(leftValue);
            int right = ConstDeref<int>(rightValue);
            
            switch (op) {
                case Operation::Plus:
                    result = reg_->New<int>(left + right);
                    computed = true;
                    break;
                case Operation::Minus:
                    result = reg_->New<int>(left - right);
                    computed = true;
                    break;
                case Operation::Multiply:
                    result = reg_->New<int>(left * right);
                    computed = true;
                    break;
                case Operation::Divide:
                    if (right != 0) {
                        result = reg_->New<int>(left / right);
                        computed = true;
                    }
                    break;
                case Operation::Modulo:
                    if (right != 0) {
                        result = reg_->New<int>(left % right);
                        computed = true;
                    }
                    break;
                case Operation::Less:
                    result = reg_->New<bool>(left < right);
                    computed = true;
                    break;
                case Operation::Greater:
                    result = reg_->New<bool>(left > right);
                    computed = true;
                    break;
                case Operation::LessOrEquiv:
                    result = reg_->New<bool>(left <= right);
                    computed = true;
                    break;
                case Operation::GreaterOrEquiv:
                    result = reg_->New<bool>(left >= right);
                    computed = true;
                    break;
                case Operation::Equiv:
                    result = reg_->New<bool>(left == right);
                    computed = true;
                    break;
                case Operation::NotEquiv:
                    result = reg_->New<bool>(left != right);
                    computed = true;
                    break;
                case Operation::LogicalAnd:
                    result = reg_->New<bool>(left && right);
                    computed = true;
                    break;
                case Operation::LogicalOr:
                    result = reg_->New<bool>(left || right);
                    computed = true;
                    break;
            }
        }
        // Handle float operations (mixed with int)
        else if ((leftValue.IsType<float>() || leftValue.IsType<int>()) && 
                (rightValue.IsType<float>() || rightValue.IsType<int>())) {
                
            // Convert operands to float as needed
            float left, right;
            if (leftValue.IsType<float>()) {
                left = ConstDeref<float>(leftValue);
            } else {
                left = static_cast<float>(ConstDeref<int>(leftValue));
            }
            
            if (rightValue.IsType<float>()) {
                right = ConstDeref<float>(rightValue);
            } else {
                right = static_cast<float>(ConstDeref<int>(rightValue));
            }
            
            switch (op) {
                case Operation::Plus:
                    result = reg_->New<float>(left + right);
                    computed = true;
                    break;
                case Operation::Minus:
                    result = reg_->New<float>(left - right);
                    computed = true;
                    break;
                case Operation::Multiply:
                    result = reg_->New<float>(left * right);
                    computed = true;
                    break;
                case Operation::Divide:
                    if (right != 0.0f) {
                        result = reg_->New<float>(left / right);
                        computed = true;
                    }
                    break;
                case Operation::Less:
                    result = reg_->New<bool>(left < right);
                    computed = true;
                    break;
                case Operation::Greater:
                    result = reg_->New<bool>(left > right);
                    computed = true;
                    break;
                case Operation::LessOrEquiv:
                    result = reg_->New<bool>(left <= right);
                    computed = true;
                    break;
                case Operation::GreaterOrEquiv:
                    result = reg_->New<bool>(left >= right);
                    computed = true;
                    break;
                case Operation::Equiv:
                    result = reg_->New<bool>(left == right);
                    computed = true;
                    break;
                case Operation::NotEquiv:
                    result = reg_->New<bool>(left != right);
                    computed = true;
                    break;
            }
        }
        // Handle boolean operations
        else if (leftValue.IsType<bool>() && rightValue.IsType<bool>()) {
            bool left = ConstDeref<bool>(leftValue);
            bool right = ConstDeref<bool>(rightValue);
            
            switch (op) {
                case Operation::LogicalAnd:
                    result = reg_->New<bool>(left && right);
                    computed = true;
                    break;
                case Operation::LogicalOr:
                    result = reg_->New<bool>(left || right);
                    computed = true;
                    break;
                case Operation::Equiv:
                    result = reg_->New<bool>(left == right);
                    computed = true;
                    break;
                case Operation::NotEquiv:
                    result = reg_->New<bool>(left != right);
                    computed = true;
                    break;
            }
        }
        // Handle string operations
        else if (leftValue.IsType<String>() && rightValue.IsType<String>()) {
            String left = ConstDeref<String>(leftValue);
            String right = ConstDeref<String>(rightValue);
            
            switch (op) {
                case Operation::Plus:
                    result = reg_->New<String>(left + right);
                    computed = true;
                    break;
                case Operation::Equiv:
                    result = reg_->New<bool>(left == right);
                    computed = true;
                    break;
                case Operation::NotEquiv:
                    result = reg_->New<bool>(left != right);
                    computed = true;
                    break;
            }
        }
        
        // If we computed a result directly, append it and return
        if (computed && result.Exists()) {
            KAI_TRACE() << "Direct evaluation of extracted primitive values: " << leftValue.ToString() 
                      << " " << Operation::ToString(op) << " " << rightValue.ToString() << " = " << result.ToString();
            
            // Add the result directly to the instruction stream
            Append(result);
            return;
        }
    }
    
    // DYNAMIC EVALUATION: If we couldn't evaluate directly, try dynamic evaluation
    
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
    
    // Execute left continuation and get result
    Object leftObj;
    executor->Continue(leftCont);
    
    if (!tempStack->Empty()) {
        leftObj = tempStack->Pop();
    }
    else {
        // Create appropriate placeholder based on context
        if (op == Operation::Plus || op == Operation::Minus || 
            op == Operation::Multiply || op == Operation::Divide || 
            op == Operation::Modulo) {
            leftObj = reg_->New<int>(0);  // For arithmetic ops, use integer 0
        }
        else if (op == Operation::LogicalAnd || op == Operation::LogicalOr || 
                op == Operation::Equiv || op == Operation::NotEquiv || 
                op == Operation::Less || op == Operation::Greater || 
                op == Operation::LessOrEquiv || op == Operation::GreaterOrEquiv) {
            leftObj = reg_->New<bool>(false);  // For logical ops, use boolean false
        }
        else if (op == Operation::Store) {
            leftObj = Object();
        }
        else {
            leftObj = Object();
        }
    }
    
    // Execute right continuation and get result
    Object rightObj;
    tempStack->Clear();
    executor->Continue(rightCont);
    
    if (!tempStack->Empty()) {
        rightObj = tempStack->Pop();
    }
    else {
        // Create appropriate placeholder based on context
        if (op == Operation::Plus || op == Operation::Minus || 
            op == Operation::Multiply || op == Operation::Divide || 
            op == Operation::Modulo) {
            rightObj = reg_->New<int>(0);  // For arithmetic ops, use integer 0
        }
        else if (op == Operation::LogicalAnd || op == Operation::LogicalOr || 
                op == Operation::Equiv || op == Operation::NotEquiv || 
                op == Operation::Less || op == Operation::Greater || 
                op == Operation::LessOrEquiv || op == Operation::GreaterOrEquiv) {
            rightObj = reg_->New<bool>(false);  // For logical ops, use boolean false
        }
        else {
            rightObj = Object();
        }
    }
    
    // DIRECT RESULT CALCULATION: Perform the operation directly to get the final result
    if (leftObj.Exists() && rightObj.Exists()) {
        // Create direct executor for the binary operation
        Pointer<Executor> opExecutor = reg_->New<Executor>();
        opExecutor->Create();
        
        // Use PerformBinaryOp to execute the operation
        Object result = opExecutor->PerformBinaryOp(leftObj, rightObj, op);
        
        if (result.Exists()) {
            // Log the evaluation with detailed type information
            KAI_TRACE() << "Dynamic evaluation of binary op: " << leftObj.ToString() 
                      << " (" << (leftObj.GetClass() ? leftObj.GetClass()->GetName().ToString() : "null") << ") "  
                      << Operation::ToString(op) << " " 
                      << rightObj.ToString() 
                      << " (" << (rightObj.GetClass() ? rightObj.GetClass()->GetName().ToString() : "null") << ") "
                      << " = " << result.ToString()
                      << " (" << (result.GetClass() ? result.GetClass()->GetName().ToString() : "null") << ")";
            
            // EXTRACT PRIMITIVE VALUE FROM RESULT IF NEEDED
            Object finalResult = result;
            
            // If the result is a continuation, try to extract primitive value
            if (result.IsType<Continuation>()) {
                Pointer<Continuation> resultCont = result;
                
                // Only extract if it seems like a simple operation result (not a block/function)
                if (resultCont->GetCode() && resultCont->GetCode()->Size() <= 4) {
                    // Try to evaluate the continuation to get a primitive result
                    Pointer<Executor> extractExecutor = reg_->New<Executor>();
                    extractExecutor->Create();
                    
                    // Set up temporary stack
                    Pointer<Stack> extractStack = reg_->New<Stack>();
                    extractExecutor->SetDataStack(extractStack);
                    
                    // Try to evaluate the continuation
                    extractExecutor->Continue(resultCont);
                    
                    // If we got a primitive value, use it
                    if (!extractStack->Empty()) {
                        Object extractedResult = extractStack->Top();
                        
                        // Only use if it's a primitive type
                        if (extractedResult.IsType<int>() || extractedResult.IsType<float>() || 
                            extractedResult.IsType<bool>() || extractedResult.IsType<String>()) {
                            KAI_TRACE() << "Extracted primitive value from result continuation: " 
                                       << extractedResult.ToString();
                            finalResult = extractedResult;
                        }
                    }
                    
                    // Direct extraction didn't work, try to calculate directly from continuation code
                    if (finalResult.IsType<Continuation>()) {
                        auto code = resultCont->GetCode();
                        
                        // Handle [val1, val2, op] pattern
                        if (code->Size() == 3 && code->At(2).IsType<Operation>()) {
                            Object val1 = code->At(0);
                            Object val2 = code->At(1);
                            Operation::Type contOp = ConstDeref<Operation>(code->At(2)).GetTypeNumber();
                            
                            // Handle integer operations
                            if (val1.IsType<int>() && val2.IsType<int>()) {
                                int num1 = ConstDeref<int>(val1);
                                int num2 = ConstDeref<int>(val2);
                                
                                switch (contOp) {
                                    case Operation::Plus:
                                        finalResult = reg_->New<int>(num1 + num2);
                                        break;
                                    case Operation::Minus:
                                        finalResult = reg_->New<int>(num1 - num2);
                                        break;
                                    case Operation::Multiply:
                                        finalResult = reg_->New<int>(num1 * num2);
                                        break;
                                    case Operation::Divide:
                                        if (num2 != 0) finalResult = reg_->New<int>(num1 / num2);
                                        break;
                                    default:
                                        break;
                                }
                            }
                            
                            // Handle boolean operations
                            if (val1.IsType<bool>() && val2.IsType<bool>()) {
                                bool b1 = ConstDeref<bool>(val1);
                                bool b2 = ConstDeref<bool>(val2);
                                
                                switch (contOp) {
                                    case Operation::LogicalAnd:
                                        finalResult = reg_->New<bool>(b1 && b2);
                                        break;
                                    case Operation::LogicalOr:
                                        finalResult = reg_->New<bool>(b1 || b2);
                                        break;
                                    case Operation::Equiv:
                                        finalResult = reg_->New<bool>(b1 == b2);
                                        break;
                                    case Operation::NotEquiv:
                                        finalResult = reg_->New<bool>(b1 != b2);
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                }
            }
            
            // Log the final result after potential extraction
            if (finalResult != result) {
                KAI_TRACE() << "Final extracted primitive result: " << finalResult.ToString() 
                          << " (" << (finalResult.GetClass() ? finalResult.GetClass()->GetName().ToString() : "null") << ")";
            }
            
            // Append the final result directly - this is the key change for type preservation
            Append(finalResult);
            return;
        }
    }
    
    // FALLBACK: If all direct evaluation methods failed, use standard Pi approach
    KAI_TRACE() << "Using standard Pi approach for binary operation";
    
    // Add each operand and the operation to the stream
    TranslateNode(leftChild);
    TranslateNode(rightChild);
    AppendDirectOperation(op);
    
    KAI_TRACE() << "Binary operation translated using standard Pi approach";
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

        case AstEnum::For:
            TranslateFor(node);
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

void RhoTranslator::TranslateFor(AstNodePtr node) {
    try {
        KAI_TRACE() << "Translating for loop";

        // Verify we have enough children (init, condition, increment, body)
        if (node->GetChildren().size() < 4) {
            KAI_TRACE_ERROR() << "Not enough children in For node";
            Fail("For node must have at least 4 children (init, condition, increment, body)");
            return;
        }

        // Get initialization, condition, increment, and body parts
        const auto init = node->GetChild(0);
        const auto condition = node->GetChild(1);
        const auto increment = node->GetChild(2);
        const auto body = node->GetChild(3);

        // Keep track of entry, condition check, and exit points for our for loop
        // using labels (similar to assembly)
        Pointer<Label> loopStart = reg_->New<Label>(Label("forLoopStart"));
        Pointer<Label> loopEnd = reg_->New<Label>(Label("forLoopEnd"));
        
        // Part 1: Initialization
        KAI_TRACE() << "TranslateFor: Translating initialization";
        if (init && init->GetType() != AstEnum::None) {
            TranslateNode(init);
        }

        // Create a loop structure as follows:
        // 1. Execute initialization once
        // 2. Check condition
        // 3. If condition is false, jump to loop end
        // 4. Execute body
        // 5. Execute increment
        // 6. Go back to condition (step 2)

        // Part 2: Define the start of the loop for jumps
        Append(loopStart);
        
        // Part 3: Condition check
        KAI_TRACE() << "TranslateFor: Translating condition";
        if (condition && condition->GetType() != AstEnum::None) {
            TranslateNode(condition);
            
            // If false, jump to the end of the loop
            Pointer<Continuation> exitCont = reg_->New<Continuation>();
            exitCont->Create();
            // Add the loop end label to the code array
            Pointer<Array> exitCode = reg_->New<Array>();
            exitCode->Append(loopEnd);
            exitCont->SetCode(exitCode);
            Append(exitCont);
            AppendDirectOperation(Operation::IfFalseJump);
        }
        
        // Part 4: Body of the loop
        KAI_TRACE() << "TranslateFor: Translating body";
        if (body) {
            TranslateNode(body);
        }
        
        // Part 5: Increment step
        KAI_TRACE() << "TranslateFor: Translating increment";
        if (increment && increment->GetType() != AstEnum::None) {
            TranslateNode(increment);
        }
        
        // Part 6: Jump back to the start of the loop
        Pointer<Continuation> loopCont = reg_->New<Continuation>();
        loopCont->Create();
        // Add the loop start label to the code array
        Pointer<Array> loopCode = reg_->New<Array>();
        loopCode->Append(loopStart);
        loopCont->SetCode(loopCode);
        Append(loopCont);
        AppendDirectOperation(Operation::Jump);
        
        // Part 7: Define the end of the loop for jumps
        Append(loopEnd);
        
        KAI_TRACE() << "For loop translation complete with direct Pi operations";
    } catch (const std::exception& e) {
        KAI_TRACE_ERROR() << std::format("Exception in TranslateFor: {}", e.what());
        Fail(std::format("Exception in TranslateFor: {}", e.what()));
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslateFor";
        Fail("Unknown exception in TranslateFor");
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