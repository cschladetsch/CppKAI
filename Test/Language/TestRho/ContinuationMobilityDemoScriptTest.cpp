#include <gtest/gtest.h>

#include <KAI/Console/Console.h>

#include "TestRhoUtils.h"

TEST(ContinuationMobilityDemoScriptTest, ScriptExecutesSuccessfully) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);

    std::string script = kai::test::LoadRhoScript("ContinuationMobilityDemo.rho");
    EXPECT_NO_THROW(console.Execute(script.c_str(), kai::Structure::Program));
}
