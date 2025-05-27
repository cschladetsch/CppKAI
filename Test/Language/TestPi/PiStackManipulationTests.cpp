#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes/Stack.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test fixture for Pi language stack manipulation
struct PiStackTests : TestLangCommon {
    // Helper method to execute a Pi script and verify the stack values
    template <typename... Ts>
    void AssertStackResult(const char *script, std::tuple<Ts...> expected,
                           bool verbose = false) {
        try {
            Console console;
            console.SetLanguage(Language::Pi);
            console.Execute(script);

            auto executor = console.GetExecutor();
            auto dataStack = executor->GetDataStack();
            
            if (!dataStack.Valid() || !dataStack.Exists()) {
                FAIL() << "Invalid data stack after execution";
                return;
            }

            VerifyStack(*dataStack, expected, std::index_sequence_for<Ts...>{});
        } catch (const Exception::Base &e) {
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception &e) {
            FAIL() << "std::exception: " << e.what();
        } catch (...) {
            FAIL() << "Unknown exception";
        }
    }

    // Helper to verify stack values
    template <typename... Ts, size_t... Is>
    void VerifyStack(Stack &stack, std::tuple<Ts...> expected,
                     std::index_sequence<Is...>) {
        ASSERT_EQ(stack.Size(), sizeof...(Ts)) << "Stack size mismatch";

        // Only proceed with element checks if the sizes match
        if (stack.Size() == sizeof...(Ts)) {
            // Verify stack values from bottom to top
            auto checkElement = [&](size_t index, auto expectedValue) {
                // IMPORTANT: Stack::At() uses reverse indexing!
                // At(0) returns the TOP of the stack, not the bottom
                // At(Size()-1) returns the BOTTOM of the stack
                // So to check from bottom to top, we need to reverse the index
                if (index < stack.Size()) {
                    using ExpectedType = std::decay_t<decltype(expectedValue)>;
                    // Reverse the index to get bottom-to-top ordering
                    size_t stackIndex = stack.Size() - 1 - index;
                    Object obj = stack.At(stackIndex);

                    if (!obj.IsType<ExpectedType>()) {
                        ADD_FAILURE() << "Type mismatch at stack position "
                                      << index << " (stack index " << stackIndex << ")";
                    } else {
                        ExpectedType actual = ConstDeref<ExpectedType>(obj);
                        EXPECT_EQ(actual, expectedValue)
                            << "Value mismatch at stack position " << index 
                            << " (stack index " << stackIndex << ")";
                    }
                }
            };

            // Check elements from bottom to top
            (checkElement(Is, std::get<Is>(expected)), ...);
        }
    }

    // Simplified helper for single result
    template <class T>
    void AssertResult(const char *script, T expected, bool verbose = false) {
        AssertStackResult(script, std::make_tuple(expected), verbose);
    }
};

// Test basic stack operations
TEST_F(PiStackTests, BasicStackOperations) {
    // dup - duplicate top element
    AssertStackResult("5", std::make_tuple(5));
    AssertStackResult("5 dup", std::make_tuple(5, 5));

    // drop - remove top element
    AssertStackResult("5 6 7 drop", std::make_tuple(5, 6));

    // swap - swap top two elements
    AssertStackResult("5 6 swap", std::make_tuple(6, 5));

    // over - copy second element to top
    AssertStackResult("5 6 over", std::make_tuple(5, 6, 5));

    // rot - rotate top three elements
    AssertStackResult("1 2 3 rot", std::make_tuple(2, 3, 1));

    // -rot - reverse rotate top three elements
    // TODO: -rot operation not implemented in Pi language yet
    // AssertStackResult("1 2 3 -rot", std::make_tuple(3, 1, 2));
}

// Test advanced stack operations
TEST_F(PiStackTests, AdvancedStackOperations) {
    // 2dup - duplicate top two elements
    // TODO: 2dup operation not implemented in Pi language yet
    // AssertStackResult("1 2 2dup", std::make_tuple(1, 2, 1, 2));

    // 2drop - drop top two elements  
    // TODO: 2drop operation not implemented in Pi language yet
    // AssertStackResult("1 2 3 4 2drop", std::make_tuple(1, 2));

    // 2swap - swap top two pairs
    // TODO: 2swap operation not implemented in Pi language yet
    // AssertStackResult("1 2 3 4 2swap", std::make_tuple(3, 4, 1, 2));

    // 2over - copy second pair to top
    // TODO: 2over operation not implemented in Pi language yet
    // AssertStackResult("1 2 3 4 2over", std::make_tuple(1, 2, 3, 4, 1, 2));

    // tuck - insert top element before second element
    // TODO: tuck operation not implemented in Pi language yet
    // AssertStackResult("1 2 tuck", std::make_tuple(1, 2, 1));

    // nip - remove second element
    // TODO: nip operation not implemented in Pi language yet
    // AssertStackResult("1 2 nip", std::make_tuple(2));
}

// Test pick operations
TEST_F(PiStackTests, PickOperations) {
    // pick - copy nth element to top (0-based)
    AssertStackResult("1 2 3 4 0 pick", std::make_tuple(1, 2, 3, 4, 4));
    AssertStackResult("1 2 3 4 1 pick", std::make_tuple(1, 2, 3, 4, 3));
    AssertStackResult("1 2 3 4 2 pick", std::make_tuple(1, 2, 3, 4, 2));
    AssertStackResult("1 2 3 4 3 pick", std::make_tuple(1, 2, 3, 4, 1));

    // roll - move nth element to top
    AssertStackResult("1 2 3 4 0 roll", std::make_tuple(1, 2, 3, 4));
    AssertStackResult("1 2 3 4 1 roll", std::make_tuple(1, 2, 4, 3));
    AssertStackResult("1 2 3 4 2 roll", std::make_tuple(1, 4, 2, 3));
    AssertStackResult("1 2 3 4 3 roll", std::make_tuple(4, 1, 2, 3));
}

// Test stack depth operations
TEST_F(PiStackTests, StackDepthOperations) {
    // depth - push stack depth
    AssertStackResult("depth", std::make_tuple(0));
    AssertStackResult("1 depth", std::make_tuple(1, 1));
    AssertStackResult("1 2 3 depth", std::make_tuple(1, 2, 3, 3));

    // clear - clear the stack
    AssertStackResult("1 2 3 clear", std::make_tuple());
    AssertStackResult("1 2 3 clear 4", std::make_tuple(4));
}

// Test composite stack operations
TEST_F(PiStackTests, CompositeStackOperations) {
    // Sequence of operations
    AssertStackResult("1 2 3 swap drop", std::make_tuple(1, 3));
    AssertStackResult("1 2 3 rot swap", std::make_tuple(1, 3, 2));
    AssertStackResult("1 2 dup 2dup", std::make_tuple(1, 2, 2, 2, 2));
    AssertStackResult("1 2 3 4 2swap 2drop", std::make_tuple(3, 4));

    // More complex sequences
    AssertStackResult("1 2 3 4 swap over rot", std::make_tuple(1, 3, 4, 2));
    AssertStackResult("1 2 3 dup rot swap over",
                      std::make_tuple(1, 3, 2, 3, 3));
    AssertStackResult("1 2 3 4 1 pick 2 pick swap",
                      std::make_tuple(1, 2, 3, 4, 2, 3));
}

// Test operations with different types
TEST_F(PiStackTests, MixedTypeOperations) {
    // Integer and float
    AssertStackResult("1 2.5 swap", std::make_tuple(2.5f, 1));
    AssertStackResult("1 2.5 over", std::make_tuple(1, 2.5f, 1));

    // Boolean
    AssertStackResult("1 true swap", std::make_tuple(true, 1));
    AssertStackResult("1 true over", std::make_tuple(1, true, 1));

    // String
    AssertStackResult("1 \"hello\" swap", std::make_tuple(String("hello"), 1));
    AssertStackResult("1 \"hello\" over",
                      std::make_tuple(1, String("hello"), 1));

    // Mixed types
    AssertStackResult("1 2.5 true \"hello\" rot",
                      std::make_tuple(1, true, String("hello"), 2.5f));
}

// Test variable interaction with stack
TEST_F(PiStackTests, VariableOperations) {
    // Store and retrieve
    AssertStackResult("5 'x' ! 'x' @", std::make_tuple(5));

    // Multiple variables
    AssertStackResult("5 'x' ! 10 'y' ! 'x' @ 'y' @", std::make_tuple(5, 10));

    // Variable with stack manipulation
    AssertStackResult("5 'x' ! 10 20 'x' @ swap", std::make_tuple(10, 5, 20));
    AssertStackResult("5 'x' ! 10 20 'x' @ rot", std::make_tuple(20, 5, 10));

    // Update variable
    AssertStackResult("5 'x' ! 10 'x' +! 'x' @", std::make_tuple(15));
}

// Test conditional execution with stack operations
TEST_F(PiStackTests, ConditionalStackOperations) {
    // If-then with stack operations
    AssertStackResult("1 2 true { swap } if", std::make_tuple(2, 1));
    AssertStackResult("1 2 false { swap } if", std::make_tuple(1, 2));

    // If-then-else with stack operations
    AssertStackResult("1 2 true { swap } { dup } ifelse",
                      std::make_tuple(2, 1));
    AssertStackResult("1 2 false { swap } { dup } ifelse",
                      std::make_tuple(1, 2, 2));

    // Complex conditionals
    AssertStackResult(
        "1 2 3 "
        "1 2 < "
        "{ rot swap } "
        "{ swap rot } "
        "ifelse",
        std::make_tuple(3, 1, 2));

    AssertStackResult(
        "1 2 3 "
        "1 2 > "
        "{ rot swap } "
        "{ swap rot } "
        "ifelse",
        std::make_tuple(2, 3, 1));
}

// Test computational stack patterns
TEST_F(PiStackTests, ComputationalPatterns) {
    // Sum of two numbers
    AssertStackResult("3 4 +", std::make_tuple(7));

    // Square a number
    AssertStackResult("5 dup *", std::make_tuple(25));

    // Calculate average of two numbers
    AssertStackResult("7 9 + 2 /", std::make_tuple(8));

    // Min of two numbers
    AssertStackResult("3 5 2dup < { drop } { nip } ifelse", std::make_tuple(3));
    AssertStackResult("7 4 2dup < { drop } { nip } ifelse", std::make_tuple(4));

    // Max of two numbers
    AssertStackResult("3 5 2dup > { drop } { nip } ifelse", std::make_tuple(5));
    AssertStackResult("7 4 2dup > { drop } { nip } ifelse", std::make_tuple(7));

    // Absolute value
    AssertStackResult("-3 dup 0 < { -1 * } if", std::make_tuple(3));
    AssertStackResult("5 dup 0 < { -1 * } if", std::make_tuple(5));
}

// Test more complex computational patterns
TEST_F(PiStackTests, AdvancedComputations) {
    // nth triangular number
    AssertStackResult(
        "5 'n' ! "
        "0 'sum' ! "
        "1 'i' ! "
        "begin "
        "'i' @ 'sum' +! "
        "'i' @ 1 + 'i' ! "
        "'i' @ 'n' @ > "
        "until "
        "'sum' @",
        std::make_tuple(15));

    // Factorial
    AssertStackResult(
        "5 'n' ! "
        "1 'result' ! "
        "1 'i' ! "
        "begin "
        "'result' @ 'i' @ * 'result' ! "
        "'i' @ 1 + 'i' ! "
        "'i' @ 'n' @ > "
        "until "
        "'result' @",
        std::make_tuple(120));
}