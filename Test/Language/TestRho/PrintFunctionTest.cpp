#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct PrintFunctionTest : TestLangCommon {
    std::stringstream capturedOutput;
    std::streambuf* oldCout;

    void SetUp() override {
        TestLangCommon::SetUp();
        // Redirect cout to capture print output
        oldCout = std::cout.rdbuf();
        std::cout.rdbuf(capturedOutput.rdbuf());
    }

    void TearDown() override {
        // Restore cout
        std::cout.rdbuf(oldCout);
        TestLangCommon::TearDown();
    }

    std::string GetCapturedOutput() { return capturedOutput.str(); }
};

TEST_F(PrintFunctionTest, SimplePrint) {
    console_.SetLanguage(Language::Rho);
    console_.Execute("print(\"Hello, World!\")", Structure::Statement);

    std::string output = GetCapturedOutput();
    EXPECT_TRUE(output.find("Hello, World!") != std::string::npos);
}

TEST_F(PrintFunctionTest, PrintVariable) {
    console_.SetLanguage(Language::Rho);
    console_.Execute("x = 42", Structure::Statement);
    console_.Execute("print(x)", Structure::Statement);

    std::string output = GetCapturedOutput();
    EXPECT_TRUE(output.find("42") != std::string::npos);
}

TEST_F(PrintFunctionTest, PrintExpression) {
    console_.SetLanguage(Language::Rho);
    console_.Execute("print(10 + 32)", Structure::Statement);

    std::string output = GetCapturedOutput();
    EXPECT_TRUE(output.find("42") != std::string::npos);
}

TEST_F(PrintFunctionTest, MultiplePrints) {
    console_.SetLanguage(Language::Rho);
    console_.Execute("print(\"First line\")", Structure::Statement);
    console_.Execute("print(\"Second line\")", Structure::Statement);
    console_.Execute("print(\"Third line\")", Structure::Statement);

    std::string output = GetCapturedOutput();
    EXPECT_TRUE(output.find("First line") != std::string::npos);
    EXPECT_TRUE(output.find("Second line") != std::string::npos);
    EXPECT_TRUE(output.find("Third line") != std::string::npos);
}

TEST_F(PrintFunctionTest, PrintInFunction) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
greet = fun(name)
    print("Hello, " + name + "!")

greet("Alice")
greet("Bob")
)";

    console_.Execute(code, Structure::Program);

    std::string output = GetCapturedOutput();
    EXPECT_TRUE(output.find("Hello, Alice!") != std::string::npos);
    EXPECT_TRUE(output.find("Hello, Bob!") != std::string::npos);
}

TEST_F(PrintFunctionTest, PrintInLoop) {
    console_.SetLanguage(Language::Rho);

    const char* code = R"(
for i = 0; i < 3; i = i + 1
    print(i)
)";

    console_.Execute(code, Structure::Program);

    std::string output = GetCapturedOutput();
    EXPECT_TRUE(output.find("0") != std::string::npos);
    EXPECT_TRUE(output.find("1") != std::string::npos);
    EXPECT_TRUE(output.find("2") != std::string::npos);
}