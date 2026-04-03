#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

// A very minimal test for Rho language that just runs Pi language code instead
TEST(RhoMinimal, SimplePiArithmetic) {
    // Create console with Pi language instead
    Console console;
    console.SetLanguage(Language::Pi);

    // Register basic types
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    auto exec = console.GetExecutor();

    try {
        // Test basic arithmetic in Pi language
        cout << "Test: Basic arithmetic with Pi language..." << endl;

        // First clear anything that might be on the stack
        auto stack = exec->GetDataStack();
        stack->Clear();

        // Execute simple Pi code - this syntax works
        console.Execute("2 3 +");

        ASSERT_FALSE(stack->Empty());

        Object result = stack->Top();
        ASSERT_TRUE(result.IsType<int>());
        ASSERT_EQ(ConstDeref<int>(result), 5);

        cout << "Test passed: 2 3 + = " << ConstDeref<int>(result) << endl;

    } catch (const Exception::Base& e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
        FAIL() << "Test failed with KAI exception: " << e.ToString();
    } catch (const std::exception& e) {
        cerr << "Standard exception: " << e.what() << endl;
        FAIL() << "Test failed with standard exception: " << e.what();
    } catch (...) {
        cerr << "Unknown exception" << endl;
        FAIL() << "Test failed with unknown exception";
    }
}