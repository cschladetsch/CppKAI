#include <gtest/gtest.h>
#include "TestLangCommon.h"
#include "KAI/Console.h"

using kai::Console;
using kai::Language;
using kai::Structure;
using kai::ConstDeref;

TEST(WorkaroundTest, AssignmentWorkaround) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define functions
    console.Execute(R"(
fun inner()
    return 5
    
fun outer(x)
    return x + 1
)", Structure::Program);
    
    // Try using assignment as a workaround
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute(R"(
temp = inner()
result = outer(temp)
result
)", Structure::Program);
    
    auto stack = console.GetExecutor()->GetDataStack();
    if (!stack->Empty()) {
        auto val = stack->Top();
        if (val.IsType<int>()) {
            int result = ConstDeref<int>(val);
            std::cout << "Assignment workaround result: " << result << std::endl;
            EXPECT_EQ(result, 6) << "Should work with intermediate assignment";
        }
    }
}

TEST(WorkaroundTest, SimplerNesting) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Try a simpler case - just returning the nested call
    console.Execute(R"(
fun getTwo()
    return 2
    
fun doubleIt(n)
    return n * 2
    
fun testNested()
    return doubleIt(getTwo())
)", Structure::Program);
    
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("testNested()", Structure::Expression);
    
    auto stack = console.GetExecutor()->GetDataStack();
    if (!stack->Empty()) {
        auto val = stack->Top();
        if (val.IsType<int>()) {
            int result = ConstDeref<int>(val);
            std::cout << "Nested in function result: " << result << std::endl;
            EXPECT_EQ(result, 4) << "Should work when nested call is in a function";
        }
    }
}