#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test fixture for Pi language advanced operations
struct PiAdvancedTests : TestLangCommon {
    // Helper method to execute a Pi script and verify the value on top of stack
    template <class T>
    void AssertResult(const char *script, T expected, bool verbose = false) {
        if (verbose) {
            KAI_LOG_INFO(std::string("Testing script: ") + script);
        }

        try {
            Registry reg;
            Console console(reg);
            console.SetScope(reg.GetGlobalScope());

            auto result = console.Execute(script);

            if (result.Failed) {
                if (verbose) {
                    KAI_LOG_ERROR("Execution failed: " + result.Error);
                }
                FAIL() << "Error executing script: " << result.Error;
                return;
            }

            if (console.GetExecutor().GetStack().Size() == 0) {
                if (verbose) {
                    KAI_LOG_WARNING("Stack is empty, cannot verify result");
                }
                FAIL() << "Stack is empty, cannot verify result";
                return;
            }

            auto val = console.GetExecutor().GetStack().Top();
            if (val.GetType() != Type::Traits<T>::TypeNumber) {
                if (verbose) {
                    KAI_LOG_ERROR("Type mismatch. Expected: " +
                                  std::to_string(Type::Traits<T>::TypeNumber) +
                                  ", Got: " + std::to_string(val.GetType()));
                }
                FAIL() << "Type mismatch. Expected: "
                       << Type::Traits<T>::TypeNumber
                       << ", Got: " << val.GetType();
                return;
            }

            T actual = kai_cast<T>(val);
            if (verbose) {
                KAI_LOG_INFO("Result: " + std::to_string(actual));
            }
            ASSERT_EQ(expected, actual)
                << "Result doesn't match expected value";
        } catch (const Exception &e) {
            if (verbose) {
                KAI_LOG_ERROR("Exception: " + std::string(e.ToString()));
            }
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception &e) {
            if (verbose) {
                KAI_LOG_ERROR("std::exception: " + std::string(e.what()));
            }
            FAIL() << "std::exception: " << e.what();
        } catch (...) {
            if (verbose) {
                KAI_LOG_ERROR("Unknown exception");
            }
            FAIL() << "Unknown exception";
        }
    }
};

// Test stack manipulation operations
TEST_F(PiAdvancedTests, StackManipulation) {
    // Testing dup operation
    AssertResult<int>("5 dup +", 10);

    // Testing swap operation
    AssertResult<int>("3 4 swap -", 1);

    // Testing drop operation
    AssertResult<int>("3 4 5 drop +", 7);

    // Testing over operation
    AssertResult<int>("3 4 over +", 7);

    // Testing rot operation
    AssertResult<int>("1 2 3 rot + +", 6);

    // Testing pick operation
    AssertResult<int>("1 2 3 4 2 pick +", 6);

    // Combining multiple stack operations
    AssertResult<int>("1 2 3 4 swap over drop rot + +", 6);
}

// Test arithmetic operations
TEST_F(PiAdvancedTests, ArithmeticOperations) {
    // Basic arithmetic
    AssertResult<int>("2 3 +", 5);
    AssertResult<int>("5 3 -", 2);
    AssertResult<int>("4 5 *", 20);
    AssertResult<int>("20 4 /", 5);

    // Modulus
    AssertResult<int>("10 3 %", 1);

    // Combining operations
    AssertResult<int>("2 3 + 4 *", 20);
    AssertResult<int>("10 2 / 3 *", 15);
    AssertResult<int>("10 3 % 2 +", 3);

    // Complex expressions
    AssertResult<int>("2 3 + 4 5 + *", 45);
    AssertResult<int>("20 3 / 2 3 * +", 12);
    AssertResult<int>("10 3 % 2 + 3 4 + *", 21);
}

// Test comparison operations
TEST_F(PiAdvancedTests, ComparisonOperations) {
    // Equal
    AssertResult<bool>("5 5 ==", true);
    AssertResult<bool>("5 6 ==", false);

    // Not equal
    AssertResult<bool>("5 6 !=", true);
    AssertResult<bool>("5 5 !=", false);

    // Less than
    AssertResult<bool>("5 6 <", true);
    AssertResult<bool>("6 5 <", false);

    // Greater than
    AssertResult<bool>("6 5 >", true);
    AssertResult<bool>("5 6 >", false);

    // Less than or equal
    AssertResult<bool>("5 5 <=", true);
    AssertResult<bool>("5 6 <=", true);
    AssertResult<bool>("6 5 <=", false);

    // Greater than or equal
    AssertResult<bool>("5 5 >=", true);
    AssertResult<bool>("6 5 >=", true);
    AssertResult<bool>("5 6 >=", false);

    // Combining comparisons with stack operations
    AssertResult<bool>("5 6 < 7 8 < and", true);
    AssertResult<bool>("5 6 < 8 7 < and", false);
    AssertResult<bool>("5 6 < 8 7 < or", true);
    AssertResult<bool>("6 5 < 8 7 < or", false);
}

// Test bitwise operations
TEST_F(PiAdvancedTests, BitwiseOperations) {
    // And
    AssertResult<int>("5 3 and", 1);

    // Or
    AssertResult<int>("5 3 or", 7);

    // Xor
    AssertResult<int>("5 3 xor", 6);

    // Not (using 15 as a mask for 4 bits)
    AssertResult<int>("5 not 15 and", 10);

    // Shift left
    AssertResult<int>("1 3 shiftl", 8);

    // Shift right
    AssertResult<int>("8 2 shiftr", 2);

    // Combining bitwise operations
    AssertResult<int>("5 3 and 2 or", 3);
    AssertResult<int>("5 3 or 4 and", 4);
    AssertResult<int>("1 2 shiftl 2 2 shiftl or", 6);
}

// Test boolean logic operations
TEST_F(PiAdvancedTests, BooleanOperations) {
    // And
    AssertResult<bool>("true true and", true);
    AssertResult<bool>("true false and", false);
    AssertResult<bool>("false true and", false);
    AssertResult<bool>("false false and", false);

    // Or
    AssertResult<bool>("true true or", true);
    AssertResult<bool>("true false or", true);
    AssertResult<bool>("false true or", true);
    AssertResult<bool>("false false or", false);

    // Not
    AssertResult<bool>("true not", false);
    AssertResult<bool>("false not", true);

    // Combining boolean operations
    AssertResult<bool>("true false and true or", true);
    AssertResult<bool>("true true and false or", true);
    AssertResult<bool>("true not false not and", false);
    AssertResult<bool>("true not false not or", true);
}

// Test conditional operations
TEST_F(PiAdvancedTests, ConditionalOperations) {
    // If-then (true case)
    AssertResult<int>("10 true { 5 + } if", 15);

    // If-then (false case)
    AssertResult<int>("10 false { 5 + } if", 10);

    // If-then-else (true case)
    AssertResult<int>("10 true { 5 + } { 5 - } ife", 15);

    // If-then-else (false case)
    AssertResult<int>("10 false { 5 + } { 5 - } ife", 5);

    // Nested conditionals
    AssertResult<int>("10 true { 5 + true { 2 * } { 2 / } ife } { 5 - } ife",
                      30);
    AssertResult<int>("10 true { 5 + false { 2 * } { 2 / } ife } { 5 - } ife",
                      7);
    AssertResult<int>(
        "10 false { 5 + true { 2 * } { 2 / } ife } { 5 - true { 2 * } { 2 / "
        "} ife } ife",
        10);
    AssertResult<int>(
        "10 false { 5 + true { 2 * } { 2 / } ife } { 5 - false { 2 * } { 2 "
        "/ } ife } ife",
        2);
}

// Test string operations
TEST_F(PiAdvancedTests, StringOperations) {
    // String concatenation
    AssertResult<String>("\"Hello\" \" \" +", String("Hello "));
    AssertResult<String>("\"Hello\" \" \" + \"World\" +",
                         String("Hello World"));

    // String comparisons
    AssertResult<bool>("\"abc\" \"abc\" ==", true);
    AssertResult<bool>("\"abc\" \"def\" ==", false);
    AssertResult<bool>("\"abc\" \"def\" !=", true);
    AssertResult<bool>("\"abc\" \"def\" <", true);
    AssertResult<bool>("\"def\" \"abc\" >", true);

    // String operations with conditionals
    AssertResult<String>(
        "\"Hello\" \"World\" \"Hello\" \"Goodbye\" == { \" Happy\" } { \" "
        "Sad\" } ife +",
        String("Hello Sad"));
    AssertResult<String>(
        "\"Hello\" \"Hello\" \"Hello\" \"Goodbye\" == { \" Happy\" } { \" "
        "Sad\" } ife +",
        String("Hello Happy"));
}

// Test variable operations
TEST_F(PiAdvancedTests, VariableOperations) {
    // Store and recall
    AssertResult<int>("5 'x' ! 'x' @", 5);

    // Operate on stored variable
    AssertResult<int>("10 'x' ! 'x' @ 5 +", 15);

    // Update stored variable
    AssertResult<int>("10 'x' ! 5 'x' +! 'x' @", 15);

    // Multiple variables
    AssertResult<int>("5 'x' ! 10 'y' ! 'x' @ 'y' @ +", 15);

    // Variables with conditionals
    AssertResult<int>(
        "5 'x' ! 10 'y' ! 'x' @ 'y' @ < { 'x' @ 2 * } { 'y' @ 2 * } ife", 20);
    AssertResult<int>(
        "15 'x' ! 10 'y' ! 'x' @ 'y' @ < { 'x' @ 2 * } { 'y' @ 2 * } ife", 30);
}

// Test complex control flow with continuations
TEST_F(PiAdvancedTests, ControlFlowOperations) {
    // Using begin-while-repeat for a loop (sum from 1 to 5)
    AssertResult<int>(
        "0 'sum' ! 1 'i' ! begin 'i' @ 'sum' +! 'i' @ 1 + 'i' ! 'i' @ 5 <= "
        "while repeat 'sum' @",
        15);

    // Using begin-until (sum from 1 to 5)
    AssertResult<int>(
        "0 'sum' ! 1 'i' ! begin 'i' @ 'sum' +! 'i' @ 1 + 'i' ! 'i' @ 5 > "
        "until 'sum' @",
        15);

    // Fibonacci calculation (for n=7)
    AssertResult<int>(
        "0 'a' ! 1 'b' ! 7 'n' ! "
        "begin "
        "  'a' @ 'temp' ! "
        "  'b' @ 'a' ! "
        "  'temp' @ 'b' @ + 'b' ! "
        "  'n' @ 1 - 'n' ! "
        "  'n' @ 1 <= "
        "until "
        "'b' @",
        13);
}

// Test with mixed operations
TEST_F(PiAdvancedTests, MixedOperations) {
    // Combining arithmetic, comparison, and conditionals
    AssertResult<int>("5 3 + 7 < { 10 } { 20 } ife", 20);

    // Stack manipulation with arithmetic and conditionals
    AssertResult<int>("3 4 swap - 0 > { 5 + } { 5 - } ife", 6);

    // Variable usage with mixed operations
    AssertResult<int>(
        "5 'x' ! "
        "10 'y' ! "
        "'x' @ 'y' @ + 'sum' ! "
        "'x' @ 'y' @ * 'product' ! "
        "'sum' @ 'product' @ < { 'sum' @ } { 'product' @ } ife",
        50);

    // Complex calculation
    AssertResult<int>(
        "10 'max' ! "
        "0 'sum' ! "
        "1 'i' ! "
        "begin "
        "  'i' @ 'i' @ * 'square' ! "
        "  'square' @ 'sum' +! "
        "  'i' @ 1 + 'i' ! "
        "  'i' @ 'max' @ <= "
        "while "
        "repeat "
        "'sum' @",
        385);  // Sum of squares from 1 to 10
}

// Test with error handling (try-catch equivalent)
TEST_F(PiAdvancedTests, ErrorHandlingOperations) {
    // Simple error recovery (division by zero)
    // In Pi, this would use more advanced continuation handling
    // This is a simplified test case
    AssertResult<int>("5 0 == { 0 } { 5 0 == { 0 } { 10 } ife } ife", 10);
}