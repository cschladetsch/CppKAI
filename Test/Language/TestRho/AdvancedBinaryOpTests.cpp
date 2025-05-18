#include <gtest/gtest.h>
#include <iostream>
#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

// Extremely simple test for Rho
TEST(RhoAdvancedOps, SimpleAddition) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();
    
    // Execute a very simple addition: 10 + 20
    console.Execute("10 20 +");
    
    // Check result
    ASSERT_FALSE(stack->Empty());
}