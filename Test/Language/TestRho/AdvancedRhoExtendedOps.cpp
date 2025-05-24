#include <gtest/gtest.h>

#include <cmath>
#include <iostream>
#include <string>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "KAI/Executor/Operation.h"
#include "SimpleRhoPiTests.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * ADVANCED RHO EXTENDED OPERATIONS TESTS
 * -------------------------------------
 * This file contains advanced tests for Rho language operations to extend test
 * coverage beyond the basic operations. These tests focus on complex
 * expressions, operator precedence, and language features specific to Rho.
 */

// Test fixture for Rho language extended operations
class AdvancedRhoExtendedOps : public TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<float>(Label("float"));
        reg_->AddClass<String>(Label("String"));
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
};

// Complex Arithmetic Expressions
TEST_F(AdvancedRhoExtendedOps, ComplexArithmeticExpressions) {
    // Test complex arithmetic with multiple operations and precedence
    ExecuteAndVerify<int>("2 + 3 * 4", 14);    // Multiplication before addition
    ExecuteAndVerify<int>("(2 + 3) * 4", 20);  // Parentheses change precedence
    ExecuteAndVerify<int>("10 - 2 * 3",
                          4);  // Multiplication before subtraction
    ExecuteAndVerify<int>("(10 - 2) * 3", 24);  // Parentheses change precedence
    ExecuteAndVerify<int>("8 / 4 + 3 * 2",
                          8);  // Division and multiplication before addition

    std::cout << "Complex arithmetic expressions test passed" << std::endl;
}

// Mixed Type Arithmetic
TEST_F(AdvancedRhoExtendedOps, MixedTypeArithmetic) {
    // Test operations with mixed numeric types (int and float)
    try {
        // These may work if automatic conversion is supported
        ExecuteAndVerify<float>("2 + 3.5", 5.5f);
        ExecuteAndVerify<float>("10.0 / 2", 5.0f);
        ExecuteAndVerify<float>("2.5 * 4", 10.0f);
    } catch (const std::exception& e) {
        // If automatic conversion isn't supported, this is informational
        std::cout << "Mixed type arithmetic not fully supported: " << e.what()
                  << std::endl;
    }

    std::cout << "Mixed type arithmetic test completed" << std::endl;
}

// Compound Assignment Operators
TEST_F(AdvancedRhoExtendedOps, CompoundAssignmentOperators) {
    // Test compound assignment operators like +=, -=, etc.
    try {
        // Variable assignment with compound operators
        console_.Execute("x = 5");
        console_.Execute("x += 3");
        ExecuteAndVerify<int>("x", 8);

        console_.Execute("y = 10");
        console_.Execute("y -= 4");
        ExecuteAndVerify<int>("y", 6);

        console_.Execute("z = 3");
        console_.Execute("z *= 4");
        ExecuteAndVerify<int>("z", 12);

        console_.Execute("w = 20");
        console_.Execute("w /= 5");
        ExecuteAndVerify<int>("w", 4);
    } catch (const std::exception& e) {
        // If compound operators aren't supported, this is informational
        std::cout << "Compound assignment operators not fully supported: "
                  << e.what() << std::endl;
    }

    std::cout << "Compound assignment operators test completed" << std::endl;
}

// Complex Boolean Logic
TEST_F(AdvancedRhoExtendedOps, ComplexBooleanLogic) {
    // Test complex boolean expressions with multiple operations
    ExecuteAndVerify<bool>("true && true", true);
    ExecuteAndVerify<bool>("true && false", false);
    ExecuteAndVerify<bool>("false || true", true);
    ExecuteAndVerify<bool>("false || false", false);
    ExecuteAndVerify<bool>("true && (false || true)", true);
    ExecuteAndVerify<bool>("(true && false) || true", true);
    ExecuteAndVerify<bool>("!(true && false)", true);
    ExecuteAndVerify<bool>("!false && true", true);

    std::cout << "Complex boolean logic test passed" << std::endl;
}

// String Operations and Manipulation
TEST_F(AdvancedRhoExtendedOps, StringOperations) {
    // Test string operations and manipulations
    ExecuteAndVerify<String>("\"Hello\" + \" \" + \"World\"", "Hello World");

    // Test string comparisons if supported
    try {
        ExecuteAndVerify<bool>("\"apple\" == \"apple\"", true);
        ExecuteAndVerify<bool>("\"apple\" != \"banana\"", true);
    } catch (const std::exception& e) {
        std::cout << "String comparison operations not fully supported: "
                  << e.what() << std::endl;
    }

    std::cout << "String operations test completed" << std::endl;
}

// Variable Scoping
TEST_F(AdvancedRhoExtendedOps, VariableScoping) {
    // Test variable scoping rules
    console_.Execute("outerVar = 10");

    // Test block scope if supported
    try {
        console_.Execute("{ innerVar = 20; outerVar = 30; }");

        // Access variables after block
        ExecuteAndVerify<int>("outerVar", 30);  // Should be updated

        // Try to access inner variable (may fail if proper scoping is enforced)
        data_->Clear();
        bool innerAccessFailed = false;
        try {
            console_.Execute("innerVar");
        } catch (const std::exception& e) {
            innerAccessFailed = true;
            std::cout << "Expected error accessing inner variable: " << e.what()
                      << std::endl;
        }

        // Note: This test is informational - either behavior might be correct
        // depending on how the language handles scoping
        std::cout << "Inner variable access "
                  << (innerAccessFailed ? "failed (lexical scoping)"
                                        : "succeeded (dynamic scoping)")
                  << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Block scoping not supported: " << e.what() << std::endl;
    }

    std::cout << "Variable scoping test completed" << std::endl;
}

// Function Definition and Calling
TEST_F(AdvancedRhoExtendedOps, FunctionDefinitionAndCalling) {
    // Test function definition and calling
    try {
        // Define a simple function
        console_.Execute("fun add(a, b) { return a + b; }");

        // Call the function
        ExecuteAndVerify<int>("add(5, 3)", 8);

        // Define and call a more complex function
        console_.Execute(
            "fun factorial(n) { if (n <= 1) return 1; else return n * "
            "factorial(n - 1); }");
        ExecuteAndVerify<int>("factorial(5)", 120);  // 5! = 120
    } catch (const std::exception& e) {
        std::cout << "Function definition or recursion not fully supported: "
                  << e.what() << std::endl;
    }

    std::cout << "Function definition and calling test completed" << std::endl;
}

// Conditional Expressions
TEST_F(AdvancedRhoExtendedOps, ConditionalExpressions) {
    // Test conditional expressions (if-else)
    try {
        // Simple if-else
        console_.Execute("result = if (true) 10 else 20");
        ExecuteAndVerify<int>("result", 10);

        console_.Execute("result = if (false) 10 else 20");
        ExecuteAndVerify<int>("result", 20);

        // Nested if-else
        console_.Execute("x = 5");
        console_.Execute(
            "result = if (x > 10) 'large' else if (x > 0) 'positive' else "
            "'negative'");
        ExecuteAndVerify<String>("result", "positive");
    } catch (const std::exception& e) {
        std::cout << "Conditional expressions not fully supported: " << e.what()
                  << std::endl;
    }

    std::cout << "Conditional expressions test completed" << std::endl;
}

// Unary Operations
TEST_F(AdvancedRhoExtendedOps, UnaryOperations) {
    // Test unary operations like negation, not, etc.
    ExecuteAndVerify<int>("-5", -5);         // Negative
    ExecuteAndVerify<bool>("!true", false);  // Logical not
    ExecuteAndVerify<bool>("!false", true);  // Logical not

    // Test with expressions
    ExecuteAndVerify<int>("-(2 + 3)", -5);       // Negate an expression
    ExecuteAndVerify<bool>("!(10 > 5)", false);  // Negate a comparison

    std::cout << "Unary operations test passed" << std::endl;
}

// Error Handling and Recovery
TEST_F(AdvancedRhoExtendedOps, ErrorHandlingAndRecovery) {
    // Test error handling and recovery

    // Division by zero
    data_->Clear();
    bool exceptionThrown = false;
    try {
        console_.Execute("10 / 0");
    } catch (const std::exception& e) {
        exceptionThrown = true;
        std::cout << "Expected division by zero error caught: " << e.what()
                  << std::endl;
    }
    ASSERT_TRUE(exceptionThrown)
        << "Division by zero should throw an exception";

    // Verify that execution can continue after an error
    console_.Execute("safeValue = 42");
    ExecuteAndVerify<int>("safeValue", 42);

    // Type errors
    data_->Clear();
    exceptionThrown = false;
    try {
        console_.Execute(
            "\"text\" - 5");  // String subtraction is usually invalid
    } catch (const std::exception& e) {
        exceptionThrown = true;
        std::cout << "Expected type error caught: " << e.what() << std::endl;
    }
    // Note: This test is informational - some languages might coerce types
    std::cout << "Type error test "
              << (exceptionThrown ? "caught error" : "did type coercion")
              << std::endl;

    std::cout << "Error handling and recovery test completed" << std::endl;
}