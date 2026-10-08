// 100 more tests of Sigma's continuation operators: '&' (suspend), '!'
// (replace, a tail call), and '...' (resume).
//
// Resume can't be used from Sigma: Rho's resume clears the context stack
// without calling the function, so `f(x)...` is rejected wherever it
// appears. The resume tests pin that down, and check that '...' in
// variadic templates (expansions, folds, packs) is not mistaken for it.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;

struct SigmaContinuationSuite : TestLangCommon {
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
        EXPECT_TRUE(top.IsType<T>()) << "wrong result type for:\n" << code << "\n--- rho:\n" << Rho(code);
        return top.IsType<T>() ? ConstDeref<T>(top) : T();
    }
};

// --- '&' (suspend) ---------------------------------------------------------

TEST_F(SigmaContinuationSuite, SuspendResultInArithmetic) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "tens(2)& * 3 + 1"), 61);
}

TEST_F(SigmaContinuationSuite, SuspendChained) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "tens(tens(1)&)&"), 100);
}

TEST_F(SigmaContinuationSuite, SuspendInListLiteral) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "xs = [tens(1)&, tens(2)&]\n"
                  "xs[1]"), 20);
}

TEST_F(SigmaContinuationSuite, SuspendInMapLiteral) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "m = {\"a\": tens(4)&}\n"
                  "m[\"a\"]"), 40);
}

TEST_F(SigmaContinuationSuite, SuspendInIfCondition) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "r = 0\n"
                  "if tens(1)& > 5\n"
                  "    r = 1\n"
                  "r"), 1);
}

TEST_F(SigmaContinuationSuite, SuspendInWhileCondition) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "i = 0\n"
                  "while tens(i)& < 50\n"
                  "    i += 1\n"
                  "i"), 5);
}

TEST_F(SigmaContinuationSuite, SuspendInForCondition) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "c = 0\n"
                  "for i = 0; i < tens(1)&; i += 1\n"
                  "    c += 1\n"
                  "c"), 10);
}

TEST_F(SigmaContinuationSuite, SuspendForEachIterable) {
    EXPECT_EQ(Run<int>("fun build(n: int) -> List[int]\n"
                  "    out: List[int] = []\n"
                  "    for i = 1; i <= n; i += 1\n"
                  "        out.push(i)\n"
                  "    return out\n"
                  "t = 0\n"
                  "for x in build(4)&\n"
                  "    t += x\n"
                  "t"), 10);
}

TEST_F(SigmaContinuationSuite, SuspendInTernaryBranches) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "b = false\n"
                  "b ? tens(1)& : tens(2)&"), 20);
}

TEST_F(SigmaContinuationSuite, SuspendInTernaryCondition) {
    EXPECT_EQ(Run<int>("fun isBig(n: int) -> bool\n"
                  "    return n > 3\n"
                  "isBig(5)& ? 1 : 2"), 1);
}

TEST_F(SigmaContinuationSuite, SuspendReturningStr) {
    EXPECT_TRUE(Run<bool>("fun greet(n: str) -> str\n"
                  "    return \"hi \" + n\n"
                  "greet(\"kai\")& == \"hi kai\""));
}

TEST_F(SigmaContinuationSuite, SuspendReturningFloat) {
    EXPECT_FLOAT_EQ(Run<float>("fun half(n: int) -> float\n"
                  "    return n / 2.0\n"
                  "half(5)&"), 2.5f);
}

TEST_F(SigmaContinuationSuite, SuspendReturningList) {
    EXPECT_EQ(Run<int>("fun pair(a: int, b: int) -> List[int]\n"
                  "    return [a, b]\n"
                  "p = pair(3, 4)&\n"
                  "p[0] + p[1]"), 7);
}

TEST_F(SigmaContinuationSuite, SuspendVoidFunction) {
    EXPECT_TRUE(Run<bool>("log: List[int] = []\n"
                  "fun note(n: int)\n"
                  "    log.push(n)\n"
                  "note(1)&\n"
                  "note(2)&\n"
                  "log.size() == 2 && log[1] == 2"));
}

TEST_F(SigmaContinuationSuite, SuspendInLoopInFunction) {
    EXPECT_EQ(Run<int>("fun sq(n: int) -> int\n"
                  "    return n * n\n"
                  "fun squares(n: int) -> List[int]\n"
                  "    out: List[int] = []\n"
                  "    for i = 1; i <= n; i += 1\n"
                  "        out.push(sq(i)&)\n"
                  "    return out\n"
                  "s = squares(4)\n"
                  "s[3]"), 16);
}

TEST_F(SigmaContinuationSuite, SuspendRecursiveSum) {
    EXPECT_EQ(Run<int>("fun sumTo(n: int) -> int\n"
                  "    return n == 0 ? 0 : n + sumTo(n - 1)&\n"
                  "sumTo(100)"), 5050);
}

TEST_F(SigmaContinuationSuite, SuspendRecursivePower) {
    EXPECT_EQ(Run<int>("fun power(b: int, e: int) -> int\n"
                  "    if e == 0\n"
                  "        return 1\n"
                  "    return b * power(b, e - 1)&\n"
                  "power(2, 10)"), 1024);
}

TEST_F(SigmaContinuationSuite, SuspendHanoiMoves) {
    EXPECT_EQ(Run<int>("fun moves(n: int) -> int\n"
                  "    if n == 0\n"
                  "        return 0\n"
                  "    return moves(n - 1)& + 1 + moves(n - 1)&\n"
                  "moves(10)"), 1023);
}

TEST_F(SigmaContinuationSuite, SuspendBinomial) {
    EXPECT_EQ(Run<int>("fun choose(n: int, k: int) -> int\n"
                  "    if k == 0 || k == n\n"
                  "        return 1\n"
                  "    return choose(n - 1, k - 1)& + choose(n - 1, k)&\n"
                  "choose(10, 3)"), 120);
}

TEST_F(SigmaContinuationSuite, SuspendNonTailGcd) {
    EXPECT_EQ(Run<int>("fun gcd(a: int, b: int) -> int\n"
                  "    if b == 0\n"
                  "        return a\n"
                  "    return gcd(b, a % b)&\n"
                  "gcd(84, 36)"), 12);
}

TEST_F(SigmaContinuationSuite, SuspendMutualCount) {
    EXPECT_EQ(Run<int>("fun f(n: int) -> int\n"
                  "    return n == 0 ? 0 : 1 + g(n - 1)&\n"
                  "fun g(n: int) -> int\n"
                  "    return n == 0 ? 0 : 1 + f(n - 1)&\n"
                  "f(9)"), 9);
}

TEST_F(SigmaContinuationSuite, SuspendFunctionVariable) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "g = tens\n"
                  "g(4)&"), 40);
}

TEST_F(SigmaContinuationSuite, SuspendFunctionReturnedFromCall) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun ones(n: int) -> int\n"
                  "    return n\n"
                  "fun chooseScale(big: bool) -> fun(int) -> int\n"
                  "    return big ? tens : ones\n"
                  "h = chooseScale(true)&\n"
                  "h(3)&"), 30);
}

TEST_F(SigmaContinuationSuite, SuspendTwice) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun twice(f: fun(int) -> int, x: int) -> int\n"
                  "    return f(f(x)&)&\n"
                  "twice(tens, 2)"), 200);
}

TEST_F(SigmaContinuationSuite, SuspendTemplate) {
    EXPECT_TRUE(Run<bool>("fun id[T](x: T) -> T\n"
                  "    return x\n"
                  "id(5)& == 5 && id(\"s\")& == \"s\""));
}

TEST_F(SigmaContinuationSuite, SuspendTemplateRecursion) {
    EXPECT_EQ(Run<int>("fun len[T](xs: List[T], i: int) -> int\n"
                  "    return i == xs.size() ? 0 : 1 + len(xs, i + 1)&\n"
                  "len([\"a\", \"b\", \"c\"], 0)"), 3);
}

TEST_F(SigmaContinuationSuite, SuspendVariadic) {
    EXPECT_EQ(Run<int>("fun sum[...Ts](xs: Ts...) -> int\n"
                  "    return (xs + ... + 0)\n"
                  "sum(1, 2, 3)&"), 6);
}

TEST_F(SigmaContinuationSuite, SuspendVariadicForward) {
    EXPECT_EQ(Run<int>("fun sum[...Ts](xs: Ts...) -> int\n"
                  "    return (xs + ... + 0)\n"
                  "fun twiceSum[...Ts](xs: Ts...) -> int\n"
                  "    return 2 * sum(xs...)&\n"
                  "twiceSum(1, 2, 3)"), 12);
}

TEST_F(SigmaContinuationSuite, SuspendPrecedence) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "tens(1)& + tens(2)& * tens(3)&"), 610);
}

TEST_F(SigmaContinuationSuite, SuspendUnaryMinus) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "x = -tens(2)&\n"
                  "x"), -20);
}

TEST_F(SigmaContinuationSuite, SuspendUnaryNot) {
    EXPECT_TRUE(Run<bool>("fun isBig(n: int) -> bool\n"
                  "    return n > 3\n"
                  "!isBig(1)&"));
}

TEST_F(SigmaContinuationSuite, SuspendCompoundAssign) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "x = 1\n"
                  "x += tens(1)&\n"
                  "x *= tens(1)&\n"
                  "x"), 110);
}

TEST_F(SigmaContinuationSuite, SuspendWidenedOnStore) {
    EXPECT_FLOAT_EQ(Run<float>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "y: float = tens(1)&\n"
                  "y / 4"), 2.5f);
}

TEST_F(SigmaContinuationSuite, SuspendWidenedArgument) {
    EXPECT_FLOAT_EQ(Run<float>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun halve(f: float) -> float\n"
                  "    return f / 2\n"
                  "halve(tens(1)&)"), 5.0f);
}

TEST_F(SigmaContinuationSuite, SuspendAsIndex) {
    EXPECT_EQ(Run<int>("fun idx(n: int) -> int\n"
                  "    return n + 1\n"
                  "xs = [10, 20, 30]\n"
                  "xs[idx(1)&]"), 30);
}

TEST_F(SigmaContinuationSuite, SuspendThenBitAnd) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "tens(3)& & 6"), 6);
}

TEST_F(SigmaContinuationSuite, SuspendGluedInRho) {
    const std::string rho = Rho("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "x = tens(3)&\n"
                  "");
    EXPECT_NE(rho.find("tens(3)&"), std::string::npos) << rho;
}

TEST_F(SigmaContinuationSuite, SpacedBitAndWrappedInRho) {
    const std::string rho = Rho("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "x = tens(3) & 6\n"
                  "");
    EXPECT_NE(rho.find("((tens(3)) & 6)"), std::string::npos) << rho;
}

TEST_F(SigmaContinuationSuite, SuspendOnMethodRejected) {
    ExpectError("xs = [1]\n"
                  "n = xs.size()&\n"
                  "",
                "applies to calls of Sigma functions");
}

TEST_F(SigmaContinuationSuite, SuspendOnPushRejected) {
    ExpectError("xs = [1]\n"
                  "xs.push(2)&\n"
                  "",
                "applies to calls of Sigma functions");
}

// --- '!' (replace) ---------------------------------------------------------

TEST_F(SigmaContinuationSuite, ReplaceForward) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(x: int) -> int\n"
                  "    return tens(x)!\n"
                  "f(7)"), 70);
}

TEST_F(SigmaContinuationSuite, ReplaceAfterLocals) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(x: int) -> int\n"
                  "    a = x + 1\n"
                  "    b = a * 2\n"
                  "    return tens(b)!\n"
                  "f(1)"), 40);
}

TEST_F(SigmaContinuationSuite, ReplaceCountdownDeep) {
    EXPECT_EQ(Run<int>("fun down(n: int) -> int\n"
                  "    if n == 0\n"
                  "        return 0\n"
                  "    return down(n - 1)!\n"
                  "down(30000)"), 0);
}

TEST_F(SigmaContinuationSuite, ReplaceAccumulateList) {
    EXPECT_TRUE(Run<bool>("fun build(n: int, acc: List[int]) -> List[int]\n"
                  "    if n == 0\n"
                  "        return acc\n"
                  "    acc.push(n)\n"
                  "    return build(n - 1, acc)!\n"
                  "none: List[int] = []\n"
                  "b = build(5, none)\n"
                  "b.size() == 5 && b[0] == 5 && b[4] == 1"));
}

TEST_F(SigmaContinuationSuite, ReplaceReverseList) {
    EXPECT_TRUE(Run<bool>("fun rev(xs: List[int], i: int, out: List[int]) -> List[int]\n"
                  "    if i < 0\n"
                  "        return out\n"
                  "    out.push(xs[i])\n"
                  "    return rev(xs, i - 1, out)!\n"
                  "none: List[int] = []\n"
                  "r = rev([1, 2, 3], 2, none)\n"
                  "r[0] == 3 && r[2] == 1"));
}

TEST_F(SigmaContinuationSuite, ReplacePowerAccumulator) {
    EXPECT_EQ(Run<int>("fun power(b: int, e: int, acc: int) -> int\n"
                  "    if e == 0\n"
                  "        return acc\n"
                  "    return power(b, e - 1, acc * b)!\n"
                  "power(3, 10, 1)"), 59049);
}

TEST_F(SigmaContinuationSuite, ReplaceFibonacciPair) {
    EXPECT_EQ(Run<int>("fun fib(n: int, a: int, b: int) -> int\n"
                  "    if n == 0\n"
                  "        return a\n"
                  "    return fib(n - 1, b, a + b)!\n"
                  "fib(30, 0, 1)"), 832040);
}

TEST_F(SigmaContinuationSuite, ReplaceDigitSum) {
    EXPECT_EQ(Run<int>("fun ds(n: int, acc: int) -> int\n"
                  "    if n == 0\n"
                  "        return acc\n"
                  "    return ds(n / 10, acc + n % 10)!\n"
                  "ds(98765, 0)"), 35);
}

TEST_F(SigmaContinuationSuite, ReplaceReverseNumber) {
    EXPECT_EQ(Run<int>("fun rev(n: int, acc: int) -> int\n"
                  "    if n == 0\n"
                  "        return acc\n"
                  "    return rev(n / 10, acc * 10 + n % 10)!\n"
                  "rev(12345, 0)"), 54321);
}

TEST_F(SigmaContinuationSuite, ReplacePalindromeNumber) {
    EXPECT_TRUE(Run<bool>("fun rev(n: int, acc: int) -> int\n"
                  "    if n == 0\n"
                  "        return acc\n"
                  "    return rev(n / 10, acc * 10 + n % 10)!\n"
                  "fun isPal(n: int) -> bool\n"
                  "    return n == rev(n, 0)\n"
                  "isPal(12321) && !isPal(12345)"));
}

TEST_F(SigmaContinuationSuite, ReplaceRepeatString) {
    EXPECT_TRUE(Run<bool>("fun rep(s: str, n: int, acc: str) -> str\n"
                  "    if n == 0\n"
                  "        return acc\n"
                  "    return rep(s, n - 1, acc + s)!\n"
                  "rep(\"ab\", 3, \"\") == \"ababab\""));
}

TEST_F(SigmaContinuationSuite, ReplaceBinaryString) {
    EXPECT_TRUE(Run<bool>("fun bin(n: int, acc: str) -> str\n"
                  "    if n == 0\n"
                  "        return acc == \"\" ? \"0\" : acc\n"
                  "    return bin(n / 2, (n % 2 == 0 ? \"0\" : \"1\") + acc)!\n"
                  "bin(10, \"\") == \"1010\" && bin(0, \"\") == \"0\""));
}

TEST_F(SigmaContinuationSuite, ReplaceReturningBool) {
    EXPECT_TRUE(Run<bool>("fun contains(xs: List[int], v: int, i: int) -> bool\n"
                  "    if i == xs.size()\n"
                  "        return false\n"
                  "    if xs[i] == v\n"
                  "        return true\n"
                  "    return contains(xs, v, i + 1)!\n"
                  "contains([4, 5, 6], 6, 0) && !contains([4, 5, 6], 7, 0)"));
}

TEST_F(SigmaContinuationSuite, ReplaceReturningFloat) {
    EXPECT_TRUE(Run<bool>("fun absf(x: float) -> float\n"
                  "    return x < 0 ? -x : x\n"
                  "fun root(x: float, g: float, n: int) -> float\n"
                  "    if n == 0\n"
                  "        return g\n"
                  "    return root(x, (g + x / g) / 2, n - 1)!\n"
                  "absf(root(2.0, 1.0, 20) - 1.4142135) < 0.0001"));
}

TEST_F(SigmaContinuationSuite, ReplaceVoidChain) {
    EXPECT_TRUE(Run<bool>("log: List[str] = []\n"
                  "fun a()\n"
                  "    log.push(\"a\")\n"
                  "    b()!\n"
                  "fun b()\n"
                  "    log.push(\"b\")\n"
                  "    c()!\n"
                  "fun c()\n"
                  "    log.push(\"c\")\n"
                  "a()\n"
                  "log.size() == 3 && log[0] == \"a\" && log[2] == \"c\""));
}

TEST_F(SigmaContinuationSuite, ReplaceVoidDeep) {
    EXPECT_TRUE(Run<bool>("hits: List[int] = []\n"
                  "fun walk(n: int)\n"
                  "    if n == 0\n"
                  "        return\n"
                  "    if n % 1000 == 0\n"
                  "        hits.push(n)\n"
                  "    walk(n - 1)!\n"
                  "walk(5000)\n"
                  "hits.size() == 5 && hits[0] == 5000"));
}

TEST_F(SigmaContinuationSuite, ReplaceMutualDeep) {
    EXPECT_TRUE(Run<bool>("fun isEven(n: int) -> bool\n"
                  "    if n == 0\n"
                  "        return true\n"
                  "    return isOdd(n - 1)!\n"
                  "fun isOdd(n: int) -> bool\n"
                  "    if n == 0\n"
                  "        return false\n"
                  "    return isEven(n - 1)!\n"
                  "isEven(20000) && isOdd(20001) && !isOdd(20000)"));
}

TEST_F(SigmaContinuationSuite, ReplaceThreeWayCycle) {
    EXPECT_TRUE(Run<bool>("fun a(n: int) -> int\n"
                  "    if n == 0\n"
                  "        return 1\n"
                  "    return b(n - 1)!\n"
                  "fun b(n: int) -> int\n"
                  "    if n == 0\n"
                  "        return 2\n"
                  "    return c(n - 1)!\n"
                  "fun c(n: int) -> int\n"
                  "    if n == 0\n"
                  "        return 3\n"
                  "    return a(n - 1)!\n"
                  "a(0) == 1 && a(1) == 2 && a(5) == 3 && a(3000) == 1"));
}

TEST_F(SigmaContinuationSuite, ReplaceThroughParameter) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun apply(f: fun(int) -> int, x: int) -> int\n"
                  "    return f(x)!\n"
                  "apply(tens, 6)"), 60);
}

TEST_F(SigmaContinuationSuite, ReplaceThroughLocalVariable) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun ones(n: int) -> int\n"
                  "    return n\n"
                  "fun scaled(big: bool, x: int) -> int\n"
                  "    g = big ? tens : ones\n"
                  "    return g(x)!\n"
                  "scaled(true, 3) + scaled(false, 3)"), 33);
}

TEST_F(SigmaContinuationSuite, ReplaceCollatzPair) {
    EXPECT_EQ(Run<int>("fun evenStep(n: int, k: int) -> int\n"
                  "    if n == 1\n"
                  "        return k\n"
                  "    next = n % 2 == 0 ? n / 2 : 3 * n + 1\n"
                  "    return oddStep(next, k + 1)!\n"
                  "fun oddStep(n: int, k: int) -> int\n"
                  "    if n == 1\n"
                  "        return k\n"
                  "    next = n % 2 == 0 ? n / 2 : 3 * n + 1\n"
                  "    return evenStep(next, k + 1)!\n"
                  "evenStep(27, 0)"), 111);
}

TEST_F(SigmaContinuationSuite, ReplaceTemplateForward) {
    EXPECT_TRUE(Run<bool>("fun id[T](x: T) -> T\n"
                  "    return x\n"
                  "fun wrap[T](x: T) -> T\n"
                  "    return id(x)!\n"
                  "wrap(4) == 4 && wrap(\"w\") == \"w\""));
}

TEST_F(SigmaContinuationSuite, ReplaceTemplateAccumulator) {
    EXPECT_EQ(Run<int>("fun total[T](xs: List[T], i: int, acc: T) -> T\n"
                  "    if i == xs.size()\n"
                  "        return acc\n"
                  "    return total(xs, i + 1, acc + xs[i])!\n"
                  "total([1, 2, 3, 4], 0, 0)"), 10);
}

TEST_F(SigmaContinuationSuite, ReplaceTemplateStrings) {
    EXPECT_TRUE(Run<bool>("fun total[T](xs: List[T], i: int, acc: T) -> T\n"
                  "    if i == xs.size()\n"
                  "        return acc\n"
                  "    return total(xs, i + 1, acc + xs[i])!\n"
                  "total([\"k\", \"a\", \"i\"], 0, \"\") == \"kai\""));
}

TEST_F(SigmaContinuationSuite, ReplaceIntoVariadic) {
    EXPECT_EQ(Run<int>("fun sum[...Ts](xs: Ts...) -> int\n"
                  "    return (xs + ... + 0)\n"
                  "fun three(a: int, b: int, c: int) -> int\n"
                  "    return sum(a, b, c)!\n"
                  "three(1, 2, 3)"), 6);
}

TEST_F(SigmaContinuationSuite, ReplaceVariadicHelper) {
    EXPECT_EQ(Run<int>("fun countFrom(n: int) -> int\n"
                  "    return n\n"
                  "fun count[...Ts](xs: Ts...) -> int\n"
                  "    return countFrom(xs.size())!\n"
                  "count(1, \"a\", 2.5)"), 3);
}

TEST_F(SigmaContinuationSuite, ReplaceAfterSeveralEarlyReturns) {
    EXPECT_TRUE(Run<bool>("fun classify(n: int) -> int\n"
                  "    if n < 0\n"
                  "        return -1\n"
                  "    if n == 0\n"
                  "        return 0\n"
                  "    if n > 100\n"
                  "        return 100\n"
                  "    return classify(n - 1)!\n"
                  "classify(-5) == -1 && classify(5) == 0 && classify(500) == 100"));
}

TEST_F(SigmaContinuationSuite, ReplaceAfterSuspend) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(x: int) -> int\n"
                  "    y = tens(x)&\n"
                  "    return tens(y)!\n"
                  "f(2)"), 200);
}

TEST_F(SigmaContinuationSuite, ReplaceOnlyReturn) {
    EXPECT_EQ(Run<int>("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(n: int) -> int\n"
                  "    return tens(n)!\n"
                  "fun g(n: int) -> int\n"
                  "    return f(n)!\n"
                  "g(9)"), 90);
}

TEST_F(SigmaContinuationSuite, ReplaceGluedInRho) {
    const std::string rho = Rho("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(n: int) -> int\n"
                  "    return tens(n)!\n"
                  "");
    EXPECT_NE(rho.find("return tens(n)!"), std::string::npos) << rho;
}

TEST_F(SigmaContinuationSuite, VoidReplaceGluedInRho) {
    const std::string rho = Rho("fun countdown(n: int)\n"
                  "    if n == 0\n"
                  "        return\n"
                  "    countdown(n - 1)!\n"
                  "");
    EXPECT_NE(rho.find("countdown((n - 1))!"), std::string::npos) << rho;
}

TEST_F(SigmaContinuationSuite, ReplaceInElseRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(n: int) -> int\n"
                  "    if n > 0\n"
                  "        return 1\n"
                  "    else\n"
                  "        return tens(n)!\n"
                  "",
                "must be the last statement of a function body");
}

TEST_F(SigmaContinuationSuite, ReplaceInForEachRejected) {
    ExpectError("fun note(n: int)\n"
                  "    print(n)\n"
                  "fun each(xs: List[int])\n"
                  "    for x in xs\n"
                  "        note(x)!\n"
                  "",
                "must be the last statement of a function body");
}

TEST_F(SigmaContinuationSuite, ReplaceInDoWhileRejected) {
    ExpectError("fun note(n: int)\n"
                  "    print(n)\n"
                  "fun f(n: int)\n"
                  "    do\n"
                  "        note(n)!\n"
                  "    while false\n"
                  "",
                "must be the last statement of a function body");
}

TEST_F(SigmaContinuationSuite, ReplaceInTernaryRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(n: int) -> int\n"
                  "    return n > 0 ? tens(n)! : 0\n"
                  "",
                "must be the last statement of a function body");
}

TEST_F(SigmaContinuationSuite, ReplaceInArgumentRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(n: int) -> int\n"
                  "    return tens(tens(n)!)\n"
                  "",
                "must be the last statement of a function body");
}

TEST_F(SigmaContinuationSuite, ReplaceNotLastRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(n: int) -> int\n"
                  "    x = tens(n)!\n"
                  "    return x\n"
                  "",
                "must be the last statement of a function body");
}

TEST_F(SigmaContinuationSuite, ReplaceInTopLevelLoopRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "for i = 0; i < 2; i += 1\n"
                  "    x = tens(i)!\n"
                  "",
                "can only be used inside a function");
}

TEST_F(SigmaContinuationSuite, ReplaceResultTypeMismatch) {
    ExpectError("fun name(n: int) -> str\n"
                  "    return \"n\"\n"
                  "fun f(n: int) -> int\n"
                  "    return name(n)!\n"
                  "",
                "must return exactly int, got str");
}

TEST_F(SigmaContinuationSuite, ReplaceTemplateNotWidened) {
    ExpectError("fun id[T](x: T) -> T\n"
                  "    return x\n"
                  "fun f() -> float\n"
                  "    return id(1)!\n"
                  "",
                "must return exactly float, got int");
}

TEST_F(SigmaContinuationSuite, VoidReplaceOfValueRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(n: int)\n"
                  "    tens(n)!\n"
                  "",
                "must call a void function, got int");
}

TEST_F(SigmaContinuationSuite, ReplaceOfVoidInValueFunctionRejected) {
    ExpectError("fun note(n: int)\n"
                  "    print(n)\n"
                  "fun f(n: int) -> int\n"
                  "    return note(n)!\n"
                  "",
                "void");
}

TEST_F(SigmaContinuationSuite, ReplaceOnMethodRejected) {
    ExpectError("fun f(xs: List[int]) -> int\n"
                  "    return xs.size()!\n"
                  "",
                "applies to calls of Sigma functions");
}

TEST_F(SigmaContinuationSuite, ReplaceOnPrintRejected) {
    ExpectError("fun f(n: int)\n"
                  "    print(n)!\n"
                  "",
                "applies to calls of Sigma functions");
}

TEST_F(SigmaContinuationSuite, SpacedBangInVoidRejected) {
    ExpectError("fun note(n: int)\n"
                  "    print(n)\n"
                  "fun f(n: int)\n"
                  "    note(n) !\n"
                  "",
                "expected end of statement");
}

// --- '...' (resume) --------------------------------------------------------

TEST_F(SigmaContinuationSuite, ResumeInReturnRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(n: int) -> int\n"
                  "    return tens(n)...\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeAtTopLevelRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "tens(1)...\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeInAssignmentRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "x = tens(1)...\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeInArgumentRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "y = tens(tens(1)...)\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeOnFunctionValueRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "g = tens\n"
                  "g(1)...\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeOnTemplateCallRejected) {
    ExpectError("fun id[T](x: T) -> T\n"
                  "    return x\n"
                  "y = id(1)...\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeOnVariadicCallRejected) {
    ExpectError("fun sum[...Ts](xs: Ts...) -> int\n"
                  "    return (xs + ... + 0)\n"
                  "y = sum(1, 2)...\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeAfterSpaceRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "x = tens(1) ...\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeInTernaryRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "x = true ? tens(1)... : 0\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeInConditionRejected) {
    ExpectError("fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "if tens(1)... > 3\n"
                  "    print(1)\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, ResumeOnMethodRejected) {
    ExpectError("xs = [1]\n"
                  "n = xs.size()...\n"
                  "",
                "'...' (resume) is not supported");
}

TEST_F(SigmaContinuationSuite, TwoDotsRejected) {
    ExpectError("x = 1 .. 2\n"
                  "",
                "'..' is not an operator");
}

TEST_F(SigmaContinuationSuite, EllipsisExpansionIsNotResume) {
    EXPECT_EQ(Errors("fun add(a: int, b: int) -> int\n"
                  "    return a + b\n"
                  "fun f[...Ts](xs: Ts...) -> int\n"
                  "    return add(xs...)\n"
                  "y = f(1, 2)\n"
                  ""), "");
}

TEST_F(SigmaContinuationSuite, EllipsisFoldIsNotResume) {
    EXPECT_TRUE(Run<bool>("fun all[...Ts](xs: Ts...) -> bool\n"
                  "    return (xs && ...)\n"
                  "all(true, true) && all()"));
}

TEST_F(SigmaContinuationSuite, EllipsisInTypeParameterIsNotResume) {
    EXPECT_EQ(Errors("fun count[...Ts](xs: Ts...) -> int\n"
                  "    return xs.size()\n"
                  "n = count(1, 2)\n"
                  ""), "");
}
