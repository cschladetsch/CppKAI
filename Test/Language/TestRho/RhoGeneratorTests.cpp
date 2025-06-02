#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Test suite for Rho generators and lazy evaluation
// Note: Generators are not yet implemented in Rho
TEST(RhoGenerator, SimpleGenerator) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        fun counter(start)
            current = start
            while true
                yield current
                current = current + 1
        
        gen = counter(1)
        a = next(gen)
        b = next(gen)
        c = next(gen)
        a + b + c
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);  // 1 + 2 + 3
}

TEST(RhoGenerator, GeneratorWithCondition) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        fun fibonacci()
            a = 0
            b = 1
            while true
                yield a
                temp = a + b
                a = b
                b = temp
        
        fib = fibonacci()
        # Get first 5 fibonacci numbers
        result = []
        for i = 0; i < 5; i = i + 1
            result.push(next(fib))
        result[4]
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);  // Fifth fibonacci number
}

TEST(RhoGenerator, GeneratorComprehension) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        squares = (x * x for x in range(1, 6))
        list(squares)
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    auto& array = kai::Deref<kai::Array>(stack->Top());
    EXPECT_EQ(array.Size(), 5);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(4)), 25);
}

TEST(RhoGenerator, LazyEvaluation) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        # Infinite generator that would crash if fully evaluated
        fun infinite_ones()
            while true
                yield 1
        
        # Take only first 3 elements
        gen = infinite_ones()
        sum = 0
        for i = 0; i < 3; i = i + 1
            sum = sum + next(gen)
        sum
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

TEST(RhoGenerator, GeneratorChaining) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        fun filter_gen(gen, predicate)
            for value in gen
                if predicate(value)
                    yield value
        
        numbers = range(1, 10)
        evens = filter_gen(numbers, lambda(x) x % 2 == 0)
        sum(evens)
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 20);  // 2+4+6+8
}