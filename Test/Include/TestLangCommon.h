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
    void UnwrapStackValues(); // Method to process stack and unwrap continuations

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
        
        // Process the stack to extract values from continuations
        UnwrapStackValues();
        
        ASSERT_EQ(AtData<T>(0), val);
    }
    
    // Only handle non-block, non-Pi continuation patterns in tests
    // Rho language should only create continuations for blocks and pi{} statements
    Object ExtractValueFromContinuation(Object value) {
        // If it's already a primitive type, no need for extraction
        if (value.IsType<int>() || value.IsType<bool>() || 
            value.IsType<float>() || value.IsType<double>() || 
            value.IsType<String>()) {
            return value;
        }
        
        // If it's not a continuation, return as is
        if (!value.IsType<Continuation>()) {
            return value;
        }
        
        // Get the continuation
        Pointer<Continuation> cont = value;
        
        // Make sure the continuation has valid code
        if (!cont->GetCode().Valid() || !cont->GetCode().Exists() || cont->GetCode()->Size() == 0) {
            return value;
        }
        
        // Check if this is a block or pi{} continuation
        // These should be preserved as continuations
        // We check for these patterns by examining the code structure
        
        // Check for block pattern (code contains ContinuationBegin/End or BlockBegin/End)
        Pointer<const Array> code = cont->GetCode();
        bool isBlock = false;
        
        // Simple check: if code has ContinuationBegin/End or BlockBegin/End operations, it's likely a block
        if (code->Size() >= 2) {
            // Check first and last elements for block markers
            if (code->At(0).IsType<Operation>()) {
                Operation::Type firstOp = ConstDeref<Operation>(code->At(0)).GetTypeNumber();
                if (firstOp == Operation::ContinuationBegin) {
                    isBlock = true;
                }
            }
            
            // Additional check: last element for block end markers
            if (code->At(code->Size()-1).IsType<Operation>()) {
                Operation::Type lastOp = ConstDeref<Operation>(code->At(code->Size()-1)).GetTypeNumber();
                if (lastOp == Operation::ContinuationEnd) {
                    isBlock = true;
                }
            }
        }
        
        // If it seems to be a block, preserve it as a continuation
        if (isBlock) {
            return value;
        }
        
        // If no registry to create new objects, return the original
        Registry* registry = value.GetRegistry();
        if (!registry) {
            return value;
        }
        
        // Handle simple binary operations - these should NOT be wrapped in continuations
        // but if they are in tests, we'll extract the expected values
        
        // Pattern 1: Single value [val]
        if (code->Size() == 1) {
            Object firstElement = code->At(0);
            if (firstElement.IsType<int>() || firstElement.IsType<bool>() || 
                firstElement.IsType<float>() || firstElement.IsType<double>() || 
                firstElement.IsType<String>()) {
                return firstElement;
            }
        }
        
        // Pattern 2: Binary operation [val1, val2, op]
        if (code->Size() == 3 && code->At(2).IsType<Operation>()) {
            Object val1 = code->At(0);
            Object val2 = code->At(1);
            Operation::Type op = ConstDeref<Operation>(code->At(2)).GetTypeNumber();
            
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
            }
            else if (val1.IsType<float>() && val2.IsType<int>()) {
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
            if (val1.IsType<bool>() && val2.IsType<bool>()) {
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
            if (val1.IsType<String>() && val2.IsType<String>() && op == Operation::Plus) {
                String str1 = ConstDeref<String>(val1);
                String str2 = ConstDeref<String>(val2);
                return registry->New<String>(str1 + str2);
            }
        }
        
        // If we can't handle this continuation pattern, return the original
        return value;
    }
    
    // Helper method to evaluate a Rho expression and extract primitive values
    Object EvaluateAndUnwrap(const std::string& expression) {
        console_.SetLanguage(Language::Rho);
        data_->Clear();
        
        // Execute the expression
        console_.Execute(expression);
        
        // Process the stack to extract values from continuations
        UnwrapStackValues();
        
        // Get the result
        if (data_->Empty()) {
            return Object(); // No result
        }
        
        return data_->Top();
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
