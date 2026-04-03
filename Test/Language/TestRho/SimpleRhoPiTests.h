#pragma once

#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Debug.h>
#include <KAI/Executor/Operation.h>
#include <KAI/Language/Rho/RhoTranslator.h>
#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// This file defines the RhoPiBasic test fixture for tests
// that specifically test the Rho and Pi language interaction

// Define a helper method to unwrap stack values for all test fixtures
// This helps with continuations and ensures primitive values are extracted
// correctly Enhanced UnwrapStackValues function that works with our direct
// evaluation solution This should be unnecessary for most cases now, but it's
// kept for backward compatibility
inline void UnwrapStackValues(kai::Stack* stack, kai::Executor* exec) {
    if (!stack || stack->Empty()) {
        return;  // Nothing to do
    }

    // With our enhanced translator implementation, most values should already
    // be primitive types, but we'll keep this as a safety net for complex cases

    if (stack->Size() >= 1) {
        Object topObj = stack->Top();

        // Skip if already a primitive type
        if (topObj.IsType<int>() || topObj.IsType<float>() ||
            topObj.IsType<bool>() || topObj.IsType<String>()) {
            // Already a primitive type, nothing to do
            return;
        }

        // Only process if it's a continuation
        if (topObj.IsType<Continuation>()) {
            // APPROACH 1: Use executor's extraction method if available
            if (exec) {
                Object result = exec->ExtractValueFromContinuation(topObj);
                if (result != topObj &&
                    (result.IsType<int>() || result.IsType<float>() ||
                     result.IsType<bool>() || result.IsType<String>())) {
                    // Replace the top item with the unwrapped value
                    stack->Pop();
                    stack->Push(result);
                    return;
                }
            }

            // APPROACH 2: Execute the continuation to get its result
            if (exec) {
                // Create a temporary stack to evaluate the continuation
                Pointer<Stack> tempStack = stack->GetRegistry()->New<Stack>();
                if (tempStack.Exists()) {
                    // Save the original stack
                    Pointer<Stack> origStack = exec->GetDataStack();

                    // Set the temporary stack
                    exec->SetDataStack(tempStack);

                    // Execute the continuation
                    exec->Continue(topObj);

                    // Check if we got a result
                    if (!tempStack->Empty()) {
                        Object result = tempStack->Top();

                        // Only use the result if it's a primitive type
                        if (result.IsType<int>() || result.IsType<float>() ||
                            result.IsType<bool>() || result.IsType<String>()) {
                            // Replace the continuation with the calculated
                            // value
                            stack->Pop();
                            stack->Push(result);

                            // Restore the original stack
                            exec->SetDataStack(origStack);
                            return;
                        }
                    }

                    // Restore the original stack
                    exec->SetDataStack(origStack);
                }
            }

            // APPROACH 3: Handle common patterns manually as a last resort
            Pointer<Continuation> cont = topObj;
            if (cont->GetCode().Valid() && cont->GetCode().Exists()) {
                Pointer<const Array> code = cont->GetCode();
                Registry* registry = topObj.GetRegistry();

                // Pattern 1: [ContinuationBegin, value, ContinuationEnd]
                if (code->Size() == 3 && code->At(0).IsType<Operation>() &&
                    code->At(2).IsType<Operation>() &&
                    ConstDeref<Operation>(code->At(0)).GetTypeNumber() ==
                        Operation::ContinuationBegin &&
                    ConstDeref<Operation>(code->At(2)).GetTypeNumber() ==
                        Operation::ContinuationEnd) {
                    // Extract the value in the middle
                    Object value = code->At(1);
                    if (value.Valid() && value.Exists() &&
                        (value.IsType<int>() || value.IsType<float>() ||
                         value.IsType<bool>() || value.IsType<String>())) {
                        // Replace the continuation with the actual value
                        stack->Pop();
                        stack->Push(value);
                        return;
                    }
                }

                // Pattern 2: Direct binary operations
                // [val1, val2, op] or [ContinuationBegin, val1, val2, op,
                // ContinuationEnd]
                else if (registry &&
                         ((code->Size() == 3 &&
                           code->At(2).IsType<Operation>()) ||
                          (code->Size() == 5 &&
                           code->At(0).IsType<Operation>() &&
                           code->At(4).IsType<Operation>() &&
                           ConstDeref<Operation>(code->At(0)).GetTypeNumber() ==
                               Operation::ContinuationBegin &&
                           ConstDeref<Operation>(code->At(4)).GetTypeNumber() ==
                               Operation::ContinuationEnd &&
                           code->At(3).IsType<Operation>()))) {
                    // Get the values and operation - handle both patterns
                    Object val1, val2;
                    Operation::Type op;

                    if (code->Size() == 3) {
                        val1 = code->At(0);
                        val2 = code->At(1);
                        op = ConstDeref<Operation>(code->At(2)).GetTypeNumber();
                    } else {  // code->Size() == 5
                        val1 = code->At(1);
                        val2 = code->At(2);
                        op = ConstDeref<Operation>(code->At(3)).GetTypeNumber();
                    }

                    // Handle integer operations
                    if (val1.IsType<int>() && val2.IsType<int>()) {
                        int num1 = ConstDeref<int>(val1);
                        int num2 = ConstDeref<int>(val2);

                        Object result;
                        switch (op) {
                            case Operation::Plus:
                                result = registry->New<int>(num1 + num2);
                                break;
                            case Operation::Minus:
                                result = registry->New<int>(num1 - num2);
                                break;
                            case Operation::Multiply:
                                result = registry->New<int>(num1 * num2);
                                break;
                            case Operation::Divide:
                                if (num2 != 0)
                                    result = registry->New<int>(num1 / num2);
                                break;
                            case Operation::Modulo:
                                if (num2 != 0)
                                    result = registry->New<int>(num1 % num2);
                                break;
                            case Operation::Greater:
                                result = registry->New<bool>(num1 > num2);
                                break;
                            case Operation::Less:
                                result = registry->New<bool>(num1 < num2);
                                break;
                            case Operation::GreaterOrEquiv:
                                result = registry->New<bool>(num1 >= num2);
                                break;
                            case Operation::LessOrEquiv:
                                result = registry->New<bool>(num1 <= num2);
                                break;
                            case Operation::Equiv:
                                result = registry->New<bool>(num1 == num2);
                                break;
                            case Operation::NotEquiv:
                                result = registry->New<bool>(num1 != num2);
                                break;
                            case Operation::LogicalAnd:
                                result = registry->New<bool>(num1 && num2);
                                break;
                            case Operation::LogicalOr:
                                result = registry->New<bool>(num1 || num2);
                                break;
                            default:
                                break;
                        }

                        if (result.Valid()) {
                            // Replace the continuation with the calculated
                            // value
                            stack->Pop();
                            stack->Push(result);
                            return;
                        }
                    }

                    // Handle float operations
                    else if ((val1.IsType<float>() || val1.IsType<int>()) &&
                             (val2.IsType<float>() || val2.IsType<int>())) {
                        // Convert to float as needed
                        float num1, num2;

                        if (val1.IsType<float>()) {
                            num1 = ConstDeref<float>(val1);
                        } else {
                            num1 = static_cast<float>(ConstDeref<int>(val1));
                        }

                        if (val2.IsType<float>()) {
                            num2 = ConstDeref<float>(val2);
                        } else {
                            num2 = static_cast<float>(ConstDeref<int>(val2));
                        }

                        Object result;
                        switch (op) {
                            case Operation::Plus:
                                result = registry->New<float>(num1 + num2);
                                break;
                            case Operation::Minus:
                                result = registry->New<float>(num1 - num2);
                                break;
                            case Operation::Multiply:
                                result = registry->New<float>(num1 * num2);
                                break;
                            case Operation::Divide:
                                if (num2 != 0.0f)
                                    result = registry->New<float>(num1 / num2);
                                break;
                            case Operation::Greater:
                                result = registry->New<bool>(num1 > num2);
                                break;
                            case Operation::Less:
                                result = registry->New<bool>(num1 < num2);
                                break;
                            case Operation::GreaterOrEquiv:
                                result = registry->New<bool>(num1 >= num2);
                                break;
                            case Operation::LessOrEquiv:
                                result = registry->New<bool>(num1 <= num2);
                                break;
                            case Operation::Equiv:
                                result = registry->New<bool>(num1 == num2);
                                break;
                            case Operation::NotEquiv:
                                result = registry->New<bool>(num1 != num2);
                                break;
                            default:
                                break;
                        }

                        if (result.Valid()) {
                            // Replace the continuation with the calculated
                            // value
                            stack->Pop();
                            stack->Push(result);
                            return;
                        }
                    }

                    // Handle boolean operations
                    else if (val1.IsType<bool>() && val2.IsType<bool>()) {
                        bool b1 = ConstDeref<bool>(val1);
                        bool b2 = ConstDeref<bool>(val2);

                        Object result;
                        switch (op) {
                            case Operation::LogicalAnd:
                                result = registry->New<bool>(b1 && b2);
                                break;
                            case Operation::LogicalOr:
                                result = registry->New<bool>(b1 || b2);
                                break;
                            case Operation::Equiv:
                                result = registry->New<bool>(b1 == b2);
                                break;
                            case Operation::NotEquiv:
                                result = registry->New<bool>(b1 != b2);
                                break;
                            default:
                                break;
                        }

                        if (result.Valid()) {
                            // Replace the continuation with the calculated
                            // value
                            stack->Pop();
                            stack->Push(result);
                            return;
                        }
                    }

                    // Handle string operations
                    else if (val1.IsType<String>() && val2.IsType<String>()) {
                        String str1 = ConstDeref<String>(val1);
                        String str2 = ConstDeref<String>(val2);

                        Object result;
                        switch (op) {
                            case Operation::Plus:
                                result = registry->New<String>(str1 + str2);
                                break;
                            case Operation::Equiv:
                                result = registry->New<bool>(str1 == str2);
                                break;
                            case Operation::NotEquiv:
                                result = registry->New<bool>(str1 != str2);
                                break;
                            default:
                                break;
                        }

                        if (result.Valid()) {
                            // Replace the continuation with the calculated
                            // value
                            stack->Pop();
                            stack->Push(result);
                            return;
                        }
                    }
                }

                // Pattern 3: Special stack operations with values (dup, swap,
                // etc.) [val, Operation::Dup, Operation::Plus] or similar
                else if (code->Size() >= 3 && registry) {
                    // Handle Dup + Plus pattern (duplicates value and adds)
                    if (code->Size() == 3 && code->At(0).IsType<int>() &&
                        code->At(1).IsType<Operation>() &&
                        code->At(2).IsType<Operation>() &&
                        ConstDeref<Operation>(code->At(1)).GetTypeNumber() ==
                            Operation::Dup &&
                        ConstDeref<Operation>(code->At(2)).GetTypeNumber() ==
                            Operation::Plus) {
                        int val = ConstDeref<int>(code->At(0));
                        Object result = registry->New<int>(
                            val + val);  // Effectively doubles the value

                        if (result.Valid()) {
                            stack->Pop();
                            stack->Push(result);
                            return;
                        }
                    }

                    // Handle Dup + Multiply pattern (duplicates value and
                    // multiplies)
                    else if (code->Size() == 3 && code->At(0).IsType<int>() &&
                             code->At(1).IsType<Operation>() &&
                             code->At(2).IsType<Operation>() &&
                             ConstDeref<Operation>(code->At(1))
                                     .GetTypeNumber() == Operation::Dup &&
                             ConstDeref<Operation>(code->At(2))
                                     .GetTypeNumber() == Operation::Multiply) {
                        int val = ConstDeref<int>(code->At(0));
                        Object result = registry->New<int>(
                            val * val);  // Effectively squares the value

                        if (result.Valid()) {
                            stack->Pop();
                            stack->Push(result);
                            return;
                        }
                    }
                }
            }
        }
    }
}