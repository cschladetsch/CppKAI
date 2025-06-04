#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test 1: Basic while loop - sum numbers from 1 to 5
TEST(WhileAndDoWhileTests, WhileLoopSum) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        sum = 0
        i = 1
        while (i <= 5) {
            sum = sum + i
            i = i + 1
        }
        sum
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 15);  // 1+2+3+4+5 = 15
}

// Test 2: Do-while loop - sum numbers from 1 to 5
TEST(WhileAndDoWhileTests, DoWhileLoopSum) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        sum = 0
        i = 1
        do {
            sum = sum + i
            i = i + 1
        } while (i <= 5)
        sum
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 15);  // 1+2+3+4+5 = 15
}

// Test 3: While loop that never executes (condition false from start)
TEST(WhileAndDoWhileTests, WhileLoopNeverExecutes) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        counter = 0
        while (false) {
            counter = counter + 1
        }
        counter
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 0);  // Never incremented
}

// Test 4: Do-while executes once even with false condition
TEST(WhileAndDoWhileTests, DoWhileExecutesOnceWithFalse) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        counter = 0
        do {
            counter = counter + 1
        } while (false)
        counter
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 1);  // Executed once
}

// Test 5: Nested while loops - multiplication table
TEST(WhileAndDoWhileTests, NestedWhileLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        result = 0
        i = 1
        while (i <= 3) {
            j = 1
            while (j <= 3) {
                result = result + (i * j)
                j = j + 1
            }
            i = i + 1
        }
        result
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    // (1*1 + 1*2 + 1*3) + (2*1 + 2*2 + 2*3) + (3*1 + 3*2 + 3*3)
    // = (1+2+3) + (2+4+6) + (3+6+9) = 6 + 12 + 18 = 36
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 36);
}

// Test 6: Nested do-while loops
TEST(WhileAndDoWhileTests, NestedDoWhileLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        result = 0
        i = 1
        do {
            j = 1
            do {
                result = result + 1
                j = j + 1
            } while (j <= 2)
            i = i + 1
        } while (i <= 2)
        result
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 4);  // 2 outer * 2 inner = 4
}

// Test 7: While loop with break
TEST(WhileAndDoWhileTests, WhileLoopWithBreak) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        i = 0
        while (i < 10) {
            i = i + 1
            if (i == 5) {
                break
            }
        }
        i
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 5);
}

// Test 8: Do-while with continue
TEST(WhileAndDoWhileTests, DoWhileWithContinue) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        sum = 0
        i = 0
        do {
            i = i + 1
            if (i % 2 != 0) {
                sum = sum + i
            }
        } while (i < 6)
        sum
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 9);  // 1 + 3 + 5 = 9
}

// Test 9: Mixed while and do-while loops
TEST(WhileAndDoWhileTests, MixedWhileAndDoWhile) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        total = 0
        
        // First use while loop
        i = 0
        while (i < 3) {
            total = total + 10
            i = i + 1
        }
        
        // Then use do-while loop  
        j = 0
        do {
            total = total + 1
            j = j + 1
        } while (j < 3)
        
        total
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()),
              33);  // 30 from while + 3 from do-while
}

// Test 10: Complex condition expressions in loops
TEST(WhileAndDoWhileTests, ComplexConditionsInLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        // While with complex condition
        x = 0
        y = 10
        while (x < 5 && y > 5) {
            x = x + 1
            y = y - 1
        }
        
        // Do-while with complex condition
        a = 0
        b = 0
        do {
            a = a + 1
            b = b + 2
        } while (a < 3 || b < 4)
        
        x + y + a + b
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    // x=5, y=5 (loop runs 5 times)
    // a=3, b=6 (loop runs 3 times)
    // Total: 5 + 5 + 3 + 6 = 19
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 19);
}

// Test 11: While loop with continuations
TEST(WhileAndDoWhileTests, WhileLoopWithContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create continuations in a while loop
        funcs = []
        i = 0
        while (i < 3) {
            { i * 2 }
            funcs dup size swap store
            i = i + 1
        }
        
        // Execute all continuations
        sum = 0
        j = 0
        while (j < funcs size) {
            funcs j at '
            sum = sum +
            j = j + 1
        }
        sum
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 6);  // 0 + 2 + 4 = 6
}

// Test 12: Do-while loop with continuations
TEST(WhileAndDoWhileTests, DoWhileLoopWithContinuations) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));
    reg.AddClass<Continuation>(Label("Continuation"));

    const std::string code = R"(
        // Create continuations in a do-while loop
        funcs = []
        i = 1
        do {
            { i i * }  // Square function
            funcs dup size swap store
            i = i + 1
        } while (i <= 3)
        
        // Execute continuations
        result = 0
        j = 0
        do {
            funcs j at '
            result = result +
            j = j + 1
        } while (j < funcs size)
        result
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 14);  // 1 + 4 + 9 = 14
}

// Test 13: Factorial using while loop
TEST(WhileAndDoWhileTests, FactorialUsingWhile) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        n = 5
        factorial = 1
        i = 1
        while (i <= n) {
            factorial = factorial * i
            i = i + 1
        }
        factorial
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 120);  // 5! = 120
}

// Test 14: Fibonacci using do-while
TEST(WhileAndDoWhileTests, FibonacciUsingDoWhile) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        // Calculate 8th Fibonacci number
        n = 8
        a = 0
        b = 1
        i = 2
        
        if (n == 0) {
            result = 0
        } else if (n == 1) {
            result = 1
        } else {
            do {
                temp = a + b
                a = b
                b = temp
                i = i + 1
            } while (i <= n)
            result = b
        }
        result
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 21);  // 8th Fibonacci number
}

// Test 15: String concatenation in loops
TEST(WhileAndDoWhileTests, StringConcatenationInLoops) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<String>(Label("String"));

    const std::string code = R"(
        // Build string with while loop
        s1 = ""
        i = 0
        while (i < 3) {
            s1 = s1 + "A"
            i = i + 1
        }
        
        // Build string with do-while loop
        s2 = ""
        j = 0
        do {
            s2 = s2 + "B"
            j = j + 1
        } while (j < 2)
        
        s1 + s2
    )";

    console.Execute(code);

    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(ConstDeref<String>(stack->Top()), "AAABB");
}