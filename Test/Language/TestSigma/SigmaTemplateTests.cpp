// Template functions: `fun name[T, ...](...)`. Type arguments are inferred at
// each call and the body is checked once per distinct set of them, as with
// C++ templates. Rho is untyped, so the generated code is the same either way.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;

namespace {
const char *const kMax =
    "fun max[T](a: T, b: T) -> T\n"
    "    return a > b ? a : b\n";
}

struct SigmaTemplateTests : TestLangCommon {
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

    void ExpectError(const std::string &code, const std::string &fragment) {
        const std::string all = Errors(code);
        EXPECT_NE(all.find(fragment), std::string::npos)
            << "expected an error containing '" << fragment << "' for:\n"
            << code << "\n--- got:\n"
            << (all.empty() ? "(no errors)\n" : all);
    }

    void ExpectClean(const std::string &code) { EXPECT_EQ(Errors(code), "") << code; }

    template <class T>
    T Run(const std::string &code) {
        data_->Clear();
        console_.Execute(code.c_str(), Structure::Program);
        EXPECT_FALSE(data_->Empty()) << "no result for:\n" << code;
        if (data_->Empty()) return T();
        Object top = data_->Top();
        EXPECT_TRUE(top.IsType<T>()) << "wrong result type for:\n" << code;
        return top.IsType<T>() ? ConstDeref<T>(top) : T();
    }
};

// --- Running ---------------------------------------------------------------

TEST_F(SigmaTemplateTests, OneTemplateManyTypes) {
    EXPECT_TRUE(Run<bool>(std::string(kMax) +
                          "max(3, 9) == 9 && max(\"pear\", \"apple\") == \"pear\" && max(2.5, 1.0) == 2.5"));
}

TEST_F(SigmaTemplateTests, IntAndFloatArgumentsMakeTFloat) {
    EXPECT_FLOAT_EQ(Run<float>(std::string(kMax) + "r = max(3, 2.5)\nr"), 3.0f);
    EXPECT_FLOAT_EQ(Run<float>(std::string(kMax) + "r = max(1, 2.5)\nr"), 2.5f);

    SigmaTranslator sigma(*reg_);
    ASSERT_TRUE(sigma.Compile((std::string(kMax) + "r = max(3, 2.5)\n").c_str()));
    EXPECT_NE(sigma.GetRho().find("(3 + 0.0)"), std::string::npos) << sigma.GetRho();
}

TEST_F(SigmaTemplateTests, ResultTypeIsInferred) {
    ExpectClean(std::string(kMax) + "s = max(\"a\", \"b\")\nn: int = s.size()\n");
    ExpectError(std::string(kMax) + "s = max(\"a\", \"b\")\nn: int = s\n", "expected int, got str");
}

TEST_F(SigmaTemplateTests, ListTemplates) {
    EXPECT_TRUE(Run<bool>(
        "fun reversed[T](xs: List[T]) -> List[T]\n"
        "    out: List[T] = []\n"
        "    for i = xs.size() - 1; i >= 0; i -= 1\n"
        "        out.push(xs[i])\n"
        "    return out\n"
        "fun count[T](xs: List[T], v: T) -> int\n"
        "    c = 0\n"
        "    for x in xs\n"
        "        if x == v\n"
        "            c += 1\n"
        "    return c\n"
        "r = reversed([1, 2, 3])\n"
        "w = reversed([\"a\", \"b\"])\n"
        "r[0] == 3 && w[0] == \"b\" && count([1, 2, 1, 1], 1) == 3 && count([\"x\", \"y\"], \"z\") == 0"));
}

TEST_F(SigmaTemplateTests, TwoTypeParametersAndFunctionArguments) {
    EXPECT_TRUE(Run<bool>(
        "fun mapList[A, B](f: fun(A) -> B, xs: List[A]) -> List[B]\n"
        "    out: List[B] = []\n"
        "    for x in xs\n"
        "        out.push(f(x))\n"
        "    return out\n"
        "fun isEven(n: int) -> bool\n"
        "    return n % 2 == 0\n"
        "fun len(s: str) -> int\n"
        "    return s.size()\n"
        "flags = mapList(isEven, [1, 2, 3, 4])\n"
        "sizes = mapList(len, [\"a\", \"bcd\"])\n"
        "!flags[0] && flags[1] && flags.size() == 4 && sizes[1] == 3"));
}

TEST_F(SigmaTemplateTests, TemplateCallsTemplate) {
    EXPECT_TRUE(Run<bool>(std::string(kMax) +
                          "fun max3[T](a: T, b: T, c: T) -> T\n"
                          "    return max(max(a, b), c)\n"
                          "max3(4, 9, 2) == 9 && max3(\"b\", \"a\", \"c\") == \"c\""));
}

TEST_F(SigmaTemplateTests, TemplateTailCall) {
    EXPECT_TRUE(Run<bool>(
        "fun nth[T](xs: List[T], n: int) -> T\n"
        "    if n == 0\n"
        "        return xs[0]\n"
        "    return nth(xs.slice(1, xs.size()), n - 1)!\n"
        "nth([10, 20, 30], 2) == 30 && nth([\"a\", \"b\"], 1) == \"b\""));
}

TEST_F(SigmaTemplateTests, SessionRemembersTemplates) {
    SigmaTranslator sigma(*reg_);
    ASSERT_TRUE(sigma.Compile("fun id[T](x: T) -> T\n    return x\n"));
    EXPECT_TRUE(sigma.Compile("y: str = id(\"a\")"));
    EXPECT_FALSE(sigma.Compile("z: int = id(\"a\")"));
}

// --- Errors ------------------------------------------------------------------

TEST_F(SigmaTemplateTests, InstantiationErrorNamesTheInstance) {
    ExpectError(std::string(kMax) + "m = max([1], [2])\n", "(in max[List[int]])");
    ExpectError(std::string(kMax) + "m = max([1], [2])\n", "operator '>' cannot be applied to List[int] and List[int]");
}

TEST_F(SigmaTemplateTests, ConflictingArguments) {
    ExpectError(std::string(kMax) + "m = max(1, \"a\")\n", "T is int from an earlier argument, got str");
}

TEST_F(SigmaTemplateTests, NoNumericWideningInsideAList) {
    ExpectError(
        "fun has[T](xs: List[T], v: T) -> bool\n"
        "    return xs.size() > 0\n"
        "b = has([1, 2], 2.5)\n",
        "T is int from an earlier argument, got float");
}

TEST_F(SigmaTemplateTests, NotAValue) {
    ExpectError(std::string(kMax) + "f = max\n", "template function 'max' can only be called");
}

TEST_F(SigmaTemplateTests, TypeParameterMustBeInferable) {
    ExpectError("fun make[T](n: int) -> List[T]\n    out: List[T] = []\n    return out\n",
                "type parameter 'T' of 'make' is not used by any parameter");
}

TEST_F(SigmaTemplateTests, BadTypeParameterNames) {
    ExpectError("fun f[int](x: int) -> int\n    return x\n", "'int' is a built-in type and cannot be a type parameter");
    ExpectError("fun f[T, T](x: T) -> T\n    return x\n", "duplicate type parameter 'T'");
}

TEST_F(SigmaTemplateTests, UncalledTemplateStillChecked) {
    ExpectError("fun f[T](x: T) -> T\n    y = nowhere\n    return x\n", "undefined name 'nowhere'");
}

TEST_F(SigmaTemplateTests, EachErrorReportedOnce) {
    const std::string all = Errors(
        "fun g[T](x: T) -> int\n"
        "    return x + 1\n"
        "a = g(\"s\")\n"
        "b = g(true)\n");
    size_t count = 0;
    for (size_t at = all.find("operator '+'"); at != std::string::npos; at = all.find("operator '+'", at + 1)) ++count;
    EXPECT_EQ(count, 1u) << all;
}

TEST_F(SigmaTemplateTests, WideningMustAgreeAcrossInstances) {
    ExpectError(
        "fun g[T](x: T) -> float\n"
        "    return x\n"
        "a = g(1)\n"
        "b = g(2.5)\n",
        "converts int to float here for some type arguments but not others");
}

TEST_F(SigmaTemplateTests, VoidArgument) {
    ExpectError("fun id[T](x: T) -> T\n    return x\ny = id(print(1))\n", "cannot pass a void value");
}

TEST_F(SigmaTemplateTests, WrongArgumentCount) {
    ExpectError(std::string(kMax) + "m = max(1)\n", "'max' expects 2 arguments, got 1");
}
