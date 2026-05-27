#include <gtest/gtest.h>

#include <KAI/Console/Console.h>

#include <fstream>
#include <sstream>

TEST(ContinuationMobilityDemoScriptTest, ScriptExecutesSuccessfully) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);

    std::ifstream file("ContinuationMobilityDemo/ContinuationMobilityDemo.rho");
    ASSERT_TRUE(file.is_open());

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string script = buffer.str();

    EXPECT_NO_THROW(console.Execute(script.c_str(), kai::Structure::Program));
}
