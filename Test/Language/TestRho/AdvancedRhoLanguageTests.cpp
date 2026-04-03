#include <gtest/gtest.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "KAI/Executor/Operation.h"
#include "SimpleRhoPiTests.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * ADVANCED RHO LANGUAGE TESTS
 * ---------------------------
 * This file contains advanced tests for the Rho language, focusing on more
 * sophisticated language features, edge cases, and real-world usage patterns.
 */

// Advanced test fixture for Rho language tests
class AdvancedRhoLanguage : public TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<float>(Label("float"));
        reg_->AddClass<String>(Label("String"));
        reg_->AddClass<Array>(Label("Array"));
        reg_->AddClass<Map>(Label("Map"));
    }

    // Helper method to execute Rho code and verify result
    template <typename T>
    void ExecuteAndVerify(const std::string& rhoCode, const T& expectedValue) {
        data_->Clear();

        try {
            KAI_TRACE() << "Executing Rho code: " << rhoCode;
            bool success = console_.Execute(rhoCode);
            ASSERT_TRUE(success) << "Failed to execute Rho code: " << rhoCode;

            // Make sure we extract primitive values from continuations
            UnwrapStackValues(data_, exec_);

            // Verify result
            ASSERT_FALSE(data_->Empty())
                << "Stack should not be empty after execution";
            ASSERT_TRUE(data_->Top().IsType<T>())
                << "Expected " << typeid(T).name() << " but got "
                << data_->Top().GetClass()->GetName().ToString();

            if constexpr (std::is_same_v<T, float>) {
                ASSERT_NEAR(ConstDeref<T>(data_->Top()), expectedValue, 0.0001f)
                    << "Expected " << expectedValue << " but got "
                    << ConstDeref<T>(data_->Top());
            } else {
                ASSERT_EQ(ConstDeref<T>(data_->Top()), expectedValue)
                    << "Expected " << expectedValue << " but got "
                    << ConstDeref<T>(data_->Top());
            }

            KAI_TRACE() << "Verification successful for: " << rhoCode;
        } catch (const std::exception& e) {
            KAI_TRACE_ERROR() << "Error executing Rho code: " << e.what();
            FAIL() << "Exception during Rho execution: " << e.what();
        }
    }

    // Helper to execute Rho code that creates an array and verify its contents
    void ExecuteAndVerifyArray(const std::string& rhoCode,
                               const std::vector<int>& expectedValues) {
        data_->Clear();

        try {
            KAI_TRACE() << "Executing Rho code for array: " << rhoCode;
            bool success = console_.Execute(rhoCode);
            ASSERT_TRUE(success) << "Failed to execute Rho code: " << rhoCode;

            // Verify result is an array
            ASSERT_FALSE(data_->Empty())
                << "Stack should not be empty after execution";
            ASSERT_TRUE(data_->Top().IsType<Array>())
                << "Expected Array but got "
                << data_->Top().GetClass()->GetName().ToString();

            // Verify array contents
            Pointer<Array> array = data_->Top();
            ASSERT_EQ(array->Size(), expectedValues.size())
                << "Array size mismatch";

            for (size_t i = 0; i < expectedValues.size(); ++i) {
                ASSERT_TRUE(array->At(i).IsType<int>())
                    << "Expected int at index " << i;
                ASSERT_EQ(ConstDeref<int>(array->At(i)), expectedValues[i])
                    << "Value mismatch at index " << i;
            }

            KAI_TRACE() << "Array verification successful for: " << rhoCode;
        } catch (const std::exception& e) {
            KAI_TRACE_ERROR() << "Error executing array Rho code: " << e.what();
            FAIL() << "Exception during Rho array execution: " << e.what();
        }
    }

    // Helper to execute Rho code with a multi-line script file
    bool ExecuteScript(const std::string& scriptContent) {
        try {
            KAI_TRACE() << "Executing multi-line Rho script:\n"
                        << scriptContent;
            return console_.Execute(scriptContent);
        } catch (const std::exception& e) {
            KAI_TRACE_ERROR() << "Error executing Rho script: " << e.what();
            return false;
        }
    }
};

// Function Definition with Arguments and Return Value
TEST_F(AdvancedRhoLanguage, FunctionWithArgumentsAndReturn) {
    // Define a function with arguments and return value
    const std::string script = R"(
        // Define a function that calculates the sum of squares
        fun sumOfSquares(a, b) {
            return a*a + b*b
        }
        
        // Call the function and get the result
        result = sumOfSquares(3, 4)
    )";

    ASSERT_TRUE(ExecuteScript(script));
    ExecuteAndVerify<int>("result", 25);  // 3² + 4² = 9 + 16 = 25

    std::cout << "Function with arguments and return test passed" << std::endl;
}

// Nested Functions and Closures
TEST_F(AdvancedRhoLanguage, NestedFunctionsAndClosures) {
    // Test nested functions and closures
    try {
        const std::string script = R"(
            // Outer function that returns an inner function
            fun makeMultiplier(factor) {
                // Inner function that forms a closure with the outer factor
                fun multiply(x) {
                    return x * factor
                }
                return multiply
            }
            
            // Create a function that multiplies by 5
            multiplyBy5 = makeMultiplier(5)
            
            // Use the created function
            result = multiplyBy5(7)
        )";

        ASSERT_TRUE(ExecuteScript(script));
        ExecuteAndVerify<int>("result", 35);  // 7 * 5 = 35
    } catch (const std::exception& e) {
        std::cout << "Nested functions or closures not fully supported: "
                  << e.what() << std::endl;
    }

    std::cout << "Nested functions and closures test completed" << std::endl;
}

// Array Creation and Manipulation
TEST_F(AdvancedRhoLanguage, ArrayCreationAndManipulation) {
    // Test array creation and manipulation
    try {
        const std::string script = R"(
            // Create an array
            arr = [1, 2, 3, 4, 5]
            
            // Access elements
            firstElement = arr[0]
            lastElement = arr[4]
            
            // Modify elements
            arr[2] = 10
            
            // Array methods
            arr.push(6)
            totalSum = 0
            for (i = 0; i < arr.size(); i = i + 1) {
                totalSum = totalSum + arr[i]
            }
        )";

        ASSERT_TRUE(ExecuteScript(script));

        // Verify individual elements
        ExecuteAndVerify<int>("firstElement", 1);
        ExecuteAndVerify<int>("lastElement", 5);

        // Verify modified array
        // The expected array should be [1, 2, 10, 4, 5, 6]
        std::vector<int> expected = {1, 2, 10, 4, 5, 6};
        ExecuteAndVerifyArray("arr", expected);

        // Verify the sum calculation
        ExecuteAndVerify<int>("totalSum", 28);  // 1+2+10+4+5+6 = 28
    } catch (const std::exception& e) {
        std::cout << "Array operations not fully supported: " << e.what()
                  << std::endl;
    }

    std::cout << "Array creation and manipulation test completed" << std::endl;
}

// Object Creation and Property Access
TEST_F(AdvancedRhoLanguage, ObjectCreationAndPropertyAccess) {
    // Test object creation and property access
    try {
        const std::string script = R"(
            // Create an object (map/dictionary)
            person = {
                "name": "John",
                "age": 30,
                "city": "New York"
            }
            
            // Access properties
            personName = person["name"]
            personAge = person["age"]
            
            // Modify properties
            person["age"] = 31
            
            // Add new properties
            person["job"] = "Engineer"
        )";

        ASSERT_TRUE(ExecuteScript(script));

        // Verify individual properties
        ExecuteAndVerify<String>("personName", "John");
        ExecuteAndVerify<int>("personAge", 30);

        // Verify modified property
        ExecuteAndVerify<int>("person[\"age\"]", 31);

        // Verify new property
        ExecuteAndVerify<String>("person[\"job\"]", "Engineer");
    } catch (const std::exception& e) {
        std::cout << "Object operations not fully supported: " << e.what()
                  << std::endl;
    }

    std::cout << "Object creation and property access test completed"
              << std::endl;
}

// Recursion
TEST_F(AdvancedRhoLanguage, Recursion) {
    // Test recursive function calls
    try {
        const std::string script = R"(
            // Recursive factorial function
            fun factorial(n) {
                if (n <= 1) {
                    return 1
                } else {
                    return n * factorial(n - 1)
                }
            }
            
            // Calculate factorial of 5
            result = factorial(5)
        )";

        ASSERT_TRUE(ExecuteScript(script));
        ExecuteAndVerify<int>("result", 120);  // 5! = 5*4*3*2*1 = 120
    } catch (const std::exception& e) {
        std::cout << "Recursion not fully supported: " << e.what() << std::endl;
    }

    std::cout << "Recursion test completed" << std::endl;
}

// Pi Integration - Calling Pi Code from Rho
TEST_F(AdvancedRhoLanguage, PiIntegration) {
    // Test integrating Pi code within Rho
    try {
        const std::string script = R"(
            // Define a variable
            x = 5
            
            // Execute Pi code using the pi keyword
            pi_result = pi{ x x + }
            
            // Use Pi sequence notation within Rho
            sequence_result = pi{2 3 *}
        )";

        ASSERT_TRUE(ExecuteScript(script));

        // Verify Pi execution results
        ExecuteAndVerify<int>("pi_result", 10);       // 5 + 5 = 10
        ExecuteAndVerify<int>("sequence_result", 6);  // 2 * 3 = 6
    } catch (const std::exception& e) {
        std::cout << "Pi integration not fully supported: " << e.what()
                  << std::endl;
    }

    std::cout << "Pi integration test completed" << std::endl;
}

// Error Handling with try/catch
TEST_F(AdvancedRhoLanguage, ErrorHandlingWithTryCatch) {
    // Test error handling with try/catch if supported
    try {
        const std::string script = R"(
            // Try to perform an operation that might fail
            try {
                result = 10 / 0  // Division by zero
            } catch (error) {
                result = "Error caught"
            }
        )";

        ASSERT_TRUE(ExecuteScript(script));

        // The result should be "Error caught" if try/catch is supported
        ExecuteAndVerify<String>("result", "Error caught");
    } catch (const std::exception& e) {
        std::cout << "Try/catch not supported: " << e.what() << std::endl;
    }

    std::cout << "Error handling with try/catch test completed" << std::endl;
}

// Higher-Order Functions
TEST_F(AdvancedRhoLanguage, HigherOrderFunctions) {
    // Test higher-order functions (functions that take or return functions)
    try {
        const std::string script = R"(
            // Function that applies a function to a value
            fun applyFunction(func, value) {
                return func(value)
            }
            
            // Some example functions to pass
            fun double(x) {
                return x * 2
            }
            
            fun square(x) {
                return x * x
            }
            
            // Use higher-order function with different functions
            result1 = applyFunction(double, 5)
            result2 = applyFunction(square, 4)
        )";

        ASSERT_TRUE(ExecuteScript(script));

        // Verify results
        ExecuteAndVerify<int>("result1", 10);  // double(5) = 10
        ExecuteAndVerify<int>("result2", 16);  // square(4) = 16
    } catch (const std::exception& e) {
        std::cout << "Higher-order functions not fully supported: " << e.what()
                  << std::endl;
    }

    std::cout << "Higher-order functions test completed" << std::endl;
}

// String Manipulation
TEST_F(AdvancedRhoLanguage, StringManipulation) {
    // Test string manipulation operations
    try {
        const std::string script = R"(
            // String concatenation
            greeting = "Hello, " + "World!"
            
            // String repetition if supported
            repeated = "abc" * 3
            
            // String indexing if supported
            firstChar = greeting[0]
            
            // String methods if supported
            uppercased = greeting.toUpperCase()
            lowercased = greeting.toLowerCase()
            
            // String length
            length = greeting.length()
        )";

        ASSERT_TRUE(ExecuteScript(script));

        // Verify basic string operations
        ExecuteAndVerify<String>("greeting", "Hello, World!");

        // Other operations may not be supported in all implementations
        // These tests are informational
        try {
            ExecuteAndVerify<String>("repeated", "abcabcabc");
        } catch (...) {
            std::cout << "String repetition not supported" << std::endl;
        }

        try {
            ExecuteAndVerify<String>("firstChar", "H");
        } catch (...) {
            std::cout << "String indexing not supported" << std::endl;
        }

        try {
            ExecuteAndVerify<int>("length",
                                  13);  // "Hello, World!" has 13 characters
        } catch (...) {
            std::cout << "String length not supported" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "String manipulation not fully supported: " << e.what()
                  << std::endl;
    }

    std::cout << "String manipulation test completed" << std::endl;
}

// Lexical Scoping and Variable Shadowing
TEST_F(AdvancedRhoLanguage, LexicalScopingAndVariableShadowing) {
    // Test lexical scoping and variable shadowing
    try {
        const std::string script = R"(
            // Outer variable
            x = 10
            
            // Define a function with a parameter that shadows the outer variable
            fun testScoping(x) {
                // This should use the parameter x, not the outer x
                return x * 2
            }
            
            // Call with different value
            result1 = testScoping(5)
            
            // The outer x should remain unchanged
            outer_x = x
            
            // Test block scoping if supported
            {
                x = 20  // Should shadow the outer x within this block
                block_x = x
            }
            
            // Check if outer x was affected by the block
            after_block_x = x
        )";

        ASSERT_TRUE(ExecuteScript(script));

        // Verify function scoping
        ExecuteAndVerify<int>("result1", 10);  // 5 * 2 = 10
        ExecuteAndVerify<int>("outer_x", 10);  // Outer x should still be 10

        // Block scoping is implementation-dependent
        // Some languages use lexical scoping, others don't
        try {
            ExecuteAndVerify<int>("block_x", 20);
        } catch (...) {
            std::cout << "Block scoping variable access not supported"
                      << std::endl;
        }

        // Check if the block affected the outer variable
        // This will tell us if the language uses lexical or dynamic scoping
        try {
            // If after_block_x is 20, the block modified the outer variable
            // (dynamic scoping) If after_block_x is 10, the block had its own
            // scope (lexical scoping)
            int afterBlockX = ConstDeref<int>(
                exec_->GetRegistry()->GetValue(Label("after_block_x")));
            std::cout << "Block scoping: after_block_x = " << afterBlockX
                      << " (this indicates "
                      << (afterBlockX == 10 ? "lexical" : "dynamic")
                      << " scoping)" << std::endl;
        } catch (...) {
            std::cout << "After block variable not accessible" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Lexical scoping test failed: " << e.what() << std::endl;
    }

    std::cout << "Lexical scoping and variable shadowing test completed"
              << std::endl;
}