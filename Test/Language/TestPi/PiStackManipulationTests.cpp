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
            // Clear stacks before executing to ensure clean state
            exec_->ClearStacks();

            // Use the console from TestLangCommon which has translators set up
            console_.SetLanguage(Language::Pi);
            console_.Execute(script);

            auto executor = console_.GetExecutor();
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
                        ADD_FAILURE()
                            << "Type mismatch at stack position " << index
                            << " (stack index " << stackIndex << ")";
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

    // dup2 - duplicate top two elements (2dup in Forth)
    AssertStackResult("1 2 dup2", std::make_tuple(1, 2, 1, 2));
    AssertStackResult("1 2 3 4 dup2", std::make_tuple(1, 2, 3, 4, 3, 4));

    // drop2 - drop top two elements (2drop in Forth)
    AssertStackResult("1 2 3 4 drop2", std::make_tuple(1, 2));
    AssertStackResult("1 2 drop2", std::make_tuple());
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
    AssertStackResult("1 2 3 4 2 roll", std::make_tuple(1, 3, 4, 2));
    AssertStackResult("1 2 3 4 3 roll", std::make_tuple(2, 3, 4, 1));
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
    // Stack trace: 1 2 3 -> rot -> 2 3 1 -> swap -> 2 1 3
    AssertStackResult("1 2 3 rot swap", std::make_tuple(2, 1, 3));

    // More complex sequences
    // Stack trace: 1 2 3 4 -> swap -> 1 2 4 3 -> over -> 1 2 4 3 4 -> rot -> 1
    // 2 3 4 4
    AssertStackResult("1 2 3 4 swap over rot", std::make_tuple(1, 2, 3, 4, 4));
    // Stack trace: 1 2 3 -> dup -> 1 2 3 3 -> rot -> 1 3 3 2 -> swap -> 1 3 2 3
    // -> over -> 1 3 2 3 2
    AssertStackResult("1 2 3 dup rot swap over",
                      std::make_tuple(1, 3, 2, 3, 2));
    // Stack trace: 1 2 3 4 -> 1 pick (gets 3) -> 1 2 3 4 3 -> 2 pick (gets 3)
    // -> 1 2 3 4 3 3 -> swap -> 1 2 3 4 3 3
    AssertStackResult("1 2 3 4 1 pick 2 pick swap",
                      std::make_tuple(1, 2, 3, 4, 3, 3));
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
    // Store and retrieve (using # for store, unquoted name for retrieve)
    AssertStackResult("5 'x # x", std::make_tuple(5));

    // Multiple variables
    AssertStackResult("5 'x # 10 'y # x y", std::make_tuple(5, 10));

    // Variable with stack manipulation
    AssertStackResult("5 'x # 10 20 x swap", std::make_tuple(10, 5, 20));
    AssertStackResult("5 'x # 10 20 x rot", std::make_tuple(20, 5, 10));

    // Update variable
    // TODO: +! operation not implemented in Pi language yet
    // AssertStackResult("5 'x' ! 10 'x' +! 'x' @", std::make_tuple(15));
}

// Test conditional execution with stack operations
// TODO: Code blocks { ... } are not properly translated to continuations in Pi
// TEST_F(PiStackTests, ConditionalStackOperations) {
//     // If-then with stack operations
//     AssertStackResult("1 2 true { swap } if", std::make_tuple(2, 1));
//     AssertStackResult("1 2 false { swap } if", std::make_tuple(1, 2));
//
//     // If-then-else with stack operations
//     AssertStackResult("1 2 true { swap } { dup } ife",
//                       std::make_tuple(2, 1));
//     AssertStackResult("1 2 false { swap } { dup } ife",
//                       std::make_tuple(1, 2, 2));
//
//     // Complex conditionals
//     AssertStackResult(
//         "1 2 3 "
//         "1 2 < "
//         "{ rot swap } "
//         "{ swap rot } "
//         "ife",
//         std::make_tuple(3, 1, 2));
//
//     AssertStackResult(
//         "1 2 3 "
//         "1 2 > "
//         "{ rot swap } "
//         "{ swap rot } "
//         "ife",
//         std::make_tuple(2, 3, 1));
// }

// Test fixture for Pi computational tests
struct PiComputationalTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
    }
};

// Test computational stack patterns
TEST_F(PiComputationalTests, SquareNumber) {
    // Square a number: 5 dup * = 25
    console_.Execute("5 dup *");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 25);
}

TEST_F(PiComputationalTests, AverageOfTwo) {
    // Calculate average of two numbers: (7 + 9) / 2 = 8
    console_.Execute("7 9 + 2 /");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);
}

TEST_F(PiComputationalTests, ComplexArithmetic) {
    auto exec = console_.GetExecutor();

    // Complex calculation: (3 + 4) * 2 = 14
    console_.Execute("3 4 + 2 *");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 14);
}

// Test more complex computational patterns
// TODO: begin/until and +! operations not implemented in Pi language yet
// TEST_F(PiStackTests, AdvancedComputations) {
//     // nth triangular number
//     AssertStackResult(
//         "5 'n' ! "
//         "0 'sum' ! "
//         "1 'i' ! "
//         "begin "

// Test fixture for standalone Pi stack tests
struct PiStackStandaloneTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
    }
};

// Test basic stack operation patterns
TEST_F(PiStackStandaloneTests, DupAndAdd) {
    console_.Execute("5 dup +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);
}

TEST_F(PiStackStandaloneTests, OverAndMultiply) {
    console_.Execute(
        "3 4 over * +");  // 3 4 over -> 3 4 3 -> * -> 3 12 -> + -> 15
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);
}

TEST_F(PiStackStandaloneTests, RotateAndSubtract) {
    console_.Execute("10 5 2 rot -");  // 10 5 2 rot -> 5 2 10 -> - -> 5 -8
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 2);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(1)), 5);   // bottom
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), -8);  // top
}
//         "'i' @ 'sum' +! "
//         "'i' @ 1 + 'i' ! "
//         "'i' @ 'n' @ > "
//         "until "
//         "'sum' @",
//         std::make_tuple(15));
//
//     // Factorial
//     AssertStackResult(
//         "5 'n' ! "
//         "1 'result' ! "
//         "1 'i' ! "
//         "begin "
//         "'result' @ 'i' @ * 'result' ! "
//         "'i' @ 1 + 'i' ! "
//         "'i' @ 'n' @ > "
//         "until "
//         "'result' @",
//         std::make_tuple(120));
// }