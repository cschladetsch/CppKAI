#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Registry.h>
#include <KAI/Language/Common/TranslatorCommon.h>

#include <iostream>

KAI_BEGIN

TranslatorCommon::TranslatorCommon(Registry &r) : ProcessCommon(r) {}

void TranslatorCommon::Append(Object const &ob) {
    try {
        KAI_TRACE() << "TranslatorCommon::Append - object type: "
                    << (ob.GetClass() ? ob.GetClass()->GetName().ToString() : "<null>")
                    << ", exists: " << ob.Exists();
                    
        if (stack.empty()) {
            KAI_TRACE_ERROR() << "TranslatorCommon::Append: Stack is empty";
            KAI_THROW_0(EmptyStack);
        }

        auto top = Top();
        if (!top.Exists()) {
            KAI_TRACE_ERROR()
                << "TranslatorCommon::Append: Top of stack is invalid";
            KAI_THROW_0(NullObject);
        }

        auto code = top->GetCode();
        if (!code.Exists()) {
            KAI_TRACE_ERROR()
                << "TranslatorCommon::Append: Code array is invalid";
            KAI_THROW_0(NullObject);
        }

        // The executor will extract primitive values from continuations
        // Translators will directly evaluate expressions and create properly
        // typed objects

        // PATCH: For binary operations with literal values, we directly
        // evaluate the result at translation time rather than creating a
        // continuation that will be evaluated later

        // For literal integers with binary operations, perform direct
        // evaluation
        if (Top()->GetCode()->Size() >= 2 && ob.IsType<Operation>()) {
            Operation::Type opType = ConstDeref<Operation>(ob).GetTypeNumber();

            // Only handle binary operations that have exactly 2 operands on the
            // stack
            if ((opType == Operation::Plus || opType == Operation::Minus ||
                 opType == Operation::Multiply || opType == Operation::Divide ||
                 opType == Operation::Greater || opType == Operation::Less ||
                 opType == Operation::LogicalAnd ||
                 opType == Operation::LogicalOr) &&
                code->Size() >= 2) {
                // Get the last two items in the code array
                Object val1 = code->At(code->Size() - 2);
                Object val2 = code->At(code->Size() - 1);

                if (val1.Valid() && val2.Valid()) {
                    // Handle integer operations
                    if (val1.IsType<int>() && val2.IsType<int>() &&
                        (opType == Operation::Plus ||
                         opType == Operation::Minus ||
                         opType == Operation::Multiply ||
                         opType == Operation::Divide)) {
                        // Extract the primitive values
                        int num1 = ConstDeref<int>(val1);
                        int num2 = ConstDeref<int>(val2);
                        int result = 0;

                        // Perform the operation directly
                        switch (opType) {
                            case Operation::Plus:
                                result = num1 + num2;
                                break;
                            case Operation::Minus:
                                result = num1 - num2;
                                break;
                            case Operation::Multiply:
                                result = num1 * num2;
                                break;
                            case Operation::Divide:
                                if (num2 != 0) result = num1 / num2;
                                break;
                            default:
                                break;
                        }

                        // Replace the operands with the result
                        code->RemoveAt(code->Size() - 1);      // Remove val2
                        code->RemoveAt(code->Size() - 1);      // Remove val1
                        code->Append(reg_->New<int>(result));  // Add the result

                        KAI_TRACE()
                            << "TranslatorCommon::Append: Directly evaluated "
                            << num1 << " " << Operation::ToString(opType) << " "
                            << num2 << " = " << result;
                        return;
                    }
                    // Handle boolean operations
                    else if (val1.IsType<bool>() && val2.IsType<bool>() &&
                             (opType == Operation::LogicalAnd ||
                              opType == Operation::LogicalOr)) {
                        // Extract the primitive values
                        bool b1 = ConstDeref<bool>(val1);
                        bool b2 = ConstDeref<bool>(val2);
                        bool result = false;

                        // Perform the operation directly
                        switch (opType) {
                            case Operation::LogicalAnd:
                                result = b1 && b2;
                                break;
                            case Operation::LogicalOr:
                                result = b1 || b2;
                                break;
                            default:
                                break;
                        }

                        // Replace the operands with the result
                        code->RemoveAt(code->Size() - 1);  // Remove val2
                        code->RemoveAt(code->Size() - 1);  // Remove val1
                        code->Append(
                            reg_->New<bool>(result));  // Add the result

                        KAI_TRACE()
                            << "TranslatorCommon::Append: Directly evaluated "
                            << (b1 ? "true" : "false") << " "
                            << Operation::ToString(opType) << " "
                            << (b2 ? "true" : "false") << " = "
                            << (result ? "true" : "false");
                        return;
                    }
                    // Handle string concatenation
                    else if (val1.IsType<String>() && val2.IsType<String>() &&
                             opType == Operation::Plus) {
                        // Extract the primitive values
                        String s1 = ConstDeref<String>(val1);
                        String s2 = ConstDeref<String>(val2);

                        // Perform the operation directly
                        String result = s1 + s2;

                        // Replace the operands with the result
                        code->RemoveAt(code->Size() - 1);  // Remove val2
                        code->RemoveAt(code->Size() - 1);  // Remove val1
                        code->Append(
                            reg_->New<String>(result));  // Add the result

                        KAI_TRACE() << "TranslatorCommon::Append: Directly "
                                       "evaluated string concatenation: "
                                    << s1 << " + " << s2 << " = " << result;
                        return;
                    }
                }
            }
        }

        // Add the object to the code array, with minimal logging
        code->Append(ob);
        KAI_TRACE() << "TranslatorCommon::Append - appended to code array, new size: " 
                    << code->Size();
    } catch (kai::Exception::Base &e) {
        KAI_TRACE_ERROR() << "Exception in TranslatorCommon::Append: "
                          << e.ToString();
        throw;
    } catch (std::exception &e) {
        KAI_TRACE_ERROR() << "Exception in TranslatorCommon::Append: "
                          << e.what();
        throw;
    } catch (...) {
        KAI_TRACE_ERROR() << "Unknown exception in TranslatorCommon::Append";
        throw;
    }
}

void TranslatorCommon::AppendOp(Operation::Type op) {
    std::cout << "Appending operation: " << Operation::ToString(op)
              << std::endl;

    // Create a new Operation object and add it directly to the code array
    Object opObject = reg_->New<Operation>(op);

    // The operation should be added directly to the current continuation's code
    // array
    Append(opObject);
}

void TranslatorCommon::AppendDirectOperation(Operation::Type op) {
    KAI_TRACE() << "Info: TranslatorCommon::AppendDirectOperation: "
                << Operation::ToString(op);

    // This method creates and adds an operation directly to the parent stack
    // instead of wrapping it in another continuation
    // This is used for Rho language binary operations to avoid unnecessary
    // nesting

    Object opObject = reg_->New<Operation>(op);

    // Add directly to the current continuation's code array
    if (stack.empty()) {
        KAI_TRACE_ERROR()
            << "TranslatorCommon::AppendDirectOperation: Stack is empty";
        return;  // Instead of throwing, just return
    }

    auto top = Top();
    if (!top.Exists()) {
        KAI_TRACE_ERROR() << "TranslatorCommon::AppendDirectOperation: Top of "
                             "stack is invalid";
        return;  // Instead of throwing, just return
    }

    auto code = top->GetCode();
    if (!code.Exists()) {
        KAI_TRACE_ERROR()
            << "TranslatorCommon::AppendDirectOperation: Code array is invalid";
        return;  // Instead of throwing, just return
    }

    code->Append(opObject);
}

void TranslatorCommon::MarkAsRhoExpression() {
    if (stack.empty()) {
        KAI_TRACE_ERROR()
            << "TranslatorCommon::MarkAsRhoExpression: Stack is empty";
        return;  // Instead of throwing, just return
    }

    auto top = Top();
    if (!top.Exists()) {
        KAI_TRACE_ERROR()
            << "TranslatorCommon::MarkAsRhoExpression: Top of stack is invalid";
        return;  // Instead of throwing, just return
    }

    // We'll skip setting properties for now since the Continuation class
    // doesn't have the RhoExpression property registered correctly
    // This will be implemented in a future PR when property handling is fixed

    // Instead, we'll rely on the language context in Console.cpp
    // to determine how to process continuations

    // Original code commented out to prevent errors:
    // top.SetPropertyValue(Label("RhoExpression"), reg_->New<bool>(true));
}

Pointer<Continuation> TranslatorCommon::Top() { return stack.back(); }

void TranslatorCommon::PushNew() {
    Pointer<Continuation> c = reg_->New<Continuation>();
    c->SetCode(reg_->New<Array>());

    // The executor will handle primitive type extraction during execution

    stack.push_back(c);
}

Pointer<Continuation> TranslatorCommon::Pop() {
    auto top = Top();
    stack.pop_back();
    return top;
}

std::string TranslatorCommon::ToString() const {
    StringStream str;
    for (auto ob : *stack.back()->GetCode()) str << ' ' << ob;
    return str.ToString().c_str();
}

KAI_END
