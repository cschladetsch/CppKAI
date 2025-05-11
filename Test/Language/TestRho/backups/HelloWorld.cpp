#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

// Simple standalone test for Rho language that doesn't use any do-while
// constructs
TEST(RhoMinimal, HelloWorld) {
    // Create console and set language
    Console console;
    console.SetLanguage(Language::Rho);

    // Register only required types
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<String>(Label("String"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();

    try {
        // Run a simple string concatenation test
        cout << "Executing hello world script..." << endl;
        try {
            // Break up the script into separate statements for better debugging
            cout << "Setting greeting variable..." << endl;
            console.Execute("greeting = \"Hello, World!\"");

            cout << "Reading greeting variable..." << endl;
            console.Execute("greeting");
        } catch (const Exception::Base& e) {
            cerr << "KAI Exception during execution: " << e.ToString() << endl;
            FAIL() << "Test failed with exception during execution: "
                   << e.ToString();
        } catch (const std::exception& e) {
            cerr << "Standard exception during execution: " << e.what() << endl;
            FAIL() << "Test failed with standard exception during execution: "
                   << e.what();
        }

        // Get the result
        auto stack = exec->GetDataStack();
        ASSERT_FALSE(stack->Empty());

        Object result = stack->Top();
        ASSERT_TRUE(result.IsType<String>());

        // Check the result
        String greeting = ConstDeref<String>(result);
        cout << "Result: " << greeting.c_str() << endl;
        ASSERT_EQ(greeting, "Hello, World!");

        cout << "Hello world test passed!" << endl;
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