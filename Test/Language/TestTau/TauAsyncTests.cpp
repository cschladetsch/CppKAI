#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Test suite for Tau async/await functionality
TEST(TauAsync, BasicAsyncFunction) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);

    const char* code = R"(
        async int DelayedComputation(int value) {
            await Task.Delay(100);
            return value * 2;
        }
        
        int result = await DelayedComputation(21);
        result;
    )";

    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST(TauAsync, MultipleAsyncOperations) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);

    const char* code = R"(
        async int FetchValue(int id) {
            await Task.Delay(50);
            return id * 10;
        }
        
        async int SumValues() {
            int a = await FetchValue(1);
            int b = await FetchValue(2);
            int c = await FetchValue(3);
            return a + b + c;
        }
        
        await SumValues();
    )";

    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 60);  // 10 + 20 + 30
}

TEST(TauAsync, AsyncWithExceptionHandling) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);

    const char* code = R"(
        async int RiskyOperation(bool shouldFail) {
            await Task.Delay(10);
            if (shouldFail) {
                throw Exception("Operation failed");
            }
            return 100;
        }
        
        try {
            await RiskyOperation(true);
        } catch (Exception e) {
            "Caught: " + e.Message;
        }
    )";

    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()),
              "Caught: Operation failed");
}

TEST(TauAsync, ParallelExecution) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);

    const char* code = R"(
        async int SlowOperation(int value) {
            await Task.Delay(100);
            return value * value;
        }
        
        // Launch multiple tasks in parallel
        Task<int>[] tasks = {
            SlowOperation(1),
            SlowOperation(2),
            SlowOperation(3),
            SlowOperation(4)
        };
        
        // Wait for all to complete
        int[] results = await Task.WhenAll(tasks);
        
        // Sum the results
        int sum = 0;
        for (int r : results) {
            sum += r;
        }
        sum;
    )";

    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 30);  // 1 + 4 + 9 + 16
}

TEST(TauAsync, AsyncStreams) {
    kai::Console console;
    console.SetLanguage(kai::Language::Tau);

    const char* code = R"(
        async IAsyncEnumerable<int> GenerateNumbers() {
            for (int i = 1; i <= 5; i++) {
                await Task.Delay(10);
                yield return i * i;
            }
        }
        
        int sum = 0;
        await foreach (int value in GenerateNumbers()) {
            sum += value;
        }
        sum;
    )";

    console.Execute(code);
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 55);  // 1 + 4 + 9 + 16 + 25
}