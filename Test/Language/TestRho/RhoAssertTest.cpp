#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Test for Rho language assertion
// Now using direct execution instead of workaround since the continuation handling has been fixed
TEST(RhoLanguage, AssertTest) {
    Console console;
    console.SetLanguage(Language::Rho);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    
    // Clear the stack before our test
    stack->Clear();
    
    // The assert operation doesn't return a value
    // but just throws an exception if the assertion fails.
    // If it passes, nothing happens, so the stack should remain unchanged.
    
    cout << "Testing assert(1+1==2) in Rho language" << endl;
    
    // Test a passing assertion
    try {
        // Now we can directly use console.Execute with our fixed Rho implementation
        console.Process("assert(1+1==2);");
        
        // If we get here, the assertion passed (no exception)
        SUCCEED() << "Assertion passed as expected";
    }
    catch (const std::exception& e) {
        FAIL() << "Assertion test failed with exception: " << e.what();
    }
    
    // Test a failing assertion
    cout << "Testing assert(1+1==3) in Rho language (should fail)" << endl;
    
    bool assertionFailed = false;
    try {
        console.Process("assert(1+1==3);");
        
        // If we get here, the assertion didn't fail as expected
        FAIL() << "Failing assertion did not throw an exception as expected";
    }
    catch (const std::exception&) {
        // This is expected - the assertion should fail
        assertionFailed = true;
    }
    
    ASSERT_TRUE(assertionFailed) << "Failing assertion threw exception as expected";
}