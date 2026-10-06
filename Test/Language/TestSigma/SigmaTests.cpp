// Sigma: statically typed KAI, compiled to Rho.

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace kai;
namespace fs = std::filesystem;

struct SigmaTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Sigma);
    }

    // Compile only; returns the errors (empty on success).
    std::vector<std::string> Errors(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        sigma.Compile(code.c_str());
        return sigma.GetErrors();
    }

    std::string Rho(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        EXPECT_TRUE(sigma.Compile(code.c_str())) << Join(sigma.GetErrors());
        return sigma.GetRho();
    }

    static std::string Join(const std::vector<std::string> &lines) {
        std::string all;
        for (auto const &l : lines) all += l + "\n";
        return all;
    }

    void ExpectOk(const std::string &code) {
        auto errors = Errors(code);
        EXPECT_TRUE(errors.empty()) << code << "\n--- errors:\n" << Join(errors);
    }

    void ExpectError(const std::string &code, const std::string &fragment) {
        auto errors = Errors(code);
        const std::string all = Join(errors);
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

// --- Running Sigma ------------------------------------------------------------

TEST_F(SigmaTests, Declarations) {
    EXPECT_EQ(Run<int>("x: int = 5\nx"), 5);
    EXPECT_EQ(Run<int>("x = 2\ny = x * 21\ny"), 42);
    EXPECT_EQ(Run<String>("s = 'kai'\nt: str = s + \"!\"\nt"), String("kai!"));
}

TEST_F(SigmaTests, Functions) {
    const std::string add = "fun add(a: int, b: int) -> int\n    return a + b\n";
    EXPECT_EQ(Run<int>(add + "add(2, 3)"), 5);
    EXPECT_EQ(Run<int>(
                  "fun fact(n: int) -> int\n"
                  "    if n <= 1\n"
                  "        return 1\n"
                  "    return n * fact(n - 1)\n"
                  "fact(6)"),
              720);
}

TEST_F(SigmaTests, ForwardCallAndMutualRecursion) {
    EXPECT_EQ(Run<bool>(
                  "fun even(n: int) -> bool\n"
                  "    if n == 0\n"
                  "        return true\n"
                  "    return odd(n - 1)\n"
                  "fun odd(n: int) -> bool\n"
                  "    if n == 0\n"
                  "        return false\n"
                  "    return even(n - 1)\n"
                  "even(10)"),
              true);
}

TEST_F(SigmaTests, PrecedenceIsSigmas) {
    // Rho alone evaluates `-1 + 5` to -1; Sigma parenthesises its output.
    EXPECT_EQ(Run<int>("a = -1 + 5\na"), 4);
    EXPECT_EQ(Run<int>("a = 2 + 3 * 4 - -2\na"), 16);
    EXPECT_EQ(Run<bool>("b = !(1 < 2) || 3 == 3\nb"), true);
}

TEST_F(SigmaTests, CompoundAssignmentAndLoops) {
    // Rho's `+=` is not implemented at runtime; Sigma expands it.
    EXPECT_EQ(Run<int>("i = 0\nwhile i < 3\n    i += 1\ni"), 3);
    EXPECT_EQ(Run<int>("t = 0\nfor i = 0; i < 4; i += 1\n    t += i\nt"), 6);
    EXPECT_EQ(Run<int>("i = 0\ndo\n    i += 2\nwhile i < 5\ni"), 6);
    EXPECT_EQ(Run<int>("t = 0\nfor x in [1, 2, 3]\n    t += x\nt"), 6);
}

TEST_F(SigmaTests, BreakAndContinue) {
    EXPECT_EQ(Run<int>(
                  "t = 0\n"
                  "for x in [1, 2, 3, 4, 5, 6]\n"
                  "    if x == 2\n"
                  "        continue\n"
                  "    if x == 5\n"
                  "        break\n"
                  "    t += x\n"
                  "t"),
              8);
}

TEST_F(SigmaTests, IntWidensToFloat) {
    // Without explicit widening Rho would store Int 1 and divide as ints.
    EXPECT_FLOAT_EQ(Run<float>("x: float = 1\ny = x / 2\ny"), 0.5f);
    EXPECT_FLOAT_EQ(Run<float>("fun f() -> float\n    return 3\nr = f() / 2\nr"), 1.5f);
    EXPECT_FLOAT_EQ(Run<float>("fun h(x: float) -> float\n    return x / 4\nh(1)"), 0.25f);
    EXPECT_FLOAT_EQ(Run<float>("x: float = 1\nx += 2\nx / 2"), 1.5f);
    EXPECT_FLOAT_EQ(Run<float>("xs = [1, 2.5]\nxs[0] / 2"), 0.5f);
}

TEST_F(SigmaTests, Containers) {
    EXPECT_EQ(Run<int>("xs: List[int] = [1, 2]\nxs.push(3)\nxs.size()"), 3);
    EXPECT_EQ(Run<int>("xs = [10, 20, 30]\nxs[1]"), 20);
    EXPECT_EQ(Run<int>("xs = [1, 2, 3]\nys = xs.slice(0, 2)\nys.size()"), 2);
    EXPECT_EQ(Run<int>("m = {\"a\": 7, \"b\": 8}\nm[\"b\"]"), 8);
    EXPECT_EQ(Run<int>("m: Map[str, int] = {\"a\": 1}\nks = m.keys()\nks.size()"), 1);
    EXPECT_EQ(Run<int>("s = \"hello\"\ns.size()"), 5);
}

TEST_F(SigmaTests, ConditionalsAndTernary) {
    const std::string sign =
        "fun sign(n: int) -> int\n"
        "    if n > 0\n"
        "        return 1\n"
        "    else if n < 0\n"
        "        return -1\n"
        "    else\n"
        "        return 0\n";
    EXPECT_EQ(Run<int>(sign + "sign(-7)"), -1);
    EXPECT_EQ(Run<int>(sign + "sign(0)"), 0);
    EXPECT_EQ(Run<int>("c = false\nr = c ? 1 : 2\nr"), 2);
}

TEST_F(SigmaTests, GlobalsWrittenFromFunctions) {
    EXPECT_EQ(Run<int>("count = 0\nfun bump()\n    count += 1\nbump()\nbump()\ncount"), 2);
}

TEST_F(SigmaTests, FunctionValues) {
    EXPECT_EQ(Run<int>(
                  "fun inc(n: int) -> int\n"
                  "    return n + 1\n"
                  "f: fun(int) -> int = inc\n"
                  "f(41)"),
              42);
}

TEST_F(SigmaTests, PiBlocks) {
    EXPECT_EQ(Run<int>("n: int = pi { 2 3 + }\nn + 1"), 6);
}

TEST_F(SigmaTests, GeneratedRho) {
    const std::string rho = Rho("x: float = 1\ni = 0\ni += 2\ny = -i + x * 2");
    EXPECT_EQ(rho,
              "x = (1 + 0.0)\n"
              "i = 0\n"
              "i = (i + 2)\n"
              "y = ((-i) + (x * 2))\n");
}

// --- Static errors --------------------------------------------------------------

TEST_F(SigmaTests, ParametersNeedTypes) {
    ExpectError("fun f(a)\n    return a\n", "parameter 'a' needs a type");
}

TEST_F(SigmaTests, UndefinedNames) {
    ExpectError("y = x + 1", "undefined name 'x'");
    ExpectError("fun f() -> int\n    return z\n", "undefined name 'z'");
}

TEST_F(SigmaTests, InferredTypesAreFixed) {
    ExpectError("x = 1\nx = \"s\"", "cannot assign to 'x': expected int, got str");
    ExpectError("x: int = \"s\"", "cannot initialise 'x': expected int, got str");
    ExpectError("x: int = 2.5", "expected int, got float");
    ExpectOk("x: float = 2\nx = 3");
}

TEST_F(SigmaTests, NoShadowing) {
    ExpectError("x = 1\nx: str = \"a\"", "'x' is already declared");
    ExpectError("x = 1\nfun f()\n    x: str = \"a\"\n", "'x' is already declared");
    ExpectError("fun f()\n    print(1)\nf = 3", "cannot assign to function 'f'");
    ExpectOk("x = 1\nfun f(x: str) -> str\n    return x\n");  // parameters are local in Rho
}

TEST_F(SigmaTests, ReturnRules) {
    ExpectError("fun f() -> int\n    return \"s\"\n", "'f' returns int: expected int, got str");
    ExpectError("fun f(n: int) -> int\n    if n > 0\n        return 1\n", "'f' does not return a value on every path");
    ExpectError("fun f()\n    return 1\n", "'f' does not return a value");
    ExpectError("fun f() -> int\n    return\n", "'f' must return int");
    ExpectError("return 1", "'return' outside a function");
    ExpectOk("fun f(n: int) -> int\n    if n > 0\n        return 1\n    else\n        return 2\n");
}

TEST_F(SigmaTests, ConditionsAreBool) {
    ExpectError("if 1\n    x = 1\n", "an if condition must be bool, got int");
    ExpectError("while \"s\"\n    x = 1\n", "a while condition must be bool");
    ExpectError("b = 1 && true", "operator '&&' cannot be applied to int and bool");
}

TEST_F(SigmaTests, Operators) {
    ExpectError("s = \"a\" + 1", "operator '+' cannot be applied to str and int");
    ExpectError("b = 1 == \"1\"", "operator '=='");
    ExpectError("n = -\"a\"", "operator '-' cannot be applied to str");
    ExpectError("n = 1.5 & 2", "operator '&'");
    ExpectOk("a = 1 + 2.5\nb = \"x\" + \"y\"\nc = [1] + [2]\nd = 1 < 2.5\ne = \"a\" < \"b\"");
}

TEST_F(SigmaTests, Containers_Errors) {
    ExpectError("xs = []", "cannot infer the element type of an empty list");
    ExpectError("m = {}", "cannot infer the value type of an empty map");
    ExpectError("xs = [1, \"a\"]", "list elements have different types: int and str");
    ExpectError("xs: List[int] = [1, \"a\"]", "list element: expected int, got str");
    ExpectError("xs = [1]\nxs.push(\"s\")", "argument 1 of 'push': expected int, got str");
    ExpectError("xs = [1]\ns: str = xs[0]", "expected str, got int");
    ExpectError("xs = [1]\nv = xs[\"k\"]", "list index: expected int, got str");
    ExpectError("xs = [1]\nxs.nope()", "List[int] has no method 'nope'");
    ExpectError("xs: List[float] = [1.0]\nys: List[int] = xs", "expected List[int], got List[float]");
    ExpectError("m: Map[int, str] = {}", "map keys must be str");
    ExpectError("n = 5\nv = n[0]", "int cannot be indexed");
}

TEST_F(SigmaTests, Calls) {
    const std::string add = "fun add(a: int, b: int) -> int\n    return a + b\n";
    ExpectError(add + "add(1)", "'add' expects 2 arguments, got 1");
    ExpectError(add + "add(1, \"2\")", "argument 2 of 'add': expected int, got str");
    ExpectError(add + "s: str = add(1, 2)", "expected str, got int");
    ExpectError("n = 1\nn(2)", "'n' of type int is not a function");
    ExpectError("fun g()\n    print(1)\nx = g()", "cannot initialise 'x' with a void value");
    ExpectError(
        "fun apply(f: fun(int) -> int, x: int) -> int\n    return f(x)\n"
        "fun up(s: str) -> str\n    return s\n"
        "apply(up, 1)",
        "argument 1 of 'apply': expected fun(int) -> int, got fun(str) -> str");
}

TEST_F(SigmaTests, AnyIsExplicit) {
    ExpectError("x = pi { 1 }", "cannot infer a type for 'x' from 'any'");
    ExpectOk("x: any = pi { 1 }\ny: int = x");
}

TEST_F(SigmaTests, LoopsAndControl) {
    ExpectError("break", "'break' outside a loop");
    ExpectError("for x in 5\n    print(x)\n", "can only iterate over a List, not int");
    ExpectError("x: str = \"a\"\nfor x in [1, 2]\n    print(x)\n", "loop variable 'x' is str, but the elements are int");
}

TEST_F(SigmaTests, UnsupportedSyntax) {
    ExpectError("i = 0\ni++", "'++' is not supported");
    ExpectError("fun f()\n    fun g()\n        print(1)\n", "nested functions are not supported");
    ExpectError("f = fun(x: int)\n    return x\n", "anonymous functions are not supported");
    ExpectError("x = [1, 2][0]", "must follow a name");
    ExpectError("dup = 1", "'dup' is a Pi word");
    ExpectError("x: int? = 1", "nullable types");
    ExpectError("x: Nope = 1", "unknown type 'Nope'");
    ExpectError("x: int", "a declaration needs an initial value");
    ExpectError("a = 1 b = 2", "expected end of statement");
}

TEST_F(SigmaTests, NativeTypes) {
    MyStruct::Register(*reg_);
    ExpectOk(
        "fun use(s: MyStruct) -> str\n"
        "    n: int = s.Method0()\n"
        "    return s.Method1(n, \"x\")\n");
    ExpectOk("fun use(s: MyStruct) -> int\n    return s.num\n");
    ExpectError("fun use(s: MyStruct)\n    s.Method1(\"x\", 1)\n", "argument 1 of 'MyStruct.Method1': expected int, got str");
    ExpectError("fun use(s: MyStruct)\n    s.Method1(1)\n", "'MyStruct.Method1' expects 2 arguments, got 1");
    ExpectError("fun use(s: MyStruct)\n    s.Nope()\n", "MyStruct has no method 'Nope'");
    ExpectError("fun use(s: MyStruct) -> str\n    return s.num\n", "expected str, got int");
}

TEST_F(SigmaTests, ErrorPositions) {
    auto errors = Errors("a = 1\nb = 2\nc: int = \"s\"");
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_EQ(errors[0].rfind("3:10:", 0), 0u) << errors[0];
}

TEST_F(SigmaTests, ErrorsAreSortedAndAllReported) {
    auto errors = Errors("fun f() -> int\n    return \"s\"\nx: int = \"t\"\ny = q");
    ASSERT_EQ(errors.size(), 3u) << Join(errors);
    EXPECT_EQ(errors[0].rfind("2:", 0), 0u);
    EXPECT_EQ(errors[1].rfind("3:", 0), 0u);
    EXPECT_EQ(errors[2].rfind("4:", 0), 0u);
}

TEST_F(SigmaTests, IllTypedProgramsDoNotRun) {
    data_->Clear();
    console_.Execute("x: int = \"s\"\n41", Structure::Program);
    EXPECT_TRUE(data_->Empty());
}

// --- Example scripts --------------------------------------------------------------

TEST_F(SigmaTests, Scripts) {
    const fs::path root(KAI_STRINGISE(KAI_SCRIPT_ROOT));
    int ran = 0;
    for (auto const &entry : fs::directory_iterator(root)) {
        if (entry.path().extension() != ".sigma") continue;
        std::ifstream in(entry.path());
        std::stringstream text;
        text << in.rdbuf();

        SigmaTranslator sigma(*reg_);
        ASSERT_TRUE(sigma.Compile(text.str().c_str())) << entry.path() << "\n" << Join(sigma.GetErrors());

        // A fresh console and translator per script. Rho resolves names when
        // a function is called, so globals one script leaves in a shared
        // console are visible to the next: a later script's loop can read an
        // earlier script's variable and never terminate.
        Console console;
        TestLangCommon::SetupTranslatorsForConsole(console);
        // As in TestLangCommon::SetUp; the Console app does not register Map.
        if (!console.GetRegistry().GetClass(Label("Map")))
            console.GetRegistry().AddClass<Map>(Label("Map"));
        SigmaTranslator runner(console.GetRegistry());
        auto program = runner.Translate(text.str().c_str(), Structure::Program);
        ASSERT_FALSE(runner.failed) << entry.path() << "\n" << runner.error;
        EXPECT_NO_THROW(console.Execute(program)) << entry.path();
        auto data = console.GetExecutor()->GetDataStack();
        ASSERT_FALSE(data->Empty()) << entry.path() << "\n--- rho:\n" << sigma.GetRho();
        EXPECT_TRUE(data->Top().IsType<bool>() && ConstDeref<bool>(data->Top()))
            << entry.path() << " should end with a true expression";
        ++ran;
    }
    EXPECT_GE(ran, 23);
}

// Rho runtime bug, not a Sigma one: an early `return` inside an `if` is lost
// when the function is called from inside a loop, so f(0) below yields true.
// Executor::ExecuteContinuationInlineAndDrain resets break_ on every drain
// step, which discards the Return's break_. Plain Rho reproduces it:
//
//   fun f(n)                c = 0
//       if n < 2            m = 0
//           return false    while m < 5
//       return true             if f(m)
//                                   c = c + 1
//                               m = m + 1
//                           c                  // 5, should be 3
TEST_F(SigmaTests, ReturnOfCallInsideIf) {
    EXPECT_EQ(Run<int>(
                  "fun tens(n: int) -> int\n"
                  "    return n * 10\n"
                  "fun f(n: int) -> int\n"
                  "    if n > 1\n"
                  "        return tens(n)\n"
                  "    return 0\n"
                  "f(4) + f(1)"),
              40);
}

TEST_F(SigmaTests, EarlyReturnInFunctionCalledFromLoop) {
    EXPECT_EQ(Run<int>(
                  "fun f(n: int) -> bool\n"
                  "    if n < 2\n"
                  "        return false\n"
                  "    return true\n"
                  "c = 0\n"
                  "for m = 0; m < 5; m += 1\n"
                  "    if f(m)\n"
                  "        c += 1\n"
                  "c"),
              3);
}

// --- Sessions (REPL) --------------------------------------------------------------

TEST_F(SigmaTests, SessionRemembersDeclarations) {
    SigmaTranslator sigma(*reg_);
    ASSERT_TRUE(sigma.Compile("x: int = 2\nfun twice(n: int) -> int\n    return n * 2\n"));
    EXPECT_TRUE(sigma.Compile("y = twice(x)")) << Join(sigma.GetErrors());
    EXPECT_FALSE(sigma.Compile("x = \"s\""));
    EXPECT_NE(Join(sigma.GetErrors()).find("expected int, got str"), std::string::npos);
    EXPECT_TRUE(sigma.Compile("x: int = 3")) << "same type may be redeclared";
    EXPECT_FALSE(sigma.Compile("x: str = \"s\""));
    EXPECT_NE(Join(sigma.GetErrors()).find("declared earlier in this session as int"), std::string::npos);
    EXPECT_TRUE(sigma.Compile("fun twice(n: int) -> int\n    return n + n\n"));
    EXPECT_FALSE(sigma.Compile("fun twice(s: str) -> str\n    return s\n"));
    sigma.ResetSession();
    EXPECT_FALSE(sigma.Compile("y = x"));
}

TEST_F(SigmaTests, ConsoleLinesShareASession) {
    data_->Clear();
    console_.Execute("x: int = 2", Structure::Program);
    console_.Execute("y = x * 21", Structure::Program);
    console_.Execute("y", Structure::Program);
    ASSERT_FALSE(data_->Empty());
    EXPECT_EQ(ConstDeref<int>(data_->Top()), 42);
}
