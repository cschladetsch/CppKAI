#include <gtest/gtest.h>

#include "TestLangCommon.h"
#include "TestRhoUtils.h"

// Simple test to debug function calls
struct SimpleFunctionCall : kai::TestLangCommon {};

TEST_F(SimpleFunctionCall, BasicCall) {
    // Enable verbose tracing
    kai::Process::trace = 0;  // Disable verbose tracing for cleaner output

    try {
        kai::Console console;
        console.SetLanguage(kai::Language::Rho);

        // Read script from file
        std::string script = kai::test::LoadRhoScript("SimpleFunctionCall.rho");

        // Instead of Structure::Program, use Structure::Expression
        // to get the result on the stack
        console.Execute(script.c_str(), kai::Structure::Expression);

        auto exec = console.GetExecutor();
        auto stack = exec->GetDataStack();

        ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";

        auto result = stack->Top();
        ASSERT_TRUE(result.IsType<int>()) << "Result should be an integer";
        ASSERT_EQ(kai::ConstDeref<int>(result), 6)
            << "double(3) should return 6";

    } catch (const kai::Exception::Base &e) {
        FAIL() << "Exception: " << e.ToString();
    }

    kai::Process::trace = 0;
}