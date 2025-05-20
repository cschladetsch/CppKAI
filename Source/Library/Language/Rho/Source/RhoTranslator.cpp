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

            // ENHANCED DIRECT Pi EVALUATION IMPLEMENTATION:
            // This completely rewritten implementation focuses on:
            // 1. Properly handling primitive types (int, float, bool, String)
            // 2. Ensuring type preservation through direct evaluation when
            // possible
            // 3. Using a multi-layered approach for fall-back evaluation
            // methods

            // LAYER 1: Try to directly evaluate simple Pi expressions with
            // literals Analyze the Pi sequence structure
            struct NodeInfo {
                AstNodePtr node;
                RhoTokenEnumType::Enum tokenType;
                std::string value;
                bool isOperation;

                NodeInfo(AstNodePtr n) : node(n), isOperation(false) {
                    if (n->GetType() == AstEnum::TokenType) {
                        tokenType = static_cast<RhoTokenEnumType::Enum>(
                            n->GetToken().type);
                        value = n->GetTokenText();

                        // Check if this is an operation token
                        isOperation =
                            (tokenType == RhoTokenEnumType::Plus ||
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
                                KAI_TRACE()
                                    << "DIRECT INTEGER EVAL: " << firstValue
                                    << " + " << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Minus: {
                                int result = firstValue - secondValue;
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE()
                                    << "DIRECT INTEGER EVAL: " << firstValue
                                    << " - " << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Mul: {
                                int result = firstValue * secondValue;
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE()
                                    << "DIRECT INTEGER EVAL: " << firstValue
                                    << " * " << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Divide: {
                                if (secondValue != 0) {
                                    int result = firstValue / secondValue;
                                    Object resultObj = reg_->New<int>(result);
                                    KAI_TRACE()
                                        << "DIRECT INTEGER EVAL: " << firstValue
                                        << " / " << secondValue << " = "
                                        << result;
                                    Append(resultObj);
                                    return;
                                }
                                break;
                            }
                            case RhoTokenEnumType::Mod: {
                                if (secondValue != 0) {
                                    int result = firstValue % secondValue;
                                    Object resultObj = reg_->New<int>(result);
                                    KAI_TRACE()
                                        << "DIRECT INTEGER EVAL: " << firstValue
                                        << " % " << secondValue << " = "
                                        << result;
                                    Append(resultObj);
                                    return;
                                }
                                break;
                            }
                            case RhoTokenEnumType::Less: {
                                bool result = firstValue < secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT INTEGER COMPARISON: "
                                    << firstValue << " < " << secondValue
                                    << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Greater: {
                                bool result = firstValue > secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT INTEGER COMPARISON: "
                                    << firstValue << " > " << secondValue
                                    << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::LessEquiv: {
                                bool result = firstValue <= secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT INTEGER COMPARISON: "
                                    << firstValue << " <= " << secondValue
                                    << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::GreaterEquiv: {
                                bool result = firstValue >= secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT INTEGER COMPARISON: "
                                    << firstValue << " >= " << secondValue
                                    << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Equiv: {
                                bool result = firstValue == secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT INTEGER COMPARISON: "
                                    << firstValue << " == " << secondValue
                                    << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::NotEquiv: {
                                bool result = firstValue != secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT INTEGER COMPARISON: "
                                    << firstValue << " != " << secondValue
                                    << " = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                        }
                    } catch (...) {
                        KAI_TRACE_ERROR()
                            << "Error in direct integer Pi evaluation";
                    }
                }

                // Float operations (including mixed int/float)
                else if ((first.tokenType == RhoTokenEnumType::Float ||
                          first.tokenType == RhoTokenEnumType::Int) &&
                         (second.tokenType == RhoTokenEnumType::Float ||
                          second.tokenType == RhoTokenEnumType::Int)) {
                    try {
                        // Parse float values, with conversion from int if
                        // needed
                        float firstValue, secondValue;

                        if (first.tokenType == RhoTokenEnumType::Float) {
                            firstValue = std::stof(first.value);
                        } else {
                            firstValue =
                                static_cast<float>(std::stoi(first.value));
                        }

                        if (second.tokenType == RhoTokenEnumType::Float) {
                            secondValue = std::stof(second.value);
                        } else {
                            secondValue =
                                static_cast<float>(std::stoi(second.value));
                        }

                        // Select the appropriate operation
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Plus: {
                                float result = firstValue + secondValue;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE()
                                    << "DIRECT FLOAT EVAL: " << firstValue
                                    << " + " << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Minus: {
                                float result = firstValue - secondValue;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE()
                                    << "DIRECT FLOAT EVAL: " << firstValue
                                    << " - " << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Mul: {
                                float result = firstValue * secondValue;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE()
                                    << "DIRECT FLOAT EVAL: " << firstValue
                                    << " * " << secondValue << " = " << result;
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Divide: {
                                if (secondValue != 0.0f) {
                                    float result = firstValue / secondValue;
                                    Object resultObj = reg_->New<float>(result);
                                    KAI_TRACE()
                                        << "DIRECT FLOAT EVAL: " << firstValue
                                        << " / " << secondValue << " = "
                                        << result;
                                    Append(resultObj);
                                    return;
                                }
                                break;
                            }
                            case RhoTokenEnumType::Less: {
                                bool result = firstValue < secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT FLOAT COMPARISON: " << firstValue
                                    << " < " << secondValue << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Greater: {
                                bool result = firstValue > secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT FLOAT COMPARISON: " << firstValue
                                    << " > " << secondValue << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::LessEquiv: {
                                bool result = firstValue <= secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT FLOAT COMPARISON: " << firstValue
                                    << " <= " << secondValue << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::GreaterEquiv: {
                                bool result = firstValue >= secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT FLOAT COMPARISON: " << firstValue
                                    << " >= " << secondValue << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Equiv: {
                                bool result = firstValue == secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT FLOAT COMPARISON: " << firstValue
                                    << " == " << secondValue << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::NotEquiv: {
                                bool result = firstValue != secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT FLOAT COMPARISON: " << firstValue
                                    << " != " << secondValue << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                        }
                    } catch (...) {
                        KAI_TRACE_ERROR()
                            << "Error in direct float Pi evaluation";
                    }
                }

                // Boolean operations
                else if ((first.tokenType == RhoTokenEnumType::True ||
                          first.tokenType == RhoTokenEnumType::False) &&
                         (second.tokenType == RhoTokenEnumType::True ||
                          second.tokenType == RhoTokenEnumType::False)) {
                    try {
                        // Get boolean values directly from token types
                        bool firstValue =
                            (first.tokenType == RhoTokenEnumType::True);
                        bool secondValue =
                            (second.tokenType == RhoTokenEnumType::True);

                        // Select the appropriate operation
                        switch (op.tokenType) {
                            case RhoTokenEnumType::And: {
                                bool result = firstValue && secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT BOOLEAN EVAL: "
                                    << (firstValue ? "true" : "false")
                                    << " AND "
                                    << (secondValue ? "true" : "false") << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Or: {
                                bool result = firstValue || secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT BOOLEAN EVAL: "
                                    << (firstValue ? "true" : "false") << " OR "
                                    << (secondValue ? "true" : "false") << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Equiv: {
                                bool result = firstValue == secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT BOOLEAN EVAL: "
                                    << (firstValue ? "true" : "false") << " == "
                                    << (secondValue ? "true" : "false") << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::NotEquiv: {
                                bool result = firstValue != secondValue;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT BOOLEAN EVAL: "
                                    << (firstValue ? "true" : "false") << " != "
                                    << (secondValue ? "true" : "false") << " = "
                                    << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                        }
                    } catch (...) {
                        KAI_TRACE_ERROR()
                            << "Error in direct boolean Pi evaluation";
                    }
                }

                // String operations (concatenation, comparison)
                else if (first.tokenType == RhoTokenEnumType::String &&
                         second.tokenType == RhoTokenEnumType::String) {
                    try {
                        // Process the string values (remove quotes)
                        std::string firstText = first.value;
                        std::string secondText = second.value;

                        if (firstText.size() >= 2 && firstText.front() == '"' &&
                            firstText.back() == '"') {
                            firstText =
                                firstText.substr(1, firstText.size() - 2);
                        }

                        if (secondText.size() >= 2 &&
                            secondText.front() == '"' &&
                            secondText.back() == '"') {
                            secondText =
                                secondText.substr(1, secondText.size() - 2);
                        }

                        // Select the appropriate operation
                        switch (op.tokenType) {
                            case RhoTokenEnumType::Plus: {
                                // String concatenation
                                String result = firstText + secondText;
                                Object resultObj = reg_->New<String>(result);
                                KAI_TRACE()
                                    << "DIRECT STRING EVAL: \"" << firstText
                                    << "\" + \"" << secondText << "\" = \""
                                    << result << "\"";
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::Equiv: {
                                // String equality
                                bool result = firstText == secondText;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT STRING COMPARISON: \""
                                    << firstText << "\" == \"" << secondText
                                    << "\" = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                            case RhoTokenEnumType::NotEquiv: {
                                // String inequality
                                bool result = firstText != secondText;
                                Object resultObj = reg_->New<bool>(result);
                                KAI_TRACE()
                                    << "DIRECT STRING COMPARISON: \""
                                    << firstText << "\" != \"" << secondText
                                    << "\" = " << (result ? "true" : "false");
                                Append(resultObj);
                                return;
                            }
                        }
                    } catch (...) {
                        KAI_TRACE_ERROR()
                            << "Error in direct string Pi evaluation";
                    }
                }
            }

            // Common pattern 2: Stack operations (dup, swap)
            // Example: 5 dup + (duplicates 5 and adds, resulting in 10)
            if (sequence.size() >= 2) {
                // Special case for 5 dup +
                if ((sequence.size() == 3 &&
                     (sequence[0].tokenType == RhoTokenEnumType::Int ||
                      sequence[0].tokenType == RhoTokenEnumType::Float) &&
                     sequence[1].value == "Dup") ||
                    (sequence[1].value == "dup" && sequence[2].isOperation)) {
                    try {
                        // Handle integer dup operations
                        if (sequence[0].tokenType == RhoTokenEnumType::Int) {
                            int value = std::stoi(sequence[0].value);

                            // Handle common operations after dup
                            if (sequence[2].tokenType ==
                                RhoTokenEnumType::Plus) {
                                // 5 dup + = 5 + 5 = 10
                                int result = value + value;
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE() << "DIRECT DUP+ADD: " << value
                                            << " dup + = " << result;
                                Append(resultObj);
                                return;
                            } else if (sequence[2].tokenType ==
                                       RhoTokenEnumType::Mul) {
                                // 5 dup * = 5 * 5 = 25
                                int result = value * value;
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE() << "DIRECT DUP+MULTIPLY: " << value
                                            << " dup * = " << result;
                                Append(resultObj);
                                return;
                            }
                        }
                        // Handle float dup operations
                        else if (sequence[0].tokenType ==
                                 RhoTokenEnumType::Float) {
                            float value = std::stof(sequence[0].value);

                            // Handle common operations after dup
                            if (sequence[2].tokenType ==
                                RhoTokenEnumType::Plus) {
                                // 5.5 dup + = 5.5 + 5.5 = 11.0
                                float result = value + value;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE() << "DIRECT DUP+ADD: " << value
                                            << " dup + = " << result;
                                Append(resultObj);
                                return;
                            } else if (sequence[2].tokenType ==
                                       RhoTokenEnumType::Mul) {
                                // 5.5 dup * = 5.5 * 5.5 = 30.25
                                float result = value * value;
                                Object resultObj = reg_->New<float>(result);
                                KAI_TRACE() << "DIRECT DUP+MULTIPLY: " << value
                                            << " dup * = " << result;
                                Append(resultObj);
                                return;
                            }
                        }
                    } catch (...) {
                        KAI_TRACE_ERROR()
                            << "Error in direct dup operation evaluation";
                    }
                }

                // Special case for swap operations: 3 4 swap -
                if (sequence.size() == 4 &&
                    (sequence[0].tokenType == RhoTokenEnumType::Int ||
                     sequence[0].tokenType == RhoTokenEnumType::Float) &&
                    (sequence[1].tokenType == RhoTokenEnumType::Int ||
                     sequence[1].tokenType == RhoTokenEnumType::Float) &&
                    (sequence[2].value == "Swap" ||
                     sequence[2].value == "swap") &&
                    sequence[3].isOperation) {
                    try {
                        // For simplicity, let's handle integer swap operations
                        // first
                        if (sequence[0].tokenType == RhoTokenEnumType::Int &&
                            sequence[1].tokenType == RhoTokenEnumType::Int) {
                            int first = std::stoi(sequence[0].value);
                            int second = std::stoi(sequence[1].value);

                            // 3 4 swap - = 4 - 3 = 1 (swap puts 3 on top, then
                            // subtract from 4)
                            if (sequence[3].tokenType ==
                                RhoTokenEnumType::Minus) {
                                int result =
                                    second - first;  // After swap, second is on
                                                     // bottom, first on top
                                Object resultObj = reg_->New<int>(result);
                                KAI_TRACE()
                                    << "DIRECT SWAP+SUBTRACT: " << first << " "
                                    << second << " swap - = " << result;
                                Append(resultObj);
                                return;
                            }
                            // Other operations could be added here as needed
                        }
                    } catch (...) {
                        KAI_TRACE_ERROR()
                            << "Error in direct swap operation evaluation";
                    }
                }
            }

            // LAYER 2: Direct evaluation by building and executing a proper Pi
            // program

            // Create a properly constructed array of objects representing the
            // Pi program First, collect and transform all elements into proper
            // Pi objects
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
                    } catch (...) {
                        allElementsTranslated = false;
                        break;
                    }
                } else if (item.tokenType == RhoTokenEnumType::Float) {
                    try {
                        float value = std::stof(item.value);
                        piCode->Append(reg_->New<float>(value));
                    } catch (...) {
                        allElementsTranslated = false;
                        break;
                    }
                } else if (item.tokenType == RhoTokenEnumType::String) {
                    std::string text = item.value;
                    if (text.size() >= 2 && text.front() == '"' &&
                        text.back() == '"') {
                        text = text.substr(1, text.size() - 2);
                    }
                    piCode->Append(reg_->New<String>(text));
                } else if (item.tokenType == RhoTokenEnumType::True) {
                    piCode->Append(reg_->New<bool>(true));
                } else if (item.tokenType == RhoTokenEnumType::False) {
                    piCode->Append(reg_->New<bool>(false));
                }
                // Operations get converted to Operation objects
                else if (item.isOperation) {
                    Operation::Type opType = TokenToOperation(item.tokenType);
                    if (opType != Operation::None) {
                        piCode->Append(reg_->New<Operation>(opType));
                    } else {
                        allElementsTranslated = false;
                        break;
                    }
                }
                // Special stack operations
                else if (item.value == "Dup" || item.value == "dup") {
                    piCode->Append(reg_->New<Operation>(Operation::Dup));
                } else if (item.value == "Swap" || item.value == "swap") {
                    piCode->Append(reg_->New<Operation>(Operation::Swap));
                } else {
                    // Unsupported element type
                    allElementsTranslated = false;
                    break;
                }
            }

            // Add proper ContinuationEnd marker
            piCode->Append(reg_->New<Operation>(Operation::ContinuationEnd));

            // If all elements were successfully translated, execute the Pi
            // program
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
                        KAI_TRACE()
                            << "Got continuation result from Pi execution, "
                               "attempting to extract value...";

                        // Try to extract a primitive value
                        Pointer<Continuation> resultCont = result;

                        // Check if it has a simple structure with a single
                        // value
                        if (resultCont->GetCode().Exists() &&
                            resultCont->GetCode()->Size() > 0) {
                            // Try to execute the continuation to get the final
                            // value
                            Pointer<Executor> extractExec =
                                reg_->New<Executor>();
                            extractExec->Create();

                            Pointer<Stack> extractStack = reg_->New<Stack>();
                            extractExec->SetDataStack(extractStack);

                            extractExec->Continue(resultCont);

                            if (!extractStack->Empty()) {
                                Object extractedResult = extractStack->Top();

                                // Only use extracted result if it's a primitive
                                // type
                                if (extractedResult.IsType<int>() ||
                                    extractedResult.IsType<float>() ||
                                    extractedResult.IsType<bool>() ||
                                    extractedResult.IsType<String>()) {
                                    KAI_TRACE()
                                        << "Extracted primitive value from Pi "
                                           "result: "
                                        << extractedResult.ToString()
                                        << " (type: "
                                        << extractedResult.GetClass()->GetName()
                                        << ")";

                                    // This is the primary result we want to use
                                    result = extractedResult;
                                }
                            }
                        }
                    }

                    // Log the final result
                    KAI_TRACE()
                        << "Pi sequence final result: " << result.ToString()
                        << " (type: "
                        << (result.GetClass()
                                ? result.GetClass()->GetName().ToString()
                                : "null")
                        << ")";

                    // Add the result to the instruction stream
                    Append(result);
                    return;
                } else {
                    KAI_TRACE_ERROR()
                        << "Pi execution completed but produced no result";
                }
            }

            // LAYER 3: Fallback to explicit evaluation for specific operation
            // sequences This creates Pi code for binary operations, executes
            // it, and returns the result

            // For binary operations with [operand1, operand2, operation]
            if (sequence.size() >= 3) {
                // Get the elements (assuming postfix notation)
                auto op =
                    sequence.back();  // Last element should be the operation

                // Only proceed if the last element is an operation
                if (op.isOperation && sequence.size() >= 3) {
                    // Get operands (stack has operands in reverse order for
                    // postfix notation)
                    auto operand2 = sequence[sequence.size() - 2];
                    auto operand1 = sequence[sequence.size() - 3];

                    // Create objects for the operands
                    Object obj1, obj2;
                    bool validOperands = false;

                    // Try to create objects for the operands based on their
                    // types
                    if (operand1.tokenType == RhoTokenEnumType::Int) {
                        try {
                            int value = std::stoi(operand1.value);
                            obj1 = reg_->New<int>(value);
                            validOperands = true;
                        } catch (...) {
                        }
                    } else if (operand1.tokenType == RhoTokenEnumType::Float) {
                        try {
                            float value = std::stof(operand1.value);
                            obj1 = reg_->New<float>(value);
                            validOperands = true;
                        } catch (...) {
                        }
                    } else if (operand1.tokenType == RhoTokenEnumType::String) {
                        std::string text = operand1.value;
                        if (text.size() >= 2 && text.front() == '"' &&
                            text.back() == '"') {
                            text = text.substr(1, text.size() - 2);
                        }
                        obj1 = reg_->New<String>(text);
                        validOperands = true;
                    } else if (operand1.tokenType == RhoTokenEnumType::True) {
                        obj1 = reg_->New<bool>(true);
                        validOperands = true;
                    } else if (operand1.tokenType == RhoTokenEnumType::False) {
                        obj1 = reg_->New<bool>(false);
                        validOperands = true;
                    }

                    // Second operand
                    if (operand2.tokenType == RhoTokenEnumType::Int) {
                        try {
                            int value = std::stoi(operand2.value);
                            obj2 = reg_->New<int>(value);
                            validOperands = validOperands && true;
                        } catch (...) {
                            validOperands = false;
                        }
                    } else if (operand2.tokenType == RhoTokenEnumType::Float) {
                        try {
                            float value = std::stof(operand2.value);
                            obj2 = reg_->New<float>(value);
                            validOperands = validOperands && true;
                        } catch (...) {
                            validOperands = false;
                        }
                    } else if (operand2.tokenType == RhoTokenEnumType::String) {
                        std::string text = operand2.value;
                        if (text.size() >= 2 && text.front() == '"' &&
                            text.back() == '"') {
                            text = text.substr(1, text.size() - 2);
                        }
                        obj2 = reg_->New<String>(text);
                        validOperands = validOperands && true;
                    } else if (operand2.tokenType == RhoTokenEnumType::True) {
                        obj2 = reg_->New<bool>(true);
                        validOperands = validOperands && true;
                    } else if (operand2.tokenType == RhoTokenEnumType::False) {
                        obj2 = reg_->New<bool>(false);
                        validOperands = validOperands && true;
                    } else {
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
                        Object result =
                            opExec->PerformBinaryOp(obj1, obj2, opType);

                        if (result.Exists()) {
                            KAI_TRACE()
                                << "Fallback binary operation result: "
                                << result.ToString() << " (type: "
                                << (result.GetClass() ? result.GetClass()
                                                            ->GetName()
                                                            .ToString()
                                                      : "null")
                                << ")";

                            // Extract primitive value if result is a
                            // continuation
                            if (result.IsType<Continuation>()) {
                                // One more attempt to evaluate the continuation
                                Pointer<Continuation> resultCont = result;

                                // Check if it has a simple structure with a
                                // single value
                                if (resultCont->GetCode().Exists() &&
                                    resultCont->GetCode()->Size() > 0) {
                                    // Try to execute the continuation to get
                                    // the final value
                                    Pointer<Executor> extractExec =
                                        reg_->New<Executor>();
                                    extractExec->Create();

                                    Pointer<Stack> extractStack =
                                        reg_->New<Stack>();
                                    extractExec->SetDataStack(extractStack);

                                    extractExec->Continue(resultCont);

                                    if (!extractStack->Empty()) {
                                        Object extractedResult =
                                            extractStack->Top();

                                        // Only use extracted result if it's a
                                        // primitive type
                                        if (extractedResult.IsType<int>() ||
                                            extractedResult.IsType<float>() ||
                                            extractedResult.IsType<bool>() ||
                                            extractedResult.IsType<String>()) {
                                            KAI_TRACE()
                                                << "Extracted primitive value "
                                                   "from continuation: "
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
            // If no direct evaluation method worked, process the Pi sequence
            // using traditional methods

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
                                KAI_TRACE_ERROR()
                                    << "Error parsing int in Pi sequence";
                            }
                            break;

                        case RhoTokenEnumType::Float:
                            try {
                                float value = std::stof(child->GetTokenText());
                                finalCode->Append(reg_->New<float>(value));
                            } catch (...) {
                                KAI_TRACE_ERROR()
                                    << "Error parsing float in Pi sequence";
                            }
                            break;

                        case RhoTokenEnumType::String: {
                            std::string text = child->GetTokenText();
                            if (text.size() >= 2 && text.front() == '"' &&
                                text.back() == '"') {
                                text = text.substr(1, text.size() - 2);
                            }
                            finalCode->Append(reg_->New<String>(text));
                        } break;

                        case RhoTokenEnumType::True:
                            finalCode->Append(reg_->New<bool>(true));
                            break;

                        case RhoTokenEnumType::False:
                            finalCode->Append(reg_->New<bool>(false));
                            break;

                        default:
                            // For operations and other token types, check if it
                            // maps to an operation
                            Operation::Type opType =
                                TokenToOperation(tokenType);
                            if (opType != Operation::None) {
                                finalCode->Append(reg_->New<Operation>(opType));
                            }
                            // For special stack operations
                            else if (child->GetTokenText() == "Dup" ||
                                     child->GetTokenText() == "dup") {
                                finalCode->Append(
                                    reg_->New<Operation>(Operation::Dup));
                            } else if (child->GetTokenText() == "Swap" ||
                                       child->GetTokenText() == "swap") {
                                finalCode->Append(
                                    reg_->New<Operation>(Operation::Swap));
                            } else {
                                // For any other token, translate the node and
                                // add its result
                                PushNew();
                                TranslateNode(child);
                                Object nodeResult = Pop();
                                finalCode->Append(nodeResult);
                            }
                            break;
                    }
                } else {
                    // For non-token nodes, translate them and add their results
                    PushNew();
                    TranslateNode(child);
                    Object nodeResult = Pop();

                    if (nodeResult.IsType<Continuation>()) {
                        // If we got a continuation, try to unwrap it or extract
                        // its code
                        Pointer<Continuation> nodeCont = nodeResult;
                        if (nodeCont->GetCode().Exists()) {
                            // Extract all code elements from the continuation
                            for (int i = 0; i < nodeCont->GetCode()->Size();
                                 i++) {
                                finalCode->Append(nodeCont->GetCode()->At(i));
                            }
                        } else {
                            // If no code, just add the continuation directly
                            finalCode->Append(nodeResult);
                        }
                    } else {
                        // Add the translation result directly
                        finalCode->Append(nodeResult);
                    }
                }
            }

            // Set the final code on the continuation
            finalCont->SetCode(finalCode);

            // Add the continuation to the instruction stream
            Append(finalCont);

            KAI_TRACE() << "Pi sequence translation complete using traditional "
                           "approach";
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

    // For binary operations, we want a linear stream of operations
    // rather than creating nested continuations

    // Special case for simple literal operations that can be directly evaluated
    bool directEvaluation = false;
    if (node->GetChild(0)->GetType() == AstEnum::TokenType &&
        node->GetChild(1)->GetType() == AstEnum::TokenType) {
        auto token0 = node->GetChild(0)->GetToken().type;
        auto token1 = node->GetChild(1)->GetToken().type;

        // If both are literals (int, float, bool, string)
        if ((token0 == RhoTokenEnumType::Int ||
             token0 == RhoTokenEnumType::Float ||
             token0 == RhoTokenEnumType::True ||
             token0 == RhoTokenEnumType::False ||
             token0 == RhoTokenEnumType::String) &&
            (token1 == RhoTokenEnumType::Int ||
             token1 == RhoTokenEnumType::Float ||
             token1 == RhoTokenEnumType::True ||
             token1 == RhoTokenEnumType::False ||
             token1 == RhoTokenEnumType::String)) {
            // For known operations that can be directly calculated
            if (op == Operation::Plus || op == Operation::Minus ||
                op == Operation::Multiply || op == Operation::Divide ||
                op == Operation::Modulo || op == Operation::Equiv ||
                op == Operation::NotEquiv || op == Operation::Less ||
                op == Operation::Greater || op == Operation::LessOrEquiv ||
                op == Operation::GreaterOrEquiv) {
                // We'll handle this with direct evaluation
                directEvaluation = true;
            }
        }
    }

    if (directEvaluation) {
        // Directly create and add operand objects based on their token types
        Object left, right;
        auto leftNode = node->GetChild(0);
        auto rightNode = node->GetChild(1);

        // Create left operand
        auto leftType = leftNode->GetToken().type;
        if (leftType == RhoTokenEnumType::Int) {
            left = reg_->New<int>(std::stoi(leftNode->GetTokenText()));
        } else if (leftType == RhoTokenEnumType::Float) {
            left = reg_->New<float>(std::stof(leftNode->GetTokenText()));
        } else if (leftType == RhoTokenEnumType::True) {
            left = reg_->New<bool>(true);
        } else if (leftType == RhoTokenEnumType::False) {
            left = reg_->New<bool>(false);
        } else if (leftType == RhoTokenEnumType::String) {
            std::string text = leftNode->GetTokenText();
            if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                text = text.substr(1, text.size() - 2);
            }
            left = reg_->New<String>(text);
        }

        // Create right operand
        auto rightType = rightNode->GetToken().type;
        if (rightType == RhoTokenEnumType::Int) {
            right = reg_->New<int>(std::stoi(rightNode->GetTokenText()));
        } else if (rightType == RhoTokenEnumType::Float) {
            right = reg_->New<float>(std::stof(rightNode->GetTokenText()));
        } else if (rightType == RhoTokenEnumType::True) {
            right = reg_->New<bool>(true);
        } else if (rightType == RhoTokenEnumType::False) {
            right = reg_->New<bool>(false);
        } else if (rightType == RhoTokenEnumType::String) {
            std::string text = rightNode->GetTokenText();
            if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
                text = text.substr(1, text.size() - 2);
            }
            right = reg_->New<String>(text);
        }

        // Create executor to perform direct operation
        Pointer<Executor> executor = reg_->New<Executor>();
        executor->Create();

        // Perform operation directly
        Object result = executor->PerformBinaryOp(left, right, op);

        // Only use the direct result if it's a primitive type
        if (result.IsType<int>() || result.IsType<float>() ||
            result.IsType<bool>() || result.IsType<String>()) {
            // Append the result directly - this keeps the primitive type
            Append(result);

            KAI_TRACE() << "Direct evaluation result: " << result.ToString();
            return;
        }
    }

    // For complex expressions where direct evaluation isn't possible,
    // translate operands and add operation in a linear fashion

    // Translate the left operand
    TranslateNode(node->GetChild(0));

    // Translate the right operand
    TranslateNode(node->GetChild(1));

    // Add the operation directly to the current continuation
    AppendDirectOperation(op);

    KAI_TRACE() << "Binary operation successfully translated in linear stream";
}

KAI_END