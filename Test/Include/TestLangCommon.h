#pragma once

#include <KAI/Console/Console.h>
#include <KAI/KAI.h>
#include <gtest/gtest.h>

#include "./TestCommon.h"

KAI_BEGIN

// Common for tests that work with the various
// languages in the system.
//
// TestLangCommon provides a fully-functional
// working test envionment with an interactive
// console and direct access to the data and
// context stacks for the Executor
class TestLangCommon : public TestCommon {
   public:
    TestLangCommon() = default;

   protected:
    void SetUp() override;
    void TearDown() override;

    void ExecScripts();
    void ExecScriptFile(const std::string &scriptName);

    // Get const ref to data at index on stack
    template <class T>
    const T &AtData(int index) {
        return Deref<T>(data_->At(index));
    }

    // get the current_ continuation context
    Continuation const &GetContext() const {
        return ConstDeref<Continuation>(context_->At(0));
    }

    template <class T>
    void AssertResult(const char *text, T const &val) {
        data_->Clear();
        console_.Execute(text);
        
        // Unwrap continuations before asserting the value
        if (!data_->Empty() && data_->Top().IsType<Continuation>()) {
            Object unwrapped = UnwrapContinuation(data_->Top());
            data_->Pop();
            data_->Push(unwrapped);
        }
        
        ASSERT_EQ(AtData<T>(0), val);
    }
    
    // Helper method to unwrap continuation objects into their primitive values
    Object UnwrapContinuation(Object value) {
        // If it's already a primitive type, no need for unwrapping
        if (value.IsType<int>() || value.IsType<bool>() || 
            value.IsType<float>() || value.IsType<double>() || 
            value.IsType<String>()) {
            return value;
        }
        
        // If it's a continuation, try unwrapping it
        if (value.IsType<Continuation>()) {
            // Get the continuation
            Pointer<Continuation> cont = value;
            
            // Make sure the continuation has valid code
            if (cont->GetCode().Valid() && cont->GetCode().Exists() && cont->GetCode()->Size() > 0) {
                // First, try the most common pattern: [val1, val2, op]
                if (cont->GetCode()->Size() == 3 && cont->GetCode()->At(2).IsType<Operation>()) {
                    Object val1 = cont->GetCode()->At(0);
                    Object val2 = cont->GetCode()->At(1);
                    Operation::Type op = ConstDeref<Operation>(cont->GetCode()->At(2)).GetTypeNumber();
                    
                    // Handle integer operations
                    if (val1.IsType<int>() && val2.IsType<int>()) {
                        int num1 = ConstDeref<int>(val1);
                        int num2 = ConstDeref<int>(val2);
                        int result = 0;
                        
                        switch (op) {
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
                                // Not a binary arithmetic op
                                break;
                        }
                        
                        return value.GetRegistry()->New<int>(result);
                    }
                    
                    // Handle boolean operations and comparisons
                    if ((op == Operation::Less || op == Operation::Greater || 
                         op == Operation::LessOrEquiv || op == Operation::GreaterOrEquiv || 
                         op == Operation::Equiv || op == Operation::NotEquiv) &&
                        val1.IsType<int>() && val2.IsType<int>()) {
                        
                        int num1 = ConstDeref<int>(val1);
                        int num2 = ConstDeref<int>(val2);
                        bool result = false;
                        
                        switch (op) {
                            case Operation::Less:
                                result = num1 < num2;
                                break;
                            case Operation::Greater:
                                result = num1 > num2;
                                break;
                            case Operation::LessOrEquiv:
                                result = num1 <= num2;
                                break;
                            case Operation::GreaterOrEquiv:
                                result = num1 >= num2;
                                break;
                            case Operation::Equiv:
                                result = num1 == num2;
                                break;
                            case Operation::NotEquiv:
                                result = num1 != num2;
                                break;
                            default:
                                // Not a comparison op
                                break;
                        }
                        
                        return value.GetRegistry()->New<bool>(result);
                    }
                }
            }
            
            // If we couldn't do a direct calculation, try the executor's unwrap method
            try {
                Object unwrapped = exec_->UnwrapValue(value);
                if (unwrapped != value) {
                    return unwrapped;
                }
            }
            catch (...) {
                // If unwrapping fails, we'll return the original value
            }
        }
        
        // If we couldn't unwrap, return the original value
        return value;
    }
    
    // Helper method to evaluate a Rho expression and unwrap the result
    Object EvaluateAndUnwrap(const std::string& expression) {
        console_.SetLanguage(Language::Rho);
        data_->Clear();
        
        // Execute the expression
        console_.Execute(expression);
        
        // Get the result and unwrap it if needed
        if (data_->Empty()) {
            return Object(); // No result
        }
        
        Object result = data_->Top();
        return UnwrapContinuation(result);
    }

   protected:
    Console console_;
    Stack *data_;
    const Stack *context_;
    Executor *exec_;
    Registry *reg_;
    Tree *tree_;
    Object root_;
};

KAI_END
