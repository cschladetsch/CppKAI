#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Pi/PiToken.h>
#include <KAI/Language/Pi/PiTranslator.h>
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
    KAI_TRACE() << "TranslateForEach: Starting foreach translation";

    // ForEach node structure:
    // - Child 0: loop variable (identifier)
    // - Child 1: collection expression
    // - Child 2: body block

    if (node->GetChildren().size() != 3) {
        KAI_TRACE_ERROR() << "TranslateForEach: Expected 3 children, got "
                          << static_cast<int>(node->GetChildren().size());
        Fail("ForEach node must have exactly 3 children");
        return;
    }

    auto loopVar = node->GetChild(0);
    auto collection = node->GetChild(1);
    auto body = node->GetChild(2);

    KAI_TRACE() << "TranslateForEach: Loop variable: "
                << loopVar->GetTokenText();

    // Translate the collection expression
    TranslateNode(collection);

    // Create a continuation for the body that:
    // 1. Stores the current element in the loop variable
    // 2. Executes the body
    // 3. Returns the result (or void)

    // Start a new continuation for the foreach body
    PushNew();

    // Store the current element in the loop variable
    // The element will be on the stack when this continuation is called
    // For Store operation, we need: [value, pathname]
    // The value is already on the stack, so just add the pathname
    String quotedPath = "'" + loopVar->GetTokenText();
    auto pathObj = reg_->New<Pathname>(Pathname(quotedPath));
    Append(pathObj);
    AppendDirectOperation(Operation::Store);

    // Translate the body
    TranslateNode(body);

    // The body should leave its result on the stack (or nothing if void)
    // ForEach will collect whatever is on the stack after each iteration

    // Get the body continuation
    auto bodyCont = Pop();

    // Push the body continuation and call ForEach
    Append(bodyCont);
    AppendDirectOperation(Operation::ForEach);

    KAI_TRACE() << "TranslateForEach: Completed foreach translation";
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
    KAI_TRACE() << "Translating if statement (linear stream version)";

    // If statements need at least condition and then-block
    if (node->GetChildren().size() < 2) {
        KAI_TRACE_ERROR() << "If node needs at least condition and then block";
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
        PushNew();
        TranslateNode(node->GetChild(2));
        auto elseCont = Pop();

        // Use IfElse operation
        // Pointer<T> inherits from Object, so we can use it directly
        Append(thenCont);
        Append(elseCont);
        AppendDirectOperation(Operation::IfElse);
    } else {
        // No else block - use If operation
        // Pointer<T> inherits from Object, so we can use it directly
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

    // Build the Pi code string from the tokens
    std::string piCode;
    auto tokenList = node->GetChild(0);

    for (const auto& tokenNode : tokenList->GetChildren()) {
        // Add space before each token except the first
        if (!piCode.empty()) {
            piCode += " ";
        }

        auto tokenType = tokenNode->GetToken().type;

        // Special handling for different token types
        if (tokenType == RhoTokenEnumType::Equiv) {
            // Pi uses == not = for equality
            piCode += "==";
        } else if (tokenType == RhoTokenEnumType::String) {
            // Preserve quotes around strings
            piCode += "\"" + tokenNode->Text() + "\"";
        } else {
            piCode += tokenNode->Text();
        }
    }

    KAI_TRACE() << "Pi code to execute: " << piCode;

    // Create a PiTranslator to translate the Pi code
    PiTranslator piTranslator(*reg_);
    piTranslator.trace = trace;

    // Translate the Pi code into a continuation
    auto piCont = piTranslator.Translate(piCode.c_str(), Structure::Expression);

    if (piTranslator.Failed) {
        KAI_TRACE_ERROR() << "Failed to translate Pi code: "
                          << piTranslator.Error;
        Fail("Failed to translate Pi code: " + piTranslator.Error);
        return;
    }

    if (!piCont.Exists()) {
        KAI_TRACE_ERROR() << "Pi translation returned null continuation";
        Fail("Pi translation returned null continuation");
        return;
    }

    // Append the Pi continuation directly
    Append(piCont);

    // Add Suspend operation to execute the Pi continuation
    AppendDirectOperation(Operation::Suspend);

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

    // Check if this is a built-in operation
    if (funcNode->GetType() == AstNodeEnum::TokenType &&
        funcNode->GetToken().type == TokenEnum::Label) {
        String funcName = funcNode->Text();
        KAI_TRACE() << "Function name: " << funcName;

        // Check for built-in operations
        if (funcName == "print") {
            // For print, translate arguments first, then use Print operation
            if (node->GetChildren().size() >= 2) {
                auto secondChild = node->GetChild(1);
                if (secondChild->GetType() == AstNodeEnum::ArgList) {
                    // Translate arguments from the ArgList node
                    for (const auto& arg : secondChild->GetChildren()) {
                        TranslateNode(arg);
                    }
                } else {
                    // Old style: arguments are direct children starting at
                    // index 1
                    for (size_t i = 1; i < node->GetChildren().size(); ++i) {
                        TranslateNode(node->GetChild(i));
                    }
                }
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
    if (node->GetChildren().size() >= 2) {
        auto secondChild = node->GetChild(1);
        if (secondChild->GetType() == AstNodeEnum::ArgList) {
            // Translate arguments from the ArgList node
            for (const auto& arg : secondChild->GetChildren()) {
                TranslateNode(arg);
            }
        } else {
            // Old style: arguments are direct children starting at index 1
            for (size_t i = 1; i < node->GetChildren().size(); ++i) {
                TranslateNode(node->GetChild(i));
            }
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

    // Extract parameters from the stack and store them
    // When the function is called, arguments are on the stack in the order they
    // were pushed For add(2, 3): stack has [2, 3] with 3 on top We need to
    // store them in the correct parameter variables

    // Process parameters from last to first (since last param is on top of
    // stack)
    for (int i = static_cast<int>(argNames.size()) - 1; i >= 0; --i) {
        // Pop the parameter value from stack and store it
        String quotedPath = "'" + argNames[i];
        auto pathObj = reg_->New<Pathname>(Pathname(quotedPath));
        Append(pathObj);
        AppendDirectOperation(Operation::Store);
    }

    // Translate the function body
    auto bodyNode = node->GetChild(2);
    KAI_TRACE() << "Translating function body into new continuation";
    TranslateNode(bodyNode);
    KAI_TRACE() << "Function body translation complete";

    // Ensure functions have a return statement
    // If the last operation isn't Return, add one
    auto cont = Top();
    if (cont.Exists() && cont.Valid()) {
        // Add return with no value (returns null)
        AppendDirectOperation(Operation::None);
        AppendDirectOperation(Operation::Return);
    }

    // Pop the function continuation
    auto functionCont = Pop();

    // Store the function continuation with its name
    if (!functionName.empty() && !functionName.StartsWith("_anon_")) {
        String quotedPath = "'" + functionName;
        auto pathObj = reg_->New<Pathname>(Pathname(quotedPath));

        // Push the function continuation and pathname
        Append(functionCont);
        Append(pathObj);
        AppendDirectOperation(Operation::Store);

        KAI_TRACE() << "Function " << functionName << " stored";
    } else {
        // For anonymous functions, just push the continuation
        Append(functionCont);
        KAI_TRACE() << "Anonymous function created";
    }
}

KAI_END