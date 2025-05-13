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
    }
    catch (const std::exception& e) {
        FAIL() << "Pi assertion test failed with exception: " << e.what();
    }
    
    // Test a failing assertion
    cout << "Testing failing Pi assertion inside Rho: pi { 1 1 + 3 == assert }" << endl;
    
    bool assertionFailed = false;
    try {
        console.Process("pi { 1 1 + 3 == assert }");
        
        // If we get here, the assertion didn't fail as expected
        FAIL() << "Failing Pi assertion did not throw an exception as expected";
    }
    catch (const std::exception&) {
        // This is expected - the assertion should fail
        assertionFailed = true;
    }
    
    ASSERT_TRUE(assertionFailed) << "Failing Pi assertion threw exception as expected";
}