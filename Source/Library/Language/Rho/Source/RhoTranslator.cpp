#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Pi/PiLexer.h>
#include <KAI/Language/Pi/PiToken.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <concepts>
#include <format>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

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
            if (node->GetChildren().size() == 1) {
                // Unary minus: translate as 0 - expr
                Append(reg_->New<int>(0));
                TranslateNode(node->GetChild(0));
                AppendDirectOperation(Operation::Minus);
            } else {
                TranslateBinaryOp(node, Operation::Minus);
            }
            return;

        case RhoTokenEnumType::Plus:
            KAI_TRACE() << "Translating Plus operation";
            if (node->GetChildren().size() == 1) {
                // Unary plus: no-op, just translate operand
                TranslateNode(node->GetChild(0));
            } else {
                TranslateBinaryOp(node, Operation::Plus);
            }
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

        case RhoTokenEnumType::Question: {
            // Ternary conditional: cond ? thenExpr : elseExpr
            TranslateNode(node->GetChild(0));

            PushNew();
            TranslateNode(node->GetChild(1));
            auto thenCont = Pop();

            PushNew();
            TranslateNode(node->GetChild(2));
            auto elseCont = Pop();

            Append(thenCont);
            Append(elseCont);
            AppendDirectOperation(Operation::IfElse);
            return;
        }

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
            // In Rho, identifiers in expressions need to be resolved to their
            // values For Pi-style execution, push the label and then a Retrieve
            // operation
            auto labelObj = reg_->New<Label>(Label(node->Text()));

            // Append the label
            Append(labelObj);

            // Add Retrieve operation to resolve the label to its value
            AppendDirectOperation(Operation::Retreive);

            KAI_TRACE() << "Translated identifier as label with retrieve: "
                        << labelObj.ToString();
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
            // directly. This caused type mismatches because the executor
            // expects operations to execute, not pre-computed values.
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

        case RhoTokenEnumType::Break:
            AppendDirectOperation(Operation::Break);
            return;

        case RhoTokenEnumType::Continue:
            AppendDirectOperation(Operation::Continue);
            return;

        case RhoTokenEnumType::ShellCommand: {
            // Push the shell command string and execute it
            auto commandText = node->GetToken().Text();
            Append(New<String>(commandText));
            AppendDirectOperation(Operation::ShellCommand);
            return;
        }
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

    // Create continuation for initialization
    PushNew();
    TranslateNode(node->GetChild(0));
    auto initCont = Pop();

    // Create continuation for condition
    PushNew();
    TranslateNode(node->GetChild(1));
    auto condCont = Pop();

    // Create continuation for update
    PushNew();
    TranslateNode(node->GetChild(2));
    auto updateCont = Pop();

    // Create continuation for body
    PushNew();
    TranslateNode(node->GetChild(3));
    auto bodyCont = Pop();

    // Push continuations on stack for ForLoop operation (init, cond, incr,
    // body)
    Append(initCont);
    Append(condCont);
    Append(updateCont);
    Append(bodyCont);
    AppendDirectOperation(Operation::ForLoop);
}

void RhoTranslator::TranslateForEach(AstNodePtr node) {
    KAI_TRACE()
        << "TranslateForEach: Starting translation for 'for x in collection'";

    // ForEach node structure:
    // - Child 0: loop variable (identifier)
    // - Child 1: collection expression
    // - Child 2: body block
    //
    // Strategy: Translate `for x in collection` into a runtime foreach so it
    // works for arrays, lists, strings, and maps uniformly.

    if (node->GetChildren().size() != 3) {
        KAI_TRACE_ERROR() << "TranslateForEach: Expected 3 children, got "
                          << static_cast<int>(node->GetChildren().size());
        Fail("ForEach node must have exactly 3 children");
        return;
    }

    auto loopVar = node->GetChild(0);
    auto collection = node->GetChild(1);
    auto userBody = node->GetChild(2);

    String loopVarName = loopVar->GetTokenText();
    KAI_TRACE() << "TranslateForEach: Loop variable: " << loopVarName;

    // Build body continuation: loopVar = <element>; userBody
    PushNew();
    Append(New<Pathname>(Pathname("'" + loopVarName)));
    AppendDirectOperation(Operation::Store);
    TranslateNode(userBody);
    auto bodyCont = Pop();

    // Push collection and body continuation, then execute ForEach.
    TranslateNode(collection);
    Append(bodyCont);
    AppendDirectOperation(Operation::ForEach);

    KAI_TRACE() << "TranslateForEach: Completed translation";
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
    KAI_TRACE() << "TranslateIf: Starting, children count: "
                << static_cast<int>(node->GetChildren().size());

    // If statements need at least condition and then-block
    if (node->GetChildren().size() < 2) {
        KAI_TRACE_ERROR() << "TranslateIf: ERROR - not enough children";
        return;
    }

    // For linear stream execution, we'll use a different approach:
    // Instead of creating separate continuations, we'll inline the if block
    // but wrap it in a conditional execution marker

    // Translate condition first
    TranslateNode(node->GetChild(0));

    // Create a continuation for the then block
    PushNew();
    TranslateNode(node->GetChild(1));
    auto thenCont = Pop();

    if (node->GetChildren().size() > 2) {
        // Has else block
        KAI_TRACE() << "TranslateIf: Has else block, translating it";
        PushNew();
        TranslateNode(node->GetChild(2));
        auto elseCont = Pop();

        // Use IfElse operation
        // Pointer<T> inherits from Object, so we can use it directly
        KAI_TRACE() << "TranslateIf: Appending IfElse operation";
        Append(thenCont);
        Append(elseCont);
        AppendDirectOperation(Operation::IfElse);
    } else {
        // No else block - use If operation
        // Pointer<T> inherits from Object, so we can use it directly
        KAI_TRACE() << "TranslateIf: No else block, using If operation";
        Append(thenCont);
        AppendDirectOperation(Operation::If);
    }
    KAI_TRACE() << "TranslateIf: Done";
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

    // Get the number of key-value pairs (children should be in pairs)
    auto children = node->GetChildren();
    size_t numPairs = children.size() / 2;

    KAI_TRACE() << "Map has " << static_cast<int>(numPairs)
                << " key-value pairs";

    // Translate all key-value pairs
    for (size_t i = 0; i < children.size(); i += 2) {
        // Translate the key (should be a string token)
        auto keyNode = children[i];
        if (keyNode->GetType() == AstNodeEnum::TokenType &&
            keyNode->GetToken().type == TokenEnum::String) {
            // Push the string key
            auto keyText = keyNode->GetTokenText();
            Append(reg_->New<String>(keyText));
        } else {
            // For complex keys, translate the expression
            TranslateNode(keyNode);
        }

        // Translate the value
        if (i + 1 < children.size()) {
            TranslateNode(children[i + 1]);
        }
    }

    // Push the number of pairs
    AppendNew<int>(static_cast<int>(numPairs));

    // Use ToMap operation to create map from stack
    AppendDirectOperation(Operation::ToMap);

    KAI_TRACE() << "Created map with " << static_cast<int>(numPairs)
                << " entries";
}

void RhoTranslator::TranslateIndex(AstNodePtr node) {
    KAI_TRACE() << "TranslateIndex - Array indexing operation";

    // IndexOp should have 2 children: the array and the index
    auto children = node->GetChildren();
    if (children.size() != 2) {
        KAI_TRACE_ERROR() << "IndexOp should have exactly 2 children, got "
                          << static_cast<int>(children.size());
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

void RhoTranslator::TranslatePiBlock(AstNodePtr node) {
    KAI_TRACE() << "TranslatePiBlock - Translating embedded Pi code";

    // The Pi block should have one child containing the list of Pi tokens
    if (node->GetChildren().empty()) {
        KAI_TRACE_ERROR() << "Pi block has no content";
        return;
    }

    auto tokenList = node->GetChild(0);

    auto appendPiKeyword = [this](PiTokenEnumType::Enum piType) {
        switch (piType) {
            case PiTokenEnumType::Assert:
                AppendDirectOperation(Operation::Assert);
                break;
            case PiTokenEnumType::Not:
                AppendDirectOperation(Operation::LogicalNot);
                break;
            case PiTokenEnumType::And:
                AppendDirectOperation(Operation::LogicalAnd);
                break;
            case PiTokenEnumType::Or:
                AppendDirectOperation(Operation::LogicalOr);
                break;
            case PiTokenEnumType::Xor:
                AppendDirectOperation(Operation::LogicalXor);
                break;
            case PiTokenEnumType::Dup:
                AppendDirectOperation(Operation::Dup);
                break;
            case PiTokenEnumType::Dup2:
                AppendDirectOperation(Operation::Dup2);
                break;
            case PiTokenEnumType::Drop:
                AppendDirectOperation(Operation::Drop);
                break;
            case PiTokenEnumType::Drop2:
                AppendDirectOperation(Operation::Drop2);
                break;
            case PiTokenEnumType::Over:
                AppendDirectOperation(Operation::Over);
                break;
            case PiTokenEnumType::Swap:
                AppendDirectOperation(Operation::Swap);
                break;
            case PiTokenEnumType::Rot:
                AppendDirectOperation(Operation::Rot);
                break;
            case PiTokenEnumType::RotN:
                AppendDirectOperation(Operation::RotN);
                break;
            case PiTokenEnumType::Roll:
                AppendDirectOperation(Operation::Roll);
                break;
            case PiTokenEnumType::PickN:
                AppendDirectOperation(Operation::Pick);
                break;
            case PiTokenEnumType::Depth:
                AppendDirectOperation(Operation::Depth);
                break;
            case PiTokenEnumType::Clear:
                AppendDirectOperation(Operation::Clear);
                break;
            case PiTokenEnumType::Min:
                AppendDirectOperation(Operation::Min);
                break;
            case PiTokenEnumType::Max:
                AppendDirectOperation(Operation::Max);
                break;
            case PiTokenEnumType::Modulo:
                AppendDirectOperation(Operation::Modulo);
                break;
            case PiTokenEnumType::ToArray:
                AppendDirectOperation(Operation::ToArray);
                break;
            case PiTokenEnumType::ToList:
                AppendDirectOperation(Operation::ToList);
                break;
            case PiTokenEnumType::ToMap:
                AppendDirectOperation(Operation::ToMap);
                break;
            case PiTokenEnumType::ToSet:
                AppendDirectOperation(Operation::ToSet);
                break;
            case PiTokenEnumType::Size:
                AppendDirectOperation(Operation::Size);
                break;
            case PiTokenEnumType::New:
                AppendDirectOperation(Operation::New);
                break;
            case PiTokenEnumType::Print:
                AppendDirectOperation(Operation::Print);
                break;
            case PiTokenEnumType::DropN:
                AppendDirectOperation(Operation::DropN);
                break;
            case PiTokenEnumType::SetChild:
                AppendDirectOperation(Operation::SetChild);
                break;
            case PiTokenEnumType::Freeze:
                AppendDirectOperation(Operation::Freeze);
                break;
            case PiTokenEnumType::Thaw:
                AppendDirectOperation(Operation::Thaw);
                break;
            case PiTokenEnumType::Suspend:
                AppendDirectOperation(Operation::Suspend);
                break;
            case PiTokenEnumType::Replace:
                AppendDirectOperation(Operation::Replace);
                break;
            case PiTokenEnumType::Resume:
                AppendDirectOperation(Operation::Resume);
                break;
            default:
                Fail(std::format("Unsupported Pi keyword in Pi block: {}",
                                 PiTokenEnumType::ToString(piType)));
                break;
        }
    };

    for (const auto& tokenNode : tokenList->GetChildren()) {
        const auto& rhoTok = tokenNode->GetToken();
        auto tokenType = rhoTok.type;

        // Skip layout tokens from the Rho lexer.
        if (tokenType == RhoTokenEnumType::Whitespace ||
            tokenType == RhoTokenEnumType::NewLine ||
            tokenType == RhoTokenEnumType::Tab ||
            tokenType == RhoTokenEnumType::Comment) {
            continue;
        }

        switch (tokenType) {
            case RhoTokenEnumType::Int:
                AppendNew(std::stoi(tokenNode->Text()));
                break;
            case RhoTokenEnumType::Float:
                AppendNew(std::stof(tokenNode->Text()));
                break;
            case RhoTokenEnumType::String:
                AppendNew(String(tokenNode->Text()));
                break;
            case RhoTokenEnumType::True:
                AppendNew(true);
                break;
            case RhoTokenEnumType::False:
                AppendNew(false);
                break;
            case RhoTokenEnumType::Label: {
                PiTokenEnumType::Enum keyword = PiTokenEnumType::None;
                if (PiLexer::TryGetKeyword(tokenNode->Text(), keyword)) {
                    appendPiKeyword(keyword);
                } else if (tokenNode->Text() == "neg") {
                    AppendNew(0);
                    AppendDirectOperation(Operation::Swap);
                    AppendDirectOperation(Operation::Minus);
                } else {
                    AppendNew(Label(tokenNode->Text()));
                    AppendDirectOperation(Operation::Retreive);
                }
                break;
            }
            case RhoTokenEnumType::Pathname:
                AppendNew(Pathname(tokenNode->Text()));
                break;
            case RhoTokenEnumType::ShellCommand:
                AppendNew(String(tokenNode->Text()));
                AppendDirectOperation(Operation::ShellCommand);
                break;
            case RhoTokenEnumType::Plus:
                AppendDirectOperation(Operation::Plus);
                break;
            case RhoTokenEnumType::Minus:
                AppendDirectOperation(Operation::Minus);
                break;
            case RhoTokenEnumType::Mul:
                AppendDirectOperation(Operation::Multiply);
                break;
            case RhoTokenEnumType::Divide:
                AppendDirectOperation(Operation::Divide);
                break;
            case RhoTokenEnumType::Mod:
                AppendDirectOperation(Operation::Modulo);
                break;
            case RhoTokenEnumType::Equiv:
                AppendDirectOperation(Operation::Equiv);
                break;
            case RhoTokenEnumType::NotEquiv:
                AppendDirectOperation(Operation::NotEquiv);
                break;
            case RhoTokenEnumType::Less:
                AppendDirectOperation(Operation::Less);
                break;
            case RhoTokenEnumType::Greater:
                AppendDirectOperation(Operation::Greater);
                break;
            case RhoTokenEnumType::LessEquiv:
                AppendDirectOperation(Operation::LessOrEquiv);
                break;
            case RhoTokenEnumType::GreaterEquiv:
                AppendDirectOperation(Operation::GreaterOrEquiv);
                break;
            case RhoTokenEnumType::And:
                AppendDirectOperation(Operation::LogicalAnd);
                break;
            case RhoTokenEnumType::Or:
                AppendDirectOperation(Operation::LogicalOr);
                break;
            case RhoTokenEnumType::Xor:
                AppendDirectOperation(Operation::LogicalXor);
                break;
            case RhoTokenEnumType::BitAnd:
                AppendDirectOperation(Operation::Suspend);
                break;
            case RhoTokenEnumType::BitOr:
                AppendDirectOperation(Operation::BitwiseOr);
                break;
            case RhoTokenEnumType::BitXor:
                AppendDirectOperation(Operation::BitwiseXor);
                break;
            case RhoTokenEnumType::Lookup:
                AppendDirectOperation(Operation::Lookup);
                break;
            case RhoTokenEnumType::Assert:
                AppendDirectOperation(Operation::Assert);
                break;
            case RhoTokenEnumType::Suspend:
                AppendDirectOperation(Operation::Suspend);
                break;
            case RhoTokenEnumType::Resume:
            case RhoTokenEnumType::Replace:
                AppendDirectOperation(Operation::Resume);
                break;
            case RhoTokenEnumType::Not:
                AppendDirectOperation(Operation::Replace);
                break;
            default:
                Fail(std::format("Unsupported token in Pi block: {}",
                                 RhoTokenEnumType::ToString(tokenType)));
                return;
        }
    }

    KAI_TRACE() << "Pi block translation complete";
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

    // Special handling for assignment operations - they need identifier names,
    // not values
    if (op == Operation::Store || op == Operation::PlusEquals ||
        op == Operation::MinusEquals || op == Operation::MulEquals ||
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
                KAI_TRACE_ERROR()
                    << "IndexOp should have 2 children for assignment";
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
            AppendDirectOperation(
                Operation::Rot);  // Now: [array, index, value]

            // Apply SetChild operation
            AppendDirectOperation(Operation::SetChild);

            // SetChild leaves the modified array on the stack
            // But Store expects [value, name], so we need to skip the Store
            // operation
            return;  // Skip the Store operation below

        } else if (identNode->GetType() == AstNodeEnum::GetMember) {
            // Special handling for member assignment: obj.member = value
            KAI_TRACE() << "Handling member assignment";

            auto objectNode = identNode->GetChild(0);
            auto memberNode = identNode->GetChild(1);

            // Translate the object expression
            TranslateNode(objectNode);

            // Push member name as a string key
            if (memberNode->GetType() == AstNodeEnum::TokenType &&
                memberNode->GetToken().type == RhoTokenEnumType::Label) {
                Append(reg_->New<String>(String(memberNode->GetTokenText())));
            } else {
                TranslateNode(memberNode);
            }

            // Stack now has: [value, object, key]
            AppendDirectOperation(Operation::Rot);  // [object, key, value]
            AppendDirectOperation(Operation::SetChild);

            // SetChild leaves the modified container on the stack; skip Store
            return;

        } else if (identNode->GetToken().type == RhoTokenEnumType::Label) {
            // Push the identifier name as a quoted Pathname for assignment
            KAI_TRACE() << "Appending pathname for assignment: "
                        << identNode->Text();
            // Create a quoted pathname by prepending '
            String quotedPath = "'" + identNode->Text();
            auto pathObj = reg_->New<Pathname>(Pathname(quotedPath));
            KAI_TRACE() << "Created pathname object: "
                        << (pathObj.Exists() ? "exists" : "null") << ", type: "
                        << (pathObj.GetClass()
                                ? pathObj.GetClass()->GetName().ToString()
                                : "<null>");
            Append(pathObj);
        } else {
            // If it's not a simple identifier or index op, translate it
            // normally
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

void RhoTranslator::TranslateCall(AstNodePtr node) {
    KAI_TRACE() << "Translating function call";

    // Call nodes have: function identifier and arguments
    if (node->GetChildren().empty()) {
        KAI_TRACE_ERROR() << "Call node needs at least a function identifier";
        return;
    }

    // Get function name
    auto funcNode = node->GetChild(0);
    std::vector<AstNodePtr> callArgs;

    // Normalize call arguments regardless of parser style.
    if (node->GetChildren().size() >= 2) {
        auto secondChild = node->GetChild(1);
        if (secondChild->GetType() == AstNodeEnum::ArgList) {
            for (const auto& arg : secondChild->GetChildren()) {
                callArgs.push_back(arg);
            }
        } else {
            for (size_t i = 1; i < node->GetChildren().size(); ++i) {
                callArgs.push_back(node->GetChild(i));
            }
        }
    }

    // Built-in member calls on containers.
    // Handles: obj.size(), obj.keys(), obj.push(x), obj.slice(a, b)
    if (funcNode->GetType() == AstNodeEnum::GetMember &&
        funcNode->GetChildren().size() >= 2) {
        auto objectNode = funcNode->GetChild(0);
        auto memberNode = funcNode->GetChild(1);

        if (memberNode->GetType() == AstNodeEnum::TokenType &&
            memberNode->GetToken().type == TokenEnum::Label) {
            String memberName = memberNode->Text();

            if (memberName == "size" && callArgs.empty()) {
                TranslateNode(objectNode);
                AppendDirectOperation(Operation::Size);
                return;
            }

            if (memberName == "keys" && callArgs.empty()) {
                TranslateNode(objectNode);
                AppendDirectOperation(Operation::MapKeys);
                return;
            }

            if (memberName == "push" && callArgs.size() == 1) {
                TranslateNode(objectNode);
                TranslateNode(callArgs[0]);
                AppendDirectOperation(Operation::ArrayPush);
                return;
            }

            if (memberName == "slice" && callArgs.size() == 2) {
                TranslateNode(objectNode);
                TranslateNode(callArgs[0]);
                TranslateNode(callArgs[1]);
                AppendDirectOperation(Operation::ArraySlice);
                return;
            }
        }
    }

    // Check if this is a built-in operation
    if (funcNode->GetType() == AstNodeEnum::TokenType &&
        funcNode->GetToken().type == TokenEnum::Label) {
        String funcName = funcNode->Text();
        KAI_TRACE() << "Function name: " << funcName;

        // Check for built-in operations
        if (funcName == "print") {
            // For print, translate arguments first, then use Print operation
            for (const auto& arg : callArgs) {
                TranslateNode(arg);
            }

            // Generate Print operation directly
            AppendDirectOperation(Operation::Print);
            KAI_TRACE()
                << "Generated Print operation for built-in print function";
            return;
        }
        // Add other built-in operations here as needed
    }

    // For non-built-in functions, use the standard function call mechanism
    // Check if we have an ArgList node (parser creates Call with [function,
    // ArgList])
    if (!callArgs.empty()) {
        for (const auto& arg : callArgs) {
            TranslateNode(arg);
        }
    }

    // Now translate the function identifier (this pushes the continuation)
    TranslateNode(funcNode);

    // Add the call operation - Suspend expects the continuation on top of stack
    AppendDirectOperation(Operation::Suspend);

    KAI_TRACE() << "Function call successfully translated";
}

void RhoTranslator::TranslateBlock(AstNodePtr node) {
    KAI_TRACE() << "Translating block";

    // A block is just a sequence of statements
    for (const auto& child : node->GetChildren()) {
        TranslateNode(child);
    }

    KAI_TRACE() << "Block successfully translated";
}

void RhoTranslator::TranslateFunction(AstNodePtr node) {
    KAI_TRACE() << "Translating function definition";

    // Function nodes have: name (or placeholder), args list, body
    if (node->GetChildren().size() < 3) {
        KAI_TRACE_ERROR() << "Function node needs name, args, and body";
        return;
    }

    // Get function name
    auto nameNode = node->GetChild(0);
    String functionName;

    if (nameNode->GetType() == AstNodeEnum::TokenType &&
        nameNode->GetToken().type == TokenEnum::Label) {
        functionName = nameNode->Text();
    } else {
        // Anonymous function - generate a unique name
        static int anonCounter = 0;
        functionName = std::format("_anon_{}", anonCounter++);
    }

    KAI_TRACE() << "Function name: " << functionName;

    // Get arguments
    auto argsNode = node->GetChild(1);
    std::vector<String> argNames;

    for (const auto& arg : argsNode->GetChildren()) {
        if (arg->GetType() == AstNodeEnum::TokenType &&
            arg->GetToken().type == TokenEnum::Label) {
            argNames.push_back(arg->Text());
            KAI_TRACE() << "  Arg: " << arg->Text();
        }
    }

    // Create a new continuation for the function body
    PushNew();

    // Translate the function body
    auto bodyNode = node->GetChild(2);
    KAI_TRACE() << "Translating function body into new continuation";
    TranslateNode(bodyNode);
    KAI_TRACE() << "Function body translation complete";

    // Function body has been translated
    // Note: We don't automatically add a return statement anymore
    // Functions should explicitly return values or return nothing

    // Pop the function continuation
    auto functionCont = Pop();

    // Add args in reverse order so Enter() pops values correctly.
    if (!argNames.empty()) {
        Pointer<Continuation> cont = functionCont;
        if (cont.Exists()) {
            for (auto it = argNames.rbegin(); it != argNames.rend(); ++it) {
                cont->AddArg(Label(it->c_str()));
            }
        }
    }

    // Append the function continuation
    Append(functionCont);

    // For named functions, store them with their name
    if (!functionName.empty() && !functionName.StartsWith("_anon_")) {
        String quotedPath = "'" + functionName;
        auto pathObj = reg_->New<Pathname>(Pathname(quotedPath));
        Append(pathObj);
        AppendDirectOperation(Operation::Store);
        KAI_TRACE() << "Function " << functionName << " stored";
    } else {
        KAI_TRACE() << "Anonymous function created";
    }
}

KAI_END
