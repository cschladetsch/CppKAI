// 50 more Sigma tests: 32 that compile and run programs end to end through
// Rho and Pi, and 18 that check static errors.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "TestLangCommon.h"

using namespace kai;

struct SigmaMoreTests : TestLangCommon {
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

// --- Expressions -------------------------------------------------------------

TEST_F(SigmaMoreTests, MultiplicationBindsTighterThanAddition) {
    EXPECT_EQ(Run<int>("r = 2 + 3 * 4\nr"), 14);
}

TEST_F(SigmaMoreTests, ParenthesesOverridePrecedence) {
    EXPECT_EQ(Run<int>("r = (2 + 3) * 4\nr"), 20);
}

TEST_F(SigmaMoreTests, IntegerDivisionTruncates) {
    EXPECT_EQ(Run<int>("r = 7 / 2\nr"), 3);
}

TEST_F(SigmaMoreTests, Modulo) {
    EXPECT_EQ(Run<int>("r = 17 % 5\nr"), 2);
}

TEST_F(SigmaMoreTests, DivisionByFloatGivesFloat) {
    EXPECT_FLOAT_EQ(Run<float>("r = 7 / 2.0\nr"), 3.5f);
}

TEST_F(SigmaMoreTests, StringConcatenationChain) {
    EXPECT_EQ(Run<String>("s = \"a\" + \"b\" + \"c\"\ns"), String("abc"));
}

TEST_F(SigmaMoreTests, StringOrdering) {
    EXPECT_TRUE(Run<bool>("b = \"abc\" < \"abd\"\nb"));
}

TEST_F(SigmaMoreTests, BooleanLogic) {
    EXPECT_TRUE(Run<bool>("t = true\nf = false\nr = (t || f) && !f\nr"));
}

TEST_F(SigmaMoreTests, NestedTernary) {
    EXPECT_EQ(Run<int>("n = 5\nr = n > 3 ? (n > 4 ? 2 : 1) : 0\nr"), 2);
}

TEST_F(SigmaMoreTests, DoubleNegation) {
    EXPECT_EQ(Run<int>("a = -(-3)\na"), 3);
}

TEST_F(SigmaMoreTests, NegativeOperandInsideExpression) {
    EXPECT_EQ(Run<int>("a = 10 - -4 * 2\na"), 18);
}

TEST_F(SigmaMoreTests, CompoundAssignmentChain) {
    EXPECT_EQ(Run<int>("x = 10\nx -= 3\nx *= 2\nx /= 7\nx %= 3\nx"), 2);
}

TEST_F(SigmaMoreTests, StringCompoundAssignment) {
    EXPECT_EQ(Run<String>("s = \"a\"\ns += \"b\"\ns += \"c\"\ns"), String("abc"));
}

// --- Control flow ------------------------------------------------------------

TEST_F(SigmaMoreTests, WhileSumsOneToTen) {
    EXPECT_EQ(Run<int>("i = 1\nt = 0\nwhile i <= 10\n    t += i\n    i += 1\nt"), 55);
}

TEST_F(SigmaMoreTests, ForLoopFactorial) {
    EXPECT_EQ(Run<int>("p = 1\nfor i = 1; i <= 5; i += 1\n    p *= i\np"), 120);
}

TEST_F(SigmaMoreTests, DoWhileRunsAtLeastOnce) {
    EXPECT_EQ(Run<int>("i = 10\nc = 0\ndo\n    c += 1\nwhile i < 5\nc"), 1);
}

TEST_F(SigmaMoreTests, ForEachOverStrings) {
    EXPECT_EQ(Run<String>("s = \"\"\nfor w in [\"a\", \"b\", \"c\"]\n    s += w\ns"), String("abc"));
}

TEST_F(SigmaMoreTests, NestedLoops) {
    EXPECT_EQ(Run<int>(
                  "c = 0\n"
                  "for i = 0; i < 3; i += 1\n"
                  "    for j = 0; j < 4; j += 1\n"
                  "        c += 1\n"
                  "c"),
              12);
}

TEST_F(SigmaMoreTests, BreakOutOfInfiniteWhile) {
    EXPECT_EQ(Run<int>("i = 0\nwhile true\n    i += 1\n    if i == 7\n        break\ni"), 7);
}

TEST_F(SigmaMoreTests, ContinueSkipsEvenNumbers) {
    EXPECT_EQ(Run<int>(
                  "t = 0\n"
                  "for i = 0; i < 10; i += 1\n"
                  "    if i % 2 == 0\n"
                  "        continue\n"
                  "    t += i\n"
                  "t"),
              25);
}

TEST_F(SigmaMoreTests, ElseIfChain) {
    const std::string grade =
        "fun grade(n: int) -> str\n"
        "    if n >= 90\n"
        "        return \"A\"\n"
        "    else if n >= 75\n"
        "        return \"B\"\n"
        "    else\n"
        "        return \"C\"\n";
    EXPECT_EQ(Run<String>(grade + "grade(80)"), String("B"));
}

// --- Functions ---------------------------------------------------------------

TEST_F(SigmaMoreTests, FunctionsCallFunctions) {
    EXPECT_EQ(Run<int>(
                  "fun inc(n: int) -> int\n    return n + 1\n"
                  "fun twice(n: int) -> int\n    return n * 2\n"
                  "twice(inc(3))"),
              8);
}

TEST_F(SigmaMoreTests, RecursiveSum) {
    EXPECT_EQ(Run<int>("fun sum(n: int) -> int\n    return n == 0 ? 0 : n + sum(n - 1)\nsum(10)"), 55);
}

TEST_F(SigmaMoreTests, RecursiveGcd) {
    EXPECT_EQ(Run<int>("fun gcd(a: int, b: int) -> int\n    return b == 0 ? a : gcd(b, a % b)\ngcd(48, 18)"), 6);
}

TEST_F(SigmaMoreTests, LoopInsideFunction) {
    EXPECT_EQ(Run<int>(
                  "fun power(b: int, e: int) -> int\n"
                  "    r = 1\n"
                  "    for i = 0; i < e; i += 1\n"
                  "        r *= b\n"
                  "    return r\n"
                  "power(2, 10)"),
              1024);
}

TEST_F(SigmaMoreTests, ListParameter) {
    EXPECT_EQ(Run<int>(
                  "fun total(xs: List[int]) -> int\n"
                  "    t = 0\n"
                  "    for x in xs\n"
                  "        t += x\n"
                  "    return t\n"
                  "total([1, 2, 3, 4])"),
              10);
}

TEST_F(SigmaMoreTests, FunctionReturningList) {
    EXPECT_EQ(Run<int>(
                  "fun upTo(n: int) -> List[int]\n"
                  "    xs: List[int] = []\n"
                  "    for i = 0; i < n; i += 1\n"
                  "        xs.push(i)\n"
                  "    return xs\n"
                  "ys = upTo(5)\n"
                  "ys.size()"),
              5);
}

TEST_F(SigmaMoreTests, FunctionPassedAsArgument) {
    EXPECT_EQ(Run<int>(
                  "fun inc(n: int) -> int\n    return n + 1\n"
                  "fun apply(f: fun(int) -> int, x: int) -> int\n    return f(x)\n"
                  "apply(inc, 5)"),
              6);
}

TEST_F(SigmaMoreTests, FunctionUsesGlobalDeclaredLater) {
    // Bodies are checked after the top level, as Rho resolves names at call time.
    EXPECT_EQ(Run<int>("fun f() -> int\n    return k * 2\nk = 21\nf()"), 42);
}

// --- Containers and widening --------------------------------------------------

TEST_F(SigmaMoreTests, NestedListIndexing) {
    EXPECT_EQ(Run<int>("g = [[1, 2], [3, 4]]\ng[1][0]"), 3);
}

TEST_F(SigmaMoreTests, ChainedMethodCalls) {
    EXPECT_EQ(Run<int>("xs = [1, 2, 3, 4]\nxs.slice(1, 3).size()"), 2);
}

TEST_F(SigmaMoreTests, FloatArgumentWidenedAtCall) {
    EXPECT_FLOAT_EQ(Run<float>("fun half(x: float) -> float\n    return x / 2\nhalf(3)"), 1.5f);
}

// --- Static errors ------------------------------------------------------------

TEST_F(SigmaMoreTests, MethodOnInt) {
    ExpectError("n = 1\nn.size()", "int has no method 'size'");
}

TEST_F(SigmaMoreTests, MapValueTypeOnStore) {
    ExpectError("m = {\"a\": 1}\nm[\"a\"] = \"x\"", "cannot store value: expected int, got str");
}

TEST_F(SigmaMoreTests, LoopVariableTypeFlowsIntoBody) {
    ExpectError("for s in [\"a\"]\n    n: int = s\n", "cannot initialise 'n': expected int, got str");
}

TEST_F(SigmaMoreTests, ContinueOutsideLoop) {
    ExpectError("continue", "'continue' outside a loop");
}

TEST_F(SigmaMoreTests, DuplicateParameter) {
    ExpectError("fun f(a: int, a: int) -> int\n    return a\n", "duplicate parameter 'a'");
}

TEST_F(SigmaMoreTests, FunctionInsideIf) {
    ExpectError("if true\n    fun f()\n        print(1)\n", "functions must be defined at the top level");
}

TEST_F(SigmaMoreTests, VoidFunctionReturningValue) {
    ExpectError("fun f() -> void\n    return 1\n", "'f' does not return a value");
}

TEST_F(SigmaMoreTests, VoidVariable) {
    ExpectError("x: void = 1", "'void' is only allowed as a function result");
}

TEST_F(SigmaMoreTests, ListWithoutElementType) {
    ExpectError("xs: List = [1]", "'List' takes 1 type argument");
}

TEST_F(SigmaMoreTests, NonStringMapKey) {
    ExpectError("m = {1: 2}", "map keys must be string literals");
}

TEST_F(SigmaMoreTests, ListComparedWithInt) {
    ExpectError("b = [1] == 1", "operator '==' cannot be applied to List[int] and int");
}

TEST_F(SigmaMoreTests, ModuloOnString) {
    ExpectError("r = \"a\" % 2", "operator '%' cannot be applied to str and int");
}

TEST_F(SigmaMoreTests, BitNotOnFloat) {
    ExpectError("r = ~1.5", "operator '~' cannot be applied to float");
}

TEST_F(SigmaMoreTests, TernaryBranchesDisagree) {
    ExpectError("x = true ? 1 : \"a\"", "branches of a conditional expression have different types: int and str");
}

TEST_F(SigmaMoreTests, CompoundOnUndeclared) {
    ExpectError("y += 1", "'y' is not declared");
}

TEST_F(SigmaMoreTests, UndefinedFunction) {
    ExpectError("foo(1)", "undefined name 'foo'");
}

TEST_F(SigmaMoreTests, BuiltinMethodArity) {
    ExpectError("xs = [1]\nn = xs.size(1)", "'size' expects 0 arguments, got 1");
}

TEST_F(SigmaMoreTests, LexicalErrors) {
    ExpectError("if true\n  x = 1\n", "indentation must be a tab or a multiple of four spaces");
}
