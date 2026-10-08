// Variadic templates: `fun f[T, ...Ts](x: T, xs: Ts...)`. A pack is expanded
// (`g(xs...)`, `[xs...]`), folded (`(xs + ...)`, `(... + xs)`, with or without
// an initial value) or counted (`xs.size()`). A condition on a pack's length
// selects its branch per instantiation, like C++'s `if constexpr`, so head/tail
// recursion terminates. Each pack length is emitted as its own Rho function.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;

namespace {
const char *const kSum =
    "fun sum[...Ts](xs: Ts...) -> int\n"
    "    return (xs + ... + 0)\n";
}

struct SigmaVariadicTests : TestLangCommon {
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

// --- Folds -------------------------------------------------------------------

TEST_F(SigmaVariadicTests, FoldWithInitialValue) {
    EXPECT_TRUE(Run<bool>(std::string(kSum) + "sum(1, 2, 3) == 6 && sum(5) == 5 && sum() == 0"));
}

TEST_F(SigmaVariadicTests, LeftFoldOverStrings) {
    EXPECT_TRUE(Run<bool>(
        "fun concat[...Ts](xs: Ts...) -> str\n"
        "    return (\"<\" + ... + xs)\n"
        "concat(\"a\", \"b\", \"c\") == \"<abc\" && concat() == \"<\""));
}

TEST_F(SigmaVariadicTests, FoldDirectionMatters) {
    // (xs - ...) is 10 - (4 - 3); (... - xs) is (10 - 4) - 3.
    EXPECT_TRUE(Run<bool>(
        "fun fromRight[...Ts](xs: Ts...) -> int\n"
        "    return (xs - ...)\n"
        "fun fromLeft[...Ts](xs: Ts...) -> int\n"
        "    return (... - xs)\n"
        "fromRight(10, 4, 3) == 9 && fromLeft(10, 4, 3) == 3"));
}

TEST_F(SigmaVariadicTests, LogicalFoldsOfEmptyPacks) {
    EXPECT_TRUE(Run<bool>(
        "fun allOf[...Ts](xs: Ts...) -> bool\n"
        "    return (xs && ...)\n"
        "fun anyOf[...Ts](xs: Ts...) -> bool\n"
        "    return (... || xs)\n"
        "allOf() && !anyOf() && allOf(true, true) && !allOf(true, false) && anyOf(false, true)"));
}

TEST_F(SigmaVariadicTests, MixedNumericFold) {
    EXPECT_FLOAT_EQ(Run<float>(
                        "fun total[...Ts](xs: Ts...) -> float\n"
                        "    return (xs + ... + 0.0)\n"
                        "total(1, 2.5, 3)"),
                    6.5f);
}

// --- Size and expansion --------------------------------------------------------

TEST_F(SigmaVariadicTests, SizeOfAHeterogeneousPack) {
    EXPECT_TRUE(Run<bool>(
        "fun count[...Ts](xs: Ts...) -> int\n"
        "    return xs.size()\n"
        "count() == 0 && count(1, \"a\", 2.5, [true]) == 4"));
}

TEST_F(SigmaVariadicTests, ExpandIntoAList) {
    EXPECT_TRUE(Run<bool>(
        "fun listOf[T, ...Ts](first: T, rest: Ts...) -> List[T]\n"
        "    return [first, rest...]\n"
        "xs = listOf(7, 8, 9)\n"
        "ws = listOf(\"only\")\n"
        "xs.size() == 3 && xs[2] == 9 && ws.size() == 1"));
}

TEST_F(SigmaVariadicTests, ExpandIntoAnOrdinaryFunction) {
    EXPECT_TRUE(Run<bool>(
        "fun add3(a: int, b: int, c: int) -> int\n"
        "    return a + b + c\n"
        "fun apply[...Ts](xs: Ts...) -> int\n"
        "    return add3(xs...)\n"
        "apply(1, 2, 3) == 6"));
}

TEST_F(SigmaVariadicTests, ExpandIntoATemplate) {
    EXPECT_TRUE(Run<bool>(
        "fun max[T](a: T, b: T) -> T\n"
        "    return a > b ? a : b\n"
        "fun maxOfPair[...Ts](xs: Ts...) -> str\n"
        "    return max(xs...)\n"
        "maxOfPair(\"pear\", \"apple\") == \"pear\""));
}

TEST_F(SigmaVariadicTests, ForwardToAnotherVariadic) {
    EXPECT_TRUE(Run<bool>(std::string(kSum) +
                          "fun twiceSum[...Ts](xs: Ts...) -> int\n"
                          "    return sum(xs...) * 2\n"
                          "twiceSum(1, 2, 3, 4) == 20 && twiceSum() == 0"));
}

// --- Compile-time branches and recursion ----------------------------------------

TEST_F(SigmaVariadicTests, HeadTailRecursionWithStaticIf) {
    EXPECT_TRUE(Run<bool>(
        "fun maxOf[T, ...Ts](first: T, rest: Ts...) -> T\n"
        "    if rest.size() == 0\n"
        "        return first\n"
        "    else\n"
        "        m = maxOf(rest...)\n"
        "        return first > m ? first : m\n"
        "maxOf(3, 9, 2) == 9 && maxOf(\"b\", \"c\", \"a\") == \"c\" && maxOf(4) == 4"));
}

TEST_F(SigmaVariadicTests, RecursionWithStaticConditional) {
    EXPECT_TRUE(Run<bool>(
        "fun addAll[T, ...Ts](first: T, rest: Ts...) -> T\n"
        "    return rest.size() == 0 ? first : first + addAll(rest...)\n"
        "addAll(1, 2, 3, 4) == 10 && addAll(\"a\", \"b\") == \"ab\""));
}

// --- Generated Rho and sessions --------------------------------------------------

TEST_F(SigmaVariadicTests, OneRhoFunctionPerPackLength) {
    SigmaTranslator sigma(*reg_);
    ASSERT_TRUE(sigma.Compile((std::string(kSum) + "a = sum(1, 2)\nb = sum(3, 4)\nc = sum()\n").c_str()));
    const std::string &rho = sigma.GetRho();
    EXPECT_NE(rho.find("fun sum__2(xs__0, xs__1)"), std::string::npos) << rho;
    EXPECT_NE(rho.find("fun sum__0()"), std::string::npos) << rho;
    EXPECT_NE(rho.find("sum__2(1, 2)"), std::string::npos) << rho;
    // Two calls of length 2 share one function.
    EXPECT_EQ(rho.find("fun sum__2"), rho.rfind("fun sum__2")) << rho;
}

TEST_F(SigmaVariadicTests, SessionInstantiatesEarlierTemplates) {
    SigmaTranslator sigma(*reg_);
    ASSERT_TRUE(sigma.Compile(kSum));
    ASSERT_TRUE(sigma.Compile("t = sum(1, 2, 3)")) << sigma.GetErrors().front();
    EXPECT_NE(sigma.GetRho().find("fun sum__3(xs__0, xs__1, xs__2)"), std::string::npos) << sigma.GetRho();
}

// --- Errors ----------------------------------------------------------------------

TEST_F(SigmaVariadicTests, PackPlacement) {
    ExpectError("fun f[...Ts, T](x: T, xs: Ts...) -> int\n    return 0\n", "must be the last type parameter");
    ExpectError("fun f[...Ts](xs: Ts..., n: int) -> int\n    return n\n", "pack parameter 'xs' must be the last parameter");
    ExpectError("fun f(xs: int...) -> int\n    return 0\n", "needs a type parameter pack");
}

TEST_F(SigmaVariadicTests, PackCannotBeUsedBare) {
    ExpectError(
        "fun f[...Ts](xs: Ts...) -> int\n"
        "    y = xs\n"
        "    return 0\n"
        "r = f(1)\n",
        "'xs' is a parameter pack");
}

TEST_F(SigmaVariadicTests, EmptyFoldNeedsAnInitialValue) {
    ExpectError(
        "fun s[...Ts](xs: Ts...) -> int\n"
        "    return (xs + ...)\n"
        "r = s()\n",
        "fold over an empty pack");
}

TEST_F(SigmaVariadicTests, OnlyPacksExpand) {
    ExpectError(
        "fun g(a: int) -> int\n"
        "    return a\n"
        "fun f(n: int) -> int\n"
        "    return g(n...)\n",
        "'...' expands a parameter pack, and 'n' is not one");
}

TEST_F(SigmaVariadicTests, PackElementsAreNotWidened) {
    ExpectError(
        "fun half(a: float, b: float) -> float\n"
        "    return (a + b) / 2\n"
        "fun g[...Ts](xs: Ts...) -> float\n"
        "    return half(xs...)\n"
        "r = g(1, 2)\n",
        "pack elements are not widened");
}

TEST_F(SigmaVariadicTests, FoldNeedsExactlyOnePack) {
    ExpectError(
        "fun f[...Ts](xs: Ts...) -> int\n"
        "    return (1 + ... + 2)\n"
        "r = f(1)\n",
        "a fold expression needs exactly one parameter pack");
}
