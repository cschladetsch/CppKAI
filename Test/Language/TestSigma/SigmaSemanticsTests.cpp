// Operator semantics that were once wrong: short-circuit && and ||, void
// operands, float modulo, bare print, and the position of string errors.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;

struct SigmaSemanticsTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Sigma);
    }

    std::string Errors(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        sigma.Compile(code.c_str());
        std::string all;
        for (auto const &e : sigma.GetErrors()) all += e + "\n";
        return all;
    }

    void ExpectFirstError(const std::string &code, const std::string &expected) {
        const std::string errors = Errors(code);
        EXPECT_EQ(errors.substr(0, errors.find('\n')), expected) << code;
    }

    template <class T>
    T Run(const std::string &code) {
        data_->Clear();
        console_.Execute(code.c_str(), Structure::Program);
        EXPECT_FALSE(data_->Empty()) << "no result for:\n" << code << "\n" << Errors(code);
        if (data_->Empty()) return T();
        Object top = data_->Top();
        EXPECT_TRUE(top.IsType<T>()) << "wrong result type for:\n" << code;
        return top.IsType<T>() ? ConstDeref<T>(top) : T();
    }
};

TEST_F(SigmaSemanticsTests, AndSkipsItsRightSideWhenLeftIsFalse) {
    EXPECT_EQ(Run<int>(
        "n = 0\n"
        "fun bump() -> bool\n"
        "    n += 1\n"
        "    return true\n"
        "b = false && bump()\n"
        "n"), 0);
}

TEST_F(SigmaSemanticsTests, OrSkipsItsRightSideWhenLeftIsTrue) {
    EXPECT_EQ(Run<int>(
        "n = 0\n"
        "fun bump() -> bool\n"
        "    n += 1\n"
        "    return true\n"
        "b = true || bump()\n"
        "n"), 0);
}

TEST_F(SigmaSemanticsTests, AndStillEvaluatesRightSideWhenLeftIsTrue) {
    EXPECT_EQ(Run<int>(
        "n = 0\n"
        "fun bump() -> bool\n"
        "    n += 1\n"
        "    return true\n"
        "b = true && bump()\n"
        "n"), 1);
}

TEST_F(SigmaSemanticsTests, GuardedIndexOfEmptyListIsSafe) {
    EXPECT_EQ(Run<bool>(
        "xs: List[int] = []\n"
        "ok = xs.size() > 0 && xs[0] > 0\n"
        "ok"), false);
}

TEST_F(SigmaSemanticsTests, VoidInListLiteralIsRejected) {
    ExpectFirstError(
        "fun g()\n    print(1)\nxs = [g()]\n",
        "3:6: list elements cannot be void");
}

TEST_F(SigmaSemanticsTests, VoidInMapLiteralIsRejected) {
    ExpectFirstError(
        "fun g()\n    print(1)\nm = {\"a\": g()}\n",
        "3:5: map values cannot be void");
}

TEST_F(SigmaSemanticsTests, ComparingVoidValuesIsRejected) {
    ExpectFirstError(
        "fun g()\n    print(1)\nb = g() == g()\n",
        "3:9: operator '==' cannot be applied to void and void");
}

TEST_F(SigmaSemanticsTests, FloatModuloIsRejectedStatically) {
    ExpectFirstError("r = 5.5 % 2\n", "1:9: operator '%' cannot be applied to float and int");
}

TEST_F(SigmaSemanticsTests, IntegerModuloStillWorks) {
    EXPECT_EQ(Run<int>("r = 17 % 5\nr"), 2);
}

TEST_F(SigmaSemanticsTests, PrintNeedsAnArgument) {
    ExpectFirstError("print()\n", "1:6: 'print' expects at least 1 argument, got 0");
}

TEST_F(SigmaSemanticsTests, UnterminatedDoubleQuotedStringHasAPosition) {
    ExpectFirstError("s = \"abc\n", "1:5: unterminated string");
}
