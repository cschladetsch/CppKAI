#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Very simple test to debug execution
struct VerySimpleTest : kai::TestLangCommon {};

TEST_F(VerySimpleTest, JustAssignment) {
    try {
        kai::Console console;
        console.SetLanguage(kai::Language::Rho);

        const char* script = "x = 5";

        console.Execute(script, kai::Structure::Program);

        auto exec = console.GetExecutor();
        auto stack = exec->GetDataStack();

        // For just an assignment, the stack might be empty
        // Let's check what's there
        std::cout << "Stack size after assignment: " << stack->Size()
                  << std::endl;

        // For now, just pass if no exception was thrown
        SUCCEED() << "Script executed without exceptions";

    } catch (const kai::Exception::Base& e) {
        FAIL() << "Exception: " << e.ToString();
    }
}