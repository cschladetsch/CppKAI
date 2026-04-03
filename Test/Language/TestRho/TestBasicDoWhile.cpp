#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Simple test for basic do-while functionality
TEST(BasicDoWhileTest, SimpleDoWhileExecutesOnce) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    // Test with proper Rho syntax
    const std::string code = R"(
        counter = 0
        do {
            counter = counter + 1
        } while (counter < 3)
        
        counter
    )";

    try {
        console.Execute(code);

        auto stack = console.GetExecutor()->GetDataStack();
        ASSERT_GE(stack->Size(), 1) << "Stack should have at least one element";
        ASSERT_EQ(ConstDeref<int>(stack->Top()), 3) << "Counter should be 3";
    } catch (const Exception::Base& e) {
        FAIL() << "KAI Exception: " << e.ToString();
    } catch (const std::exception& e) {
        FAIL() << "Exception: " << e.what();
    }
}

// Test that do-while executes at least once even if condition is false
TEST(BasicDoWhileTest, ExecutesAtLeastOnce) {
    Console console;
    console.SetLanguage(Language::Rho);

    auto& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    const std::string code = R"(
        counter = 10
        do {
            counter = counter + 1
        } while (counter < 5)
        
        counter
    )";

    try {
        console.Execute(code);

        auto stack = console.GetExecutor()->GetDataStack();
        ASSERT_GE(stack->Size(), 1) << "Stack should have at least one element";
        ASSERT_EQ(ConstDeref<int>(stack->Top()), 11) << "Counter should be 11";
    } catch (const Exception::Base& e) {
        FAIL() << "KAI Exception: " << e.ToString();
    } catch (const std::exception& e) {
        FAIL() << "Exception: " << e.what();
    }
}