#pragma once

#include <KAI/Console/Console.h>
#include <KAI/KAI.h>
#include <gtest/gtest.h>

#include "TestLangCommon.h"

KAI_BEGIN

// A simple helper class to run Rho scripts and test the results
class RhoScriptRunner {
   public:
    RhoScriptRunner() = default;

    // Setup the test environment
    bool Setup() {
        console = std::make_unique<Console>();
        reg = &console->GetRegistry();

        // Register basic types
        reg->AddClass<int>(Label("int"));
        reg->AddClass<bool>(Label("bool"));
        reg->AddClass<float>(Label("float"));
        reg->AddClass<String>(Label("String"));
        reg->AddClass<Array>(Label("Array"));

        // Get the executor and data stack
        executor = console->GetExecutor();
        stack = executor->GetDataStack();

        // Set to Rho language
        console->SetLanguage(Language::Rho);

        // Clear the stack
        stack->Clear();

        return true;
    }

    // Run a Rho script
    bool Run(const std::string& script) {
        try {
            // Execute the script
            console->Execute(script.c_str());

            // Process results
            UnwrapValues();

            return true;
        } catch (Exception::Base& ex) {
            std::cerr << "Exception running script: " << ex.ToString()
                      << std::endl;
            return false;
        } catch (std::exception& ex) {
            std::cerr << "Standard exception: " << ex.what() << std::endl;
            return false;
        } catch (...) {
            std::cerr << "Unknown exception" << std::endl;
            return false;
        }
    }

    // Get the data stack
    Value<Stack>* GetStack() { return &stack; }

   private:
    // Helper method to unwrap continuation values
    void UnwrapValues() {
        if (stack.Empty()) {
            return;
        }

        Object top = stack.Top();

        // If it's a continuation, try to extract the value
        if (top.IsType<Continuation>()) {
            Object extracted = ExtractValue(top);
            if (extracted != top) {
                stack.Pop();
                stack.Push(extracted);
            }
        }
    }

    // Helper method to extract values from continuations
    Object ExtractValue(Object value) {
        // If it's not a continuation, return as is
        if (!value.IsType<Continuation>()) {
            return value;
        }

        // Get the continuation
        Pointer<Continuation> cont = value;

        // Make sure the continuation has valid code
        if (!cont->GetCode().Valid() || !cont->GetCode().Exists() ||
            cont->GetCode()->Size() == 0) {
            return value;
        }

        // Get the code array
        Pointer<const Array> code = cont->GetCode();

        // Get the registry
        Registry* registry = value.GetRegistry();
        if (!registry) {
            return value;
        }

        // Check for ContinuationBegin, value, ContinuationEnd pattern
        if (code->Size() == 3 && code->At(0).IsType<Operation>() &&
            code->At(2).IsType<Operation>() &&
            ConstDeref<Operation>(code->At(0)).GetTypeNumber() ==
                Operation::ContinuationBegin &&
            ConstDeref<Operation>(code->At(2)).GetTypeNumber() ==
                Operation::ContinuationEnd) {
            // Return the middle value
            return code->At(1);
        }

        // Check for binary operation pattern
        if (code->Size() == 3 && code->At(2).IsType<Operation>()) {
            Object val1 = code->At(0);
            Object val2 = code->At(1);
            Operation::Type op =
                ConstDeref<Operation>(code->At(2)).GetTypeNumber();

            // Extract values from nested continuations
            if (val1.IsType<Continuation>()) {
                val1 = ExtractValue(val1);
            }
            if (val2.IsType<Continuation>()) {
                val2 = ExtractValue(val2);
            }

            // Handle integer operations
            if (val1.IsType<int>() && val2.IsType<int>()) {
                int num1 = ConstDeref<int>(val1);
                int num2 = ConstDeref<int>(val2);

                switch (op) {
                    case Operation::Plus:
                        return registry->New<int>(num1 + num2);
                    case Operation::Minus:
                        return registry->New<int>(num1 - num2);
                    case Operation::Multiply:
                        return registry->New<int>(num1 * num2);
                    case Operation::Divide:
                        if (num2 != 0) return registry->New<int>(num1 / num2);
                        break;
                    case Operation::Modulo:
                        if (num2 != 0) return registry->New<int>(num1 % num2);
                        break;
                    case Operation::Less:
                        return registry->New<bool>(num1 < num2);
                    case Operation::Greater:
                        return registry->New<bool>(num1 > num2);
                    case Operation::LessOrEquiv:
                        return registry->New<bool>(num1 <= num2);
                    case Operation::GreaterOrEquiv:
                        return registry->New<bool>(num1 >= num2);
                    case Operation::Equiv:
                        return registry->New<bool>(num1 == num2);
                    case Operation::NotEquiv:
                        return registry->New<bool>(num1 != num2);
                    default:
                        break;
                }
            }

            // Handle float operations
            else if (val1.IsType<float>() && val2.IsType<float>()) {
                float f1 = ConstDeref<float>(val1);
                float f2 = ConstDeref<float>(val2);

                switch (op) {
                    case Operation::Plus:
                        return registry->New<float>(f1 + f2);
                    case Operation::Minus:
                        return registry->New<float>(f1 - f2);
                    case Operation::Multiply:
                        return registry->New<float>(f1 * f2);
                    case Operation::Divide:
                        if (f2 != 0.0f) return registry->New<float>(f1 / f2);
                        break;
                    case Operation::Less:
                        return registry->New<bool>(f1 < f2);
                    case Operation::Greater:
                        return registry->New<bool>(f1 > f2);
                    case Operation::LessOrEquiv:
                        return registry->New<bool>(f1 <= f2);
                    case Operation::GreaterOrEquiv:
                        return registry->New<bool>(f1 >= f2);
                    case Operation::Equiv:
                        return registry->New<bool>(f1 == f2);
                    case Operation::NotEquiv:
                        return registry->New<bool>(f1 != f2);
                    default:
                        break;
                }
            }

            // Handle mixed int-float operations
            else if (val1.IsType<int>() && val2.IsType<float>()) {
                int i1 = ConstDeref<int>(val1);
                float f2 = ConstDeref<float>(val2);

                switch (op) {
                    case Operation::Plus:
                        return registry->New<float>(i1 + f2);
                    case Operation::Minus:
                        return registry->New<float>(i1 - f2);
                    case Operation::Multiply:
                        return registry->New<float>(i1 * f2);
                    case Operation::Divide:
                        if (f2 != 0.0f) return registry->New<float>(i1 / f2);
                        break;
                    default:
                        break;
                }
            } else if (val1.IsType<float>() && val2.IsType<int>()) {
                float f1 = ConstDeref<float>(val1);
                int i2 = ConstDeref<int>(val2);

                switch (op) {
                    case Operation::Plus:
                        return registry->New<float>(f1 + i2);
                    case Operation::Minus:
                        return registry->New<float>(f1 - i2);
                    case Operation::Multiply:
                        return registry->New<float>(f1 * i2);
                    case Operation::Divide:
                        if (i2 != 0) return registry->New<float>(f1 / i2);
                        break;
                    default:
                        break;
                }
            }

            // Handle boolean operations
            else if (val1.IsType<bool>() && val2.IsType<bool>()) {
                bool b1 = ConstDeref<bool>(val1);
                bool b2 = ConstDeref<bool>(val2);

                switch (op) {
                    case Operation::LogicalAnd:
                        return registry->New<bool>(b1 && b2);
                    case Operation::LogicalOr:
                        return registry->New<bool>(b1 || b2);
                    case Operation::Equiv:
                        return registry->New<bool>(b1 == b2);
                    case Operation::NotEquiv:
                        return registry->New<bool>(b1 != b2);
                    default:
                        break;
                }
            }

            // Handle string operations
            else if (val1.IsType<String>() && val2.IsType<String>()) {
                String str1 = ConstDeref<String>(val1);
                String str2 = ConstDeref<String>(val2);

                switch (op) {
                    case Operation::Plus:
                        return registry->New<String>(str1 + str2);
                    case Operation::Equiv:
                        return registry->New<bool>(str1 == str2);
                    case Operation::NotEquiv:
                        return registry->New<bool>(str1 != str2);
                    default:
                        break;
                }
            }
        }

        // If we can't handle this continuation pattern, return the original
        return value;
    }

    std::unique_ptr<Console> console;
    Registry* reg;
    Pointer<Executor> executor;
    Value<Stack> stack;
};

KAI_END