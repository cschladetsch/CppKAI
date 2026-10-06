// Sigma's continuation operators after a call: `f(x)&` (suspend, an ordinary
// call written out) and `f(x)!` (replace: a tail call). See Doc/Sigma.md.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;

struct SigmaContinuationTests : TestLangCommon {
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

    std::string Rho(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        EXPECT_TRUE(sigma.Compile(code.c_str())) << Errors(code);
        return sigma.GetRho();
    }

    void ExpectError(const std::string &code, const std::string &fragment) {
        const std::string all = Errors(code);
        EXPECT_NE(all.find(fragment), std::string::npos)
            << "expected an error containing '" << fragment << "' for:\n"
            << code << "\n--- got:\n"
            << (all.empty() ? "(no errors)\n" : all);
    }

    template <class T>
    T Run(const std::string &code) {
        data_->Clear();
        console_.Execute(code.c_str(), Structure::Program);
        EXPECT_FALSE(data_->Empty()) << "no result for:\n" << code << "\n--- rho:\n" << Rho(code);
        if (data_->Empty()) return T();
        Object top = data_->Top();
        EXPECT_TRUE(top.IsType<T>()) << "wrong result type for:\n" << code;
        return top.IsType<T>() ? ConstDeref<T>(top) : T();
    }
};

static const char *Tens = "fun tens(n: int) -> int\n    return n * 10\n";

// --- '&' (suspend) -------------------------------------------------------------

TEST_F(SigmaContinuationTests, SuspendIsAnOrdinaryCall) {
    EXPECT_EQ(Run<int>(std::string(Tens) + "x = tens(3)&\nx + 1"), 31);
}

TEST_F(SigmaContinuationTests, SuspendInArguments) {
    EXPECT_EQ(Run<int>(std::string(Tens) + "fun add(a: int, b: int) -> int\n    return a + b\nadd(tens(1)&, tens(2)&)"),
              30);
}

TEST_F(SigmaContinuationTests, SuspendInsideFunction) {
    EXPECT_EQ(Run<int>(std::string(Tens) + "fun f(n: int) -> int\n    if n > 1\n        return tens(n)&\n    return 0\nf(4)"),
              40);
}

TEST_F(SigmaContinuationTests, SuspendKeepsItsType) {
    ExpectError(std::string(Tens) + "s: str = tens(1)&", "expected str, got int");
}

TEST_F(SigmaContinuationTests, SuspendIsGluedInGeneratedRho) {
    EXPECT_NE(Rho(std::string(Tens) + "x = tens(3)&").find("x = tens(3)&"), std::string::npos);
}

// With a space, '&' is the bitwise and of the call's result.
TEST_F(SigmaContinuationTests, SpacedAmpersandIsBitwiseAnd) {
    EXPECT_EQ(Run<int>(std::string(Tens) + "a = tens(6) & 3\nb = tens(7) & 6\na + b"), 6);
}

TEST_F(SigmaContinuationTests, BitwiseAndOfCallIsWrappedForRho) {
    EXPECT_NE(Rho(std::string(Tens) + "a = tens(6) & 3").find("((tens(6)) & 3)"), std::string::npos);
}

// --- '!' (replace: tail call) --------------------------------------------------

TEST_F(SigmaContinuationTests, TailCall) {
    EXPECT_EQ(Run<int>(std::string(Tens) + "fun f(n: int) -> int\n    return tens(n)!\nf(3)"), 30);
}

TEST_F(SigmaContinuationTests, TailCallAfterOtherStatements) {
    EXPECT_EQ(Run<int>(std::string(Tens) + "fun f(n: int) -> int\n    m = n + 1\n    return tens(m)!\nf(2) * 2"), 60);
}

TEST_F(SigmaContinuationTests, TailRecursion) {
    EXPECT_EQ(Run<int>("fun sum(n: int, acc: int) -> int\n"
                       "    if n == 0\n"
                       "        return acc\n"
                       "    return sum(n - 1, acc + n)!\n"
                       "sum(100, 0) + 1"),
              5051);
}

TEST_F(SigmaContinuationTests, MutualTailRecursion) {
    EXPECT_TRUE(Run<bool>("fun even(n: int) -> bool\n"
                          "    if n == 0\n"
                          "        return true\n"
                          "    return odd(n - 1)!\n"
                          "fun odd(n: int) -> bool\n"
                          "    if n == 0\n"
                          "        return false\n"
                          "    return even(n - 1)!\n"
                          "even(10)"));
}

TEST_F(SigmaContinuationTests, TailCallFromLoop) {
    EXPECT_EQ(Run<int>(std::string(Tens) +
                       "fun f(n: int) -> int\n    return tens(n)!\n"
                       "c = 0\nfor i = 0; i < 4; i += 1\n    c += f(i)\nc"),
              60);
}

TEST_F(SigmaContinuationTests, TailCallThroughFunctionValue) {
    EXPECT_EQ(Run<int>(std::string(Tens) +
                       "fun apply(g: fun(int) -> int, x: int) -> int\n    return g(x)!\napply(tens, 5)"),
              50);
}

TEST_F(SigmaContinuationTests, VoidTailCall) {
    EXPECT_EQ(Run<int>("fun show(n: int)\n    print(n)\nfun f(n: int)\n    show(n)!\nf(5)\n1"), 1);
}

TEST_F(SigmaContinuationTests, TailCallCountsAsReturn) {
    EXPECT_EQ(Errors(std::string(Tens) + "fun f(n: int) -> int\n    return tens(n)!\n"), "");
}

// --- rejected uses ----------------------------------------------------------------

TEST_F(SigmaContinuationTests, TailCallInsideIfIsRejected) {
    ExpectError(std::string(Tens) + "fun f(n: int) -> int\n    if n > 0\n        return tens(n)!\n    return 0\n",
                "must be the last statement of a function body");
}

TEST_F(SigmaContinuationTests, TailCallInsideLoopIsRejected) {
    ExpectError(std::string(Tens) + "fun f(n: int) -> int\n    while n > 0\n        return tens(n)!\n    return 0\n",
                "must be the last statement of a function body");
}

TEST_F(SigmaContinuationTests, TailCallInExpressionIsRejected) {
    ExpectError(std::string(Tens) + "fun f(n: int) -> int\n    x = tens(n)!\n    return x\n",
                "must be the last statement of a function body");
}

TEST_F(SigmaContinuationTests, TailCallAtTopLevelIsRejected) {
    ExpectError(std::string(Tens) + "tens(1)!", "can only be used inside a function");
}

TEST_F(SigmaContinuationTests, TailCallDoesNotWiden) {
    ExpectError(std::string(Tens) + "fun f(n: int) -> float\n    return tens(n)!\n",
                "must return exactly float, got int");
}

TEST_F(SigmaContinuationTests, VoidTailCallMustCallVoid) {
    ExpectError(std::string(Tens) + "fun f(n: int)\n    tens(n)!\n", "must call a void function, got int");
}

TEST_F(SigmaContinuationTests, NotOnMethods) {
    ExpectError("xs: List[int] = [1]\nn = xs.size()&", "not built-ins or methods");
}

TEST_F(SigmaContinuationTests, NotOnPrint) {
    ExpectError("fun f(n: int)\n    print(n)!\n", "not built-ins or methods");
}

TEST_F(SigmaContinuationTests, SpacedBangIsNotATailCall) {
    ExpectError(std::string(Tens) + "fun f(n: int) -> int\n    return tens(n) !\n", "expected end of statement");
}

TEST_F(SigmaContinuationTests, ResumeIsRejected) {
    ExpectError(std::string(Tens) + "fun f(n: int)\n    tens(n)...\n", "'...' (resume) is not supported");
}
