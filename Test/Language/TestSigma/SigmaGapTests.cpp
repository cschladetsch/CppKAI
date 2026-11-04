// 50 Sigma tests for behaviour the other suites do not pin down: statement
// syntax (semicolons, comments, tabs, CRLF, assert), the bitwise and shift
// operators, widening inside containers, scope and reference semantics, string
// handling, the SigmaTranslator API (determinism, sessions, error format), and
// the exact examples printed in Doc/Sigma/README.md.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace kai;

struct SigmaGapTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Sigma);
    }

    static std::string Join(const std::vector<std::string> &lines) {
        std::string all;
        for (auto const &l : lines) all += l + "\n";
        return all;
    }

    std::vector<std::string> ErrorList(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        sigma.Compile(code.c_str());
        return sigma.GetErrors();
    }

    std::string Errors(const std::string &code) { return Join(ErrorList(code)); }

    std::string Rho(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        EXPECT_TRUE(sigma.Compile(code.c_str())) << Join(sigma.GetErrors());
        return sigma.GetRho();
    }

    void ExpectOk(const std::string &code) {
        const std::string all = Errors(code);
        EXPECT_TRUE(all.empty()) << code << "\n--- errors:\n" << all;
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

// --- Statement syntax ----------------------------------------------------------

TEST_F(SigmaGapTests, SemicolonsSeparateStatements) {
    EXPECT_EQ(Run<int>("a = 1; b = 2; c = a + b\nc"), 3);
}

TEST_F(SigmaGapTests, CommentsAreIgnored) {
    EXPECT_EQ(Run<int>("x = 1 // trailing\n// a whole line\ny = x + 1 // another\ny"), 2);
}

TEST_F(SigmaGapTests, TabIndentationWorksLikeFourSpaces) {
    EXPECT_EQ(Run<int>("t = 0\nfor x in [1, 2, 3]\n\tt += x\nt"), 6);
}

TEST_F(SigmaGapTests, CrLfLineEndings) {
    // Files saved on Windows arrive with \r\n.
    EXPECT_EQ(Run<int>("t = 0\r\nfor x in [1, 2]\r\n    t += x\r\nt\r\n"), 3);
}

TEST_F(SigmaGapTests, BlankLinesInsideABody) {
    EXPECT_EQ(Run<int>(
                  "fun f(n: int) -> int\n"
                  "    a = n + 1\n"
                  "\n"
                  "    b = a * 2\n"
                  "\n"
                  "    return b\n"
                  "f(4)"),
              10);
}

TEST_F(SigmaGapTests, AssertPassesSilently) {
    EXPECT_EQ(Run<int>("assert(1 + 1 == 2)\nassert(\"a\" < \"b\")\n7"), 7);
}

TEST_F(SigmaGapTests, AssertConditionMustBeBool) {
    ExpectError("assert(1)", "must be bool");
}

// --- Operators -----------------------------------------------------------------

TEST_F(SigmaGapTests, ShiftOperators) {
    EXPECT_EQ(Run<int>("r = 1 << 4\nr"), 16);
    EXPECT_EQ(Run<int>("r = 256 >> 2\nr"), 64);
}

TEST_F(SigmaGapTests, BitwisePrecedenceLadder) {
    // & binds tighter than ^, which binds tighter than |. Left to right would
    // give ((8 | 6) ^ 3) & 1 = 1.
    EXPECT_EQ(Run<int>("r = 8 | 6 ^ 3 & 1\nr"), 15);
}

TEST_F(SigmaGapTests, ShiftBindsLooserThanAddition) {
    EXPECT_EQ(Run<int>("r = 1 << 2 + 1\nr"), 8);
}

TEST_F(SigmaGapTests, NegativeDivisionAndModulo) {
    // Integer division truncates toward zero and % takes the dividend's sign.
    EXPECT_EQ(Run<int>("r = -7 / 2\nr"), -3);
    EXPECT_EQ(Run<int>("r = -7 % 3\nr"), -1);
}

TEST_F(SigmaGapTests, BitNotOnInts) {
    EXPECT_EQ(Run<int>("r = ~0\nr"), -1);
    EXPECT_EQ(Run<int>("r = ~5 & 7\nr"), 2);
}

TEST_F(SigmaGapTests, MixedIntFloatComparisons) {
    EXPECT_TRUE(Run<bool>("b = 1 < 1.5\nb"));
    EXPECT_TRUE(Run<bool>("b = 2 == 2.0\nb"));
    EXPECT_FALSE(Run<bool>("b = 3 <= 2.5\nb"));
}

TEST_F(SigmaGapTests, StringEquality) {
    EXPECT_TRUE(Run<bool>("b = \"kai\" == \"kai\" && \"kai\" != \"pi\"\nb"));
}

TEST_F(SigmaGapTests, ListConcatenation) {
    EXPECT_EQ(Run<int>("xs = [1, 2] + [3]\nxs.size()"), 3);
    ExpectError("xs = [1] + [\"a\"]", "operator '+'");
}

TEST_F(SigmaGapTests, ChainedComparisonIsRejected) {
    // 1 < 2 < 3 is (1 < 2) < 3: a bool compared with an int.
    ExpectError("b = 1 < 2 < 3", "operator '<'");
}

// --- Types and widening ----------------------------------------------------------

TEST_F(SigmaGapTests, TernaryBranchesWidenToFloat) {
    EXPECT_FLOAT_EQ(Run<float>("c = true\nr: float = c ? 1 : 2.5\nr / 2"), 0.5f);
}

TEST_F(SigmaGapTests, ListLiteralWidensToDeclaredFloatList) {
    EXPECT_FLOAT_EQ(Run<float>("xs: List[float] = [1, 2]\nxs[0] / 2"), 0.5f);
}

TEST_F(SigmaGapTests, MapLiteralWidensToDeclaredFloatMap) {
    EXPECT_FLOAT_EQ(Run<float>("m: Map[str, float] = {\"a\": 1}\nm[\"a\"] / 2"), 0.5f);
}

TEST_F(SigmaGapTests, StringIsAnAliasOfStr) {
    EXPECT_EQ(Run<String>("s: string = \"x\"\nt: str = s + \"y\"\nt"), String("xy"));
}

TEST_F(SigmaGapTests, FunctionTypesAreExact) {
    // Widening applies to values, not to the result type of a function value.
    ExpectError(
        "fun inc(n: int) -> int\n    return n + 1\nf: fun(int) -> float = inc",
        "expected fun(int) -> float, got fun(int) -> int");
}

TEST_F(SigmaGapTests, AnyRoundTrip) {
    EXPECT_EQ(Run<int>("x: any = 5\ny: int = x\ny + 1"), 6);
}

TEST_F(SigmaGapTests, NestedListsIndexTwice) {
    EXPECT_EQ(Run<int>("g: List[List[int]] = [[1, 2], [3]]\ng[0][1] + g[1][0]"), 5);
}

TEST_F(SigmaGapTests, MapOfLists) {
    EXPECT_EQ(Run<int>(
                  "m: Map[str, List[int]] = {\"a\": [1, 2, 3], \"b\": [4]}\n"
                  "v = m[\"a\"]\n"
                  "v.size() + m[\"b\"][0]"),
              7);
}

TEST_F(SigmaGapTests, ListOfFunctions) {
    EXPECT_EQ(Run<int>(
                  "fun inc(n: int) -> int\n    return n + 1\n"
                  "fun dbl(n: int) -> int\n    return n * 2\n"
                  "fs: List[fun(int) -> int] = [inc, dbl]\n"
                  "f = fs[1]\n"
                  "f(21)"),
              42);
}

// --- Scope, references and loops -------------------------------------------------

TEST_F(SigmaGapTests, ParameterDoesNotLeakIntoGlobal) {
    EXPECT_EQ(Run<int>(
                  "x = 10\n"
                  "fun f(x: int) -> int\n"
                  "    x += 1\n"
                  "    return x\n"
                  "r = f(1)\n"
                  "x + r"),
              12);
}

TEST_F(SigmaGapTests, LoopVariableOutlivesTheLoop) {
    EXPECT_EQ(Run<int>("for i = 0; i < 3; i += 1\n    print(i)\ni"), 3);
    EXPECT_EQ(Run<int>("for x in [4, 5, 6]\n    print(x)\nx"), 6);
}

TEST_F(SigmaGapTests, DeepNonTailRecursion) {
    EXPECT_EQ(Run<int>(
                  "fun depth(n: int) -> int\n"
                  "    if n == 0\n"
                  "        return 0\n"
                  "    return 1 + depth(n - 1)\n"
                  "depth(300)"),
              300);
}

TEST_F(SigmaGapTests, ListsArePassedByReference) {
    EXPECT_EQ(Run<int>(
                  "ys: List[int] = []\n"
                  "fun fill(xs: List[int], n: int)\n"
                  "    for i = 0; i < n; i += 1\n"
                  "        xs.push(i * i)\n"
                  "fill(ys, 4)\n"
                  "ys.size() * 100 + ys[3]"),
              409);
}

TEST_F(SigmaGapTests, ReturnFromNestedLoops) {
    EXPECT_EQ(Run<int>(
                  "fun find(target: int) -> int\n"
                  "    for i = 0; i < 5; i += 1\n"
                  "        for j = 0; j < 5; j += 1\n"
                  "            if i * j == target\n"
                  "                return i * 10 + j\n"
                  "    return -1\n"
                  "find(6) + find(99)"),
              22);
}

TEST_F(SigmaGapTests, FloatAccumulationInLoop) {
    EXPECT_FLOAT_EQ(Run<float>("x: float = 0\nwhile x < 2\n    x += 0.5\nx"), 2.0f);
}

TEST_F(SigmaGapTests, StringBuiltInALoop) {
    EXPECT_EQ(Run<int>("s = \"\"\nfor i = 0; i < 3; i += 1\n    s += \"ab\"\ns.size()"), 6);
}

// --- Strings and containers ------------------------------------------------------

TEST_F(SigmaGapTests, SingleAndDoubleQuotedStringsMix) {
    EXPECT_EQ(Run<String>("a = 'ab'\nb = \"cd\"\na + b"), String("abcd"));
    EXPECT_EQ(Run<int>("s = \"it's\"\ns.size()"), 4);
}

TEST_F(SigmaGapTests, EmptyListLoopSkipsItsBody) {
    EXPECT_EQ(Run<int>("xs: List[int] = []\nt = 0\nfor x in xs\n    t += 1\nt"), 0);
}

TEST_F(SigmaGapTests, MapKeysThenSize) {
    EXPECT_EQ(Run<int>("m = {\"a\": 1, \"b\": 2, \"c\": 3}\nm.keys().size()"), 3);
}

TEST_F(SigmaGapTests, SliceThenConcatenate) {
    EXPECT_EQ(Run<int>(
                  "xs = [1, 2, 3, 4]\n"
                  "ys = xs.slice(1, 3) + [9]\n"
                  "ys[0] * 100 + ys[1] * 10 + ys[2]"),
              239);
}

TEST_F(SigmaGapTests, PrintHasNoValue) {
    ExpectOk("print(1)\nprint(\"s\")\nprint([1, 2])");
    ExpectError("x = print(1)", "void");
}

// --- The SigmaTranslator API -----------------------------------------------------

TEST_F(SigmaGapTests, CompileIsDeterministic) {
    const std::string program =
        "fun f(a: int, b: float) -> float\n"
        "    return a * b - 1\n"
        "xs: List[float] = [1, 2, 3]\n"
        "t: float = 0\n"
        "for x in xs\n"
        "    t += f(2, x)\n";
    EXPECT_EQ(Rho(program), Rho(program));
}

TEST_F(SigmaGapTests, ErrorsClearAfterASuccessfulCompile) {
    SigmaTranslator sigma(*reg_);
    EXPECT_FALSE(sigma.Compile("x: int = \"s\""));
    EXPECT_FALSE(sigma.GetErrors().empty());
    EXPECT_TRUE(sigma.Compile("y = 1")) << Join(sigma.GetErrors());
    EXPECT_TRUE(sigma.GetErrors().empty()) << Join(sigma.GetErrors());
}

TEST_F(SigmaGapTests, ResetSessionForgetsFunctions) {
    SigmaTranslator sigma(*reg_);
    ASSERT_TRUE(sigma.Compile("fun twice(n: int) -> int\n    return n * 2\n"));
    EXPECT_TRUE(sigma.Compile("a = twice(4)"));
    sigma.ResetSession();
    EXPECT_FALSE(sigma.Compile("b = twice(4)"));
    EXPECT_NE(Join(sigma.GetErrors()).find("twice"), std::string::npos);
}

TEST_F(SigmaGapTests, FailedCompileDoesNotPolluteTheSession) {
    // The failed line never ran, so it must not have declared x as an int.
    SigmaTranslator sigma(*reg_);
    EXPECT_FALSE(sigma.Compile("x: int = \"s\""));
    EXPECT_TRUE(sigma.Compile("x: str = \"s\"")) << Join(sigma.GetErrors());
}

TEST_F(SigmaGapTests, TranslateReportsAPositionedError) {
    SigmaTranslator sigma(*reg_);
    sigma.Translate("a = 1\nb: int = \"s\"", Structure::Program);
    EXPECT_TRUE(sigma.failed);
    EXPECT_EQ(sigma.error.rfind("2:", 0), 0u) << sigma.error;
}

TEST_F(SigmaGapTests, DocsExampleCompilesAndRuns) {
    // The example at the top of Doc/Sigma/README.md: its generated Rho and its
    // documented result.
    const std::string program =
        "fun mean(xs: List[float]) -> float\n"
        "    total: float = 0\n"
        "    for x in xs\n"
        "        total += x\n"
        "    return total / xs.size()\n"
        "\n"
        "samples: List[float] = [2, 4, 4, 5]\n"
        "m = mean(samples)\n"
        "m\n";
    EXPECT_FLOAT_EQ(Run<float>(program), 3.75f);
    EXPECT_EQ(Rho(program),
              "fun mean(xs)\n"
              "    total = (0 + 0.0)\n"
              "    for x in xs\n"
              "        total = (total + x)\n"
              "    return (total / xs.size())\n"
              "samples = [(2 + 0.0), (4 + 0.0), (4 + 0.0), (5 + 0.0)]\n"
              "m = mean(samples)\n"
              "m\n");
}

// --- Static errors ---------------------------------------------------------------

TEST_F(SigmaGapTests, DocsErrorExample) {
    // The error listing in Doc/Sigma/README.md, with its positions.
    auto errors = ErrorList(
        "fun area(w: int, h: int) -> int\n"
        "    return w * h\n"
        "\n"
        "a = area(3, \"4\")\n"
        "a = \"big\"\n"
        "b: bool = a > 10 ? 1 : 0\n");
    ASSERT_EQ(errors.size(), 4u) << Join(errors);
    EXPECT_EQ(errors[0], "4:13: argument 2 of 'area': expected int, got str");
    EXPECT_EQ(errors[1], "5:5: cannot assign to 'a': expected int, got str");
    EXPECT_EQ(errors[2], "6:20: conditional branch: expected bool, got int");
    EXPECT_EQ(errors[3], "6:24: conditional branch: expected bool, got int");
}

TEST_F(SigmaGapTests, EveryErrorInOneBodyIsReported) {
    auto errors = ErrorList(
        "fun f(n: int) -> int\n"
        "    a: str = n\n"
        "    b: int = \"x\"\n"
        "    return c\n");
    EXPECT_EQ(errors.size(), 3u) << Join(errors);
}

TEST_F(SigmaGapTests, VoidCallInsideAnExpression) {
    ExpectError("fun g()\n    print(1)\nr = g() + 1", "void");
}

TEST_F(SigmaGapTests, MapIndexMustBeAString) {
    ExpectError("m = {\"a\": 1}\nv = m[0]", "expected str");
}

TEST_F(SigmaGapTests, UseBeforeDeclarationInsideAFunction) {
    // Only globals are visible before their declaration; a local is not.
    ExpectError(
        "fun f() -> int\n"
        "    y = x + 1\n"
        "    x = 2\n"
        "    return y\n",
        "undefined name 'x'");
}

// --- Native types ----------------------------------------------------------------

TEST_F(SigmaGapTests, NativePropertiesAndVoidMethods) {
    MyStruct::Register(*reg_);
    ExpectOk("fun f(s: MyStruct) -> str\n    return s.string\n");
    ExpectOk("fun f(s: MyStruct)\n    s.Method2()\n");
    ExpectError("fun f(s: MyStruct)\n    x = s.Method2()\n", "void");
}

TEST_F(SigmaGapTests, NativeTypesInContainers) {
    MyStruct::Register(*reg_);
    ExpectOk(
        "fun total(xs: List[MyStruct]) -> int\n"
        "    t = 0\n"
        "    for s in xs\n"
        "        t += s.num\n"
        "    return t\n");
    ExpectError(
        "fun total(xs: List[MyStruct]) -> int\n"
        "    return xs[0]\n",
        "expected int, got MyStruct");
}
