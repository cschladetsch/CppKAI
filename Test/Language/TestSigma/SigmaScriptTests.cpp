// One test per example program in Scripts/. Each script must type-check,
// run, and end with an expression that is true.

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;
namespace fs = std::filesystem;

struct SigmaScriptTests : TestLangCommon {
    void RunScript(const std::string &name) {
        const fs::path path = fs::path(KAI_STRINGISE(KAI_SCRIPT_ROOT)) / (name + ".sigma");
        std::ifstream in(path);
        ASSERT_TRUE(in.good()) << "missing script " << path;
        std::stringstream text;
        text << in.rdbuf();

        SigmaTranslator sigma(*reg_);
        auto program = sigma.Translate(text.str().c_str(), Structure::Program);
        std::string errors;
        for (auto const &e : sigma.GetErrors()) errors += e + "\n";
        ASSERT_FALSE(sigma.failed) << name << ".sigma did not compile:\n" << errors;

        data_->Clear();
        ASSERT_NO_THROW(console_.Execute(program)) << name << ".sigma threw\n--- rho:\n" << sigma.GetRho();
        ASSERT_FALSE(data_->Empty()) << name << ".sigma left no result\n--- rho:\n" << sigma.GetRho();
        Object result = data_->Top();
        ASSERT_TRUE(result.IsType<bool>()) << name << ".sigma should end with a bool expression";
        EXPECT_TRUE(ConstDeref<bool>(result)) << name << ".sigma ended with false\n--- rho:\n" << sigma.GetRho();
    }
};

TEST_F(SigmaScriptTests, BubbleSort) { RunScript("BubbleSort"); }
TEST_F(SigmaScriptTests, GcdLcm) { RunScript("GcdLcm"); }
TEST_F(SigmaScriptTests, Collatz) { RunScript("Collatz"); }
TEST_F(SigmaScriptTests, Factorial) { RunScript("Factorial"); }
TEST_F(SigmaScriptTests, Binary) { RunScript("Binary"); }
TEST_F(SigmaScriptTests, ReverseList) { RunScript("ReverseList"); }
TEST_F(SigmaScriptTests, SumOfSquares) { RunScript("SumOfSquares"); }
TEST_F(SigmaScriptTests, FizzBuzz) { RunScript("FizzBuzz"); }
TEST_F(SigmaScriptTests, Sieve) { RunScript("Sieve"); }
TEST_F(SigmaScriptTests, Matrix) { RunScript("Matrix"); }
TEST_F(SigmaScriptTests, Inventory) { RunScript("Inventory"); }
TEST_F(SigmaScriptTests, HigherOrder) { RunScript("HigherOrder"); }
TEST_F(SigmaScriptTests, FibonacciList) { RunScript("FibonacciList"); }
TEST_F(SigmaScriptTests, NewtonSqrt) { RunScript("NewtonSqrt"); }
TEST_F(SigmaScriptTests, Temperature) { RunScript("Temperature"); }
TEST_F(SigmaScriptTests, Balanced) { RunScript("Balanced"); }
TEST_F(SigmaScriptTests, BinarySearch) { RunScript("BinarySearch"); }
TEST_F(SigmaScriptTests, MinMax) { RunScript("MinMax"); }
TEST_F(SigmaScriptTests, StringJoin) { RunScript("StringJoin"); }
TEST_F(SigmaScriptTests, Hanoi) { RunScript("Hanoi"); }
