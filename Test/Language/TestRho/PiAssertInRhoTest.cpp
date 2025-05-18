#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test for Pi assertion inside Rho
TEST(RhoLanguage, PiAssertInRho) {
    Console console;
    console.SetLanguage(Language::Rho);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Clear the stack before our test
    stack->Clear();

    cout << "Testing Pi assertion inside Rho: pi { 1 1 + 2 == assert }" << endl;

    // Test a passing assertion
    try {
        // Now we can directly process the Pi assertion in Rho
        console.Process("pi { 1 1 + 2 == assert }");

        // If we get here, the assertion passed (no exception)
        SUCCEED() << "Pi assertion inside Rho passed successfully";
    } catch (const std::exception& e) {
        FAIL() << "Pi assertion test failed with exception: " << e.what();
    }

    // Test a failing assertion
    cout << "Testing failing Pi assertion inside Rho: pi { 1 1 + 3 == assert }"
         << endl;

    // The Process method catches exceptions and returns a string with the error
    // message It won't throw an exception to the caller, so we need to check
    // the return value
    String result = console.Process("pi { 1 1 + 3 == assert }");

    // Convert to std::string for easier testing
    std::string stdResult = result.StdString();

    // If the assertion failed, the result should contain an exception message
    // The Process method returns a string with "Exception: " prefix when an
    // exception occurs
    ASSERT_FALSE(stdResult.empty())
        << "Failing Pi assertion should return an error message";
    ASSERT_TRUE(stdResult.find("Exception:") != std::string::npos)
        << "Error message should indicate an exception occurred, but got: "
        << stdResult;
}