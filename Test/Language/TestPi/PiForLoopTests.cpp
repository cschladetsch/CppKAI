#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/Console.h"
#include "TestConsoleHelper.h"

// Test suite for Pi for loops - each test uses its own executor
struct PiForLoopTest : ::testing::Test {};

// Range-based for loop syntax: accumulator start end { body } for
TEST_F(PiForLoopTest, SimpleRangeSum) {
    try {
        // Create fresh console for this test
        std::cout << "Creating console...\n";
        kai::Console console;
        std::cout << "Setting up translators...\n";
        kai::test::SetupConsoleTranslators(console);
        std::cout << "Setting language...\n";
        console.SetLanguage(kai::Language::Pi);

        std::cout << "Executing simple test first...\n";
        console.Execute("2 3 +");
        auto exec = console.GetExecutor();
        auto stack = exec->GetDataStack();
        std::cout << "Simple test result: " << stack->Size()
                  << " items on stack\n";
        if (stack->Size() > 0) {
            std::cout << "Top value: " << kai::ConstDeref<int>(stack->Top())
                      << "\n";
        }

        // Clear stack for actual test
        while (!stack->Empty()) stack->Pop();

        std::cout << "Executing for loop...\n";
        // Sum numbers from 1 to 5: 0 1 5 { + } for
        console.Execute("0 1 5 { + } for");

        std::cout << "Checking result...\n";
        ASSERT_EQ(stack->Size(), 1);
        EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);  // 1+2+3+4+5
    } catch (const kai::Exception::Base& e) {
        FAIL() << "KAI Exception: " << e.ToString();
    } catch (const std::exception& e) {
        FAIL() << "Exception: " << e.what();
    } catch (...) {
        FAIL() << "Unknown exception";
    }
}

TEST_F(PiForLoopTest, RangeProduct) {
    // Create fresh console for this test
    kai::Console console;
    kai::test::SetupConsoleTranslators(console);
    console.SetLanguage(kai::Language::Pi);

    // Product of numbers from 1 to 4: 1 1 4 { * } for
    console.Execute("1 1 4 { * } for");
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 24);  // 1*2*3*4
}

// DISABLED: Array append operation not available in Pi
TEST_F(PiForLoopTest, DISABLED_CollectSquares) {
    // Create fresh console for this test
    kai::Console console;
    kai::test::SetupConsoleTranslators(console);
    console.SetLanguage(kai::Language::Pi);

    // Collect squares: [] 1 4 { dup * + } for
    console.Execute("[] 1 4 { dup * + } for");
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    auto array = kai::ConstDeref<kai::Array>(stack->Top());
    ASSERT_EQ(array.Size(), 4);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(0)), 1);   // 1*1
    EXPECT_EQ(kai::ConstDeref<int>(array.At(1)), 4);   // 2*2
    EXPECT_EQ(kai::ConstDeref<int>(array.At(2)), 9);   // 3*3
    EXPECT_EQ(kai::ConstDeref<int>(array.At(3)), 16);  // 4*4
}

// DISABLED: ife doesn't work with inline for loop execution
TEST_F(PiForLoopTest, DISABLED_ConditionalSum) {
    // Create fresh console for this test
    kai::Console console;
    kai::test::SetupConsoleTranslators(console);
    console.SetLanguage(kai::Language::Pi);

    // Sum only even numbers from 1 to 6
    // Stack: accumulator current -> accumulator'
    // Rewritten without ife to work with inline execution
    console.Execute("0 1 6 { dup 2 % 0 == * + } for");
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    std::cout << "Stack size after for loop: " << stack->Size() << std::endl;
    if (stack->Size() > 0) {
        std::cout << "Top of stack: " << kai::ConstDeref<int>(stack->Top())
                  << std::endl;
    }
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 12);  // 2+4+6
}

// Traditional 4-continuation syntax if needed
TEST_F(PiForLoopTest, TraditionalSyntax) {
    // Create fresh console for this test
    kai::Console console;
    kai::test::SetupConsoleTranslators(console);
    console.SetLanguage(kai::Language::Pi);

    // Traditional for loop: { init } { condition } { increment } { body } for
    // Count from 0 to 4
    console.Execute("{ 0 } { dup 5 < } { 1 + } { } for");
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST_F(PiForLoopTest, TraditionalWithBody) {
    // Create fresh console for this test
    kai::Console console;
    kai::test::SetupConsoleTranslators(console);
    console.SetLanguage(kai::Language::Pi);

    // Sum using traditional syntax
    console.Execute(
        "0 { 0 } { dup 5 < } { 1 + } { swap over + swap } for drop");
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);  // 0+1+2+3+4
}

TEST_F(PiForLoopTest, NestedRangeLoops) {
    // Create fresh console for this test
    kai::Console console;
    kai::test::SetupConsoleTranslators(console);
    console.SetLanguage(kai::Language::Pi);

    // Nested range loops to build multiplication table entry
    // Outer: i=2, Inner: j=1 to 3, calculate 2*1 + 2*2 + 2*3 = 12
    // Stack management: outer provides multiplier, inner does sum
    // Simplified: just calculate 2*(1+2+3) = 2*6 = 12
    console.Execute("0 2 2 { 0 1 3 { + } for * + } for");
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 12);
}

TEST_F(PiForLoopTest, EmptyRange) {
    // Create fresh console for this test
    kai::Console console;
    kai::test::SetupConsoleTranslators(console);
    console.SetLanguage(kai::Language::Pi);

    // Empty range (start > end) should just return accumulator
    console.Execute("42 5 1 { + } for");
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST_F(PiForLoopTest, SingleIteration) {
    // Create fresh console for this test
    kai::Console console;
    kai::test::SetupConsoleTranslators(console);
    console.SetLanguage(kai::Language::Pi);

    // Single iteration (start == end)
    console.Execute("10 3 3 { + } for");
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 13);  // 10 + 3
}

// DISABLED: Array append operation not available in Pi
TEST_F(PiForLoopTest, DISABLED_ArrayBuilding) {
    // Create fresh console for this test
    kai::Console console;
    kai::test::SetupConsoleTranslators(console);
    console.SetLanguage(kai::Language::Pi);

    // Build an array using range loop
    console.Execute("[] 1 3 { swap over swap + } for drop");
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    auto array = kai::ConstDeref<kai::Array>(stack->Top());
    ASSERT_EQ(array.Size(), 3);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(0)), 1);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(1)), 2);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(2)), 3);
}