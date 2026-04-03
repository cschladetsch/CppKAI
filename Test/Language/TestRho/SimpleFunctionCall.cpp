#include <gtest/gtest.h>

#include "TestLangCommon.h"
#include "TestRhoUtils.h"

// Simple test to debug function calls
struct SimpleFunctionCall : kai::TestLangCommon {};

TEST_F(SimpleFunctionCall, BasicCall) {
    // Enable verbose tracing
    kai::Process::trace = 0;  // Disable verbose tracing for cleaner output

    try {
        // Use the console from TestLangCommon which has translators set up
        console_.SetLanguage(kai::Language::Rho);

        // Read script from file
        std::string script = kai::test::LoadRhoScript("SimpleFunctionCall.rho");

        // Execute as Program since we have multiple statements
        console_.Execute(script.c_str(), kai::Structure::Program);

        // The function call result should be on the stack
        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty after function call";

        auto result = data_->Top();
        ASSERT_TRUE(result.IsType<int>()) << "Result should be an integer, got: " + result.GetClass()->GetName().ToString();
        ASSERT_EQ(kai::ConstDeref<int>(result), 6)
            << "double(3) should return 6";

    } catch (const kai::Exception::Base &e) {
        FAIL() << "Exception: " << e.ToString();
    }

    kai::Process::trace = 0;
}