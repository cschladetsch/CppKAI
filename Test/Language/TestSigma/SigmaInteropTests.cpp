// C++ <> Sigma round trips: values, functions and objects crossing the
// boundary in both directions through SigmaHost.

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <functional>

#include "KAI/Language/Sigma/SigmaHost.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace kai;

KAI_BEGIN
struct Vec {
    int x = 0;
    int y = 0;
    int Dot(int a, int b) const { return x * a + y * b; }
    void Scale(int k) { x *= k; y *= k; }
    static void Register(Registry &r) {
        ClassBuilder<Vec>(r, "Vec")
            .methods("Dot", &Vec::Dot, "dot product")("Scale", &Vec::Scale, "scale in place")
            .properties("x", &Vec::x, "x")("y", &Vec::y, "y");
    }
};
KAI_TYPE_TRAITS(Vec, 778, Properties::Nothing);
KAI_END

struct SigmaInteropTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Sigma);
        host_ = std::make_unique<SigmaHost>(console_);
    }

    void BindVec() {
        Vec::Register(*reg_);
        host_->Bind("Vec", [](int x, int y) {
            Vec v;
            v.x = x;
            v.y = y;
            return v;
        });
    }

    std::string ErrorsOf(const std::string &code) {
        try {
            host_->Run(code);
        } catch (const SigmaError &e) {
            return e.what();
        }
        return "";
    }

    std::unique_ptr<SigmaHost> host_;
};

// --- values in -----------------------------------------------------------------

TEST_F(SigmaInteropTests, SetBoolIsReadAsBool) {
    host_->Set("flag", true);
    EXPECT_TRUE(host_->Eval<bool>("flag"));
}

TEST_F(SigmaInteropTests, SetFloatIsUsedInArithmetic) {
    host_->Set("ratio", 0.5f);
    EXPECT_FLOAT_EQ(host_->Eval<float>("ratio * 8"), 4.0f);
}

TEST_F(SigmaInteropTests, SetValueDrivesAControlFlowCondition) {
    host_->Set("n", 3);
    EXPECT_EQ(host_->Eval<int>("t = 0\nfor i = 0; i < n; i += 1\n    t += 10\nt"), 30);
}

TEST_F(SigmaInteropTests, SetValueIsTypeCheckedAgainstItsUse) {
    host_->Set("n", 3);
    EXPECT_NE(ErrorsOf("s: str = n").find("expected str, got int"), std::string::npos);
}

TEST_F(SigmaInteropTests, SetAgainReplacesTheValue) {
    host_->Set("n", 1);
    EXPECT_EQ(host_->Eval<int>("n"), 1);
    host_->Set("n", 2);
    EXPECT_EQ(host_->Eval<int>("n"), 2);
}

// --- values out ----------------------------------------------------------------

TEST_F(SigmaInteropTests, EvalSeesWhatRunDeclared) {
    host_->Run("base = 40");
    EXPECT_EQ(host_->Eval<int>("base + 2"), 42);
}

TEST_F(SigmaInteropTests, GetReadsAStringAndAFloat) {
    host_->Run("s = \"hi\"\nf = 1.5");
    EXPECT_EQ(host_->Get<String>("s"), String("hi"));
    EXPECT_FLOAT_EQ(host_->Get<float>("f"), 1.5f);
}

TEST_F(SigmaInteropTests, GetOfAnUnknownNameThrows) {
    EXPECT_THROW(host_->Get<int>("nothing_here"), SigmaError);
}

TEST_F(SigmaInteropTests, GetWithTheWrongTypeThrows) {
    host_->Run("s = \"hi\"");
    EXPECT_THROW(host_->Get<int>("s"), SigmaError);
}

TEST_F(SigmaInteropTests, EvalOfAStatementWithNoValueThrows) {
    EXPECT_THROW(host_->Eval<int>("x = 1"), SigmaError);
}

// --- calls out of Sigma --------------------------------------------------------

TEST_F(SigmaInteropTests, BoundFunctionKeepsCapturedState) {
    int counter = 0;
    host_->Bind("tick", [&counter]() { return ++counter; });
    host_->Run("a = tick()\nb = tick()\nc = tick()");
    EXPECT_EQ(counter, 3);
    EXPECT_EQ(host_->Get<int>("c"), 3);
}

TEST_F(SigmaInteropTests, NativeCallInsideALoopRunsEachIteration) {
    int sum = 0;
    host_->Bind("add", [&sum](int n) { sum += n; });
    host_->Run("for i = 1; i <= 4; i += 1\n    add(i)");
    EXPECT_EQ(sum, 10);
}

TEST_F(SigmaInteropTests, NativeFloatMathFromSigma) {
    host_->Bind("root", [](float x) { return std::sqrt(x); });
    EXPECT_FLOAT_EQ(host_->Eval<float>("root(16)"), 4.0f);
}

TEST_F(SigmaInteropTests, NativeBoolDecidesATernary) {
    host_->Bind("is_big", [](int n) { return n > 100; });
    EXPECT_EQ(host_->Eval<int>("is_big(500) ? 1 : 2"), 1);
    EXPECT_EQ(host_->Eval<int>("is_big(5) ? 1 : 2"), 2);
}

TEST_F(SigmaInteropTests, NativeStringJoinedWithSigmaStrings) {
    host_->Bind("upper_first", [](String s) { return s + String("?"); });
    EXPECT_EQ(host_->Eval<String>("upper_first(\"a\") + \"b\""), String("a?b"));
}

TEST_F(SigmaInteropTests, NativeCallsNest) {
    host_->Bind("inc", [](int n) { return n + 1; });
    EXPECT_EQ(host_->Eval<int>("inc(inc(inc(0)))"), 3);
}

TEST_F(SigmaInteropTests, SigmaRecursionThroughANativeHelper) {
    host_->Bind("sub1", [](int n) { return n - 1; });
    host_->Run("fun fact(n: int) -> int\n    if n < 2\n        return 1\n    return n * fact(sub1(n))");
    EXPECT_EQ(host_->Eval<int>("fact(5)"), 120);
}

TEST_F(SigmaInteropTests, NativeFunctionPassedToATemplate) {
    host_->Bind("neg", [](int n) { return -n; });
    EXPECT_EQ(host_->Eval<int>(
                  "fun twice[T](f: fun(T) -> T, x: T) -> T\n    return f(f(x))\ntwice(neg, 7)"),
              7);
}

TEST_F(SigmaInteropTests, NativeResultStoredInATypedList) {
    host_->Bind("sq", [](int n) { return n * n; });
    EXPECT_EQ(host_->Eval<int>("xs: List[int] = []\nfor i = 1; i <= 3; i += 1\n    xs.push(sq(i))\nxs[2]"), 9);
}

TEST_F(SigmaInteropTests, WrongArgumentTypeToANativeIsACompileError) {
    host_->Bind("sq", [](int n) { return n * n; });
    EXPECT_NE(ErrorsOf("sq(1.5)").find("argument 1 of 'sq': expected int, got float"), std::string::npos);
}

TEST_F(SigmaInteropTests, NativeVoidResultCannotBeUsedAsAValue) {
    host_->Bind("log", [](int) {});
    EXPECT_NE(ErrorsOf("x = log(1)").find("cannot initialise 'x' with a void value"), std::string::npos);
}

// --- calls into Sigma ----------------------------------------------------------

TEST_F(SigmaInteropTests, CallReturnsABool) {
    host_->Run("fun positive(n: int) -> bool\n    return n > 0");
    EXPECT_TRUE((host_->Call<bool>("positive", 3)));
    EXPECT_FALSE((host_->Call<bool>("positive", -3)));
}

TEST_F(SigmaInteropTests, CallPassesNegativeNumbers) {
    host_->Run("fun twice(n: int) -> int\n    return n * 2");
    EXPECT_EQ((host_->Call<int>("twice", -21)), -42);
}

TEST_F(SigmaInteropTests, CallWithNoArguments) {
    host_->Run("fun answer() -> int\n    return 42");
    EXPECT_EQ((host_->Call<int>("answer")), 42);
}

TEST_F(SigmaInteropTests, CallOfAVoidFunctionRunsIt) {
    int seen = 0;
    host_->Bind("record", [&seen](int n) { seen = n; });
    host_->Run("fun announce(n: int)\n    record(n * 10)");
    host_->Call<void>("announce", 4);
    EXPECT_EQ(seen, 40);
}

TEST_F(SigmaInteropTests, CallOfAnUnknownFunctionThrows) {
    EXPECT_THROW((host_->Call<int>("missing", 1)), SigmaError);
}

TEST_F(SigmaInteropTests, CallWithTheWrongArgumentTypeThrows) {
    host_->Run("fun twice(n: int) -> int\n    return n * 2");
    EXPECT_THROW((host_->Call<int>("twice", "x")), SigmaError);
}

TEST_F(SigmaInteropTests, CallWithTheWrongArgumentCountThrows) {
    host_->Run("fun twice(n: int) -> int\n    return n * 2");
    EXPECT_THROW((host_->Call<int>("twice", 1, 2)), SigmaError);
}

TEST_F(SigmaInteropTests, CallWidensAnIntArgumentToFloat) {
    host_->Run("fun half(x: float) -> float\n    return x / 2.0");
    EXPECT_FLOAT_EQ((host_->Call<float>("half", 5)), 2.5f);
}

TEST_F(SigmaInteropTests, CallbackIsAStdFunctionUsableWithAlgorithms) {
    host_->Run("fun cube(n: int) -> int\n    return n * n * n");
    auto cube = host_->Callback<int(int)>("cube");
    std::vector<int> in{1, 2, 3}, out(3);
    std::transform(in.begin(), in.end(), out.begin(), cube);
    EXPECT_EQ(out, (std::vector<int>{1, 8, 27}));
}

TEST_F(SigmaInteropTests, CallbackWithTwoArguments) {
    host_->Run("fun mix(a: int, b: int) -> int\n    return a * 100 + b");
    EXPECT_EQ((host_->Callback<int(int, int)>("mix")(4, 2)), 402);
}

TEST_F(SigmaInteropTests, CallbackReturningAString) {
    host_->Run("fun greet(s: str) -> str\n    return \"hello \" + s");
    EXPECT_EQ((host_->Callback<String(String)>("greet")(String("bob"))), String("hello bob"));
}

TEST_F(SigmaInteropTests, CppToSigmaToCppRoundTrip) {
    host_->Bind("plus_one", [](int n) { return n + 1; });
    host_->Run("fun f(n: int) -> int\n    return plus_one(n) * 2");
    auto f = host_->Callback<int(int)>("f");
    EXPECT_EQ(f(f(1)), 10);
}

// --- objects -------------------------------------------------------------------

TEST_F(SigmaInteropTests, MethodOnAFactoryMadeObject) {
    BindVec();
    EXPECT_EQ(host_->Eval<int>("v = Vec(2, 3)\nv.Dot(10, 100)"), 320);
}

TEST_F(SigmaInteropTests, MutatingMethodChangesTheObject) {
    BindVec();
    EXPECT_EQ(host_->Eval<int>("v = Vec(2, 3)\nv.Scale(5)\nv.x + v.y"), 25);
}

TEST_F(SigmaInteropTests, PropertyAssignmentThenMethod) {
    BindVec();
    EXPECT_EQ(host_->Eval<int>("v = Vec(0, 0)\nv.x = 4\nv.y = 5\nv.Dot(1, 1)"), 9);
}

TEST_F(SigmaInteropTests, ObjectsPassThroughSigmaFunctions) {
    BindVec();
    host_->Run("fun length_sq(v: Vec) -> int\n    return v.Dot(v.x, v.y)");
    EXPECT_EQ(host_->Eval<int>("length_sq(Vec(3, 4))"), 25);
}

TEST_F(SigmaInteropTests, ObjectsFillATypedList) {
    BindVec();
    EXPECT_EQ(host_->Eval<int>("vs: List[Vec] = []\nvs.push(Vec(1, 2))\nvs.push(Vec(3, 4))\nvs[1].y"), 4);
}

TEST_F(SigmaInteropTests, UnknownMethodIsACompileError) {
    BindVec();
    EXPECT_NE(ErrorsOf("v = Vec(1, 2)\nv.Nope()").find("Vec has no method 'Nope'"), std::string::npos);
}

TEST_F(SigmaInteropTests, MethodArgumentTypesAreChecked) {
    BindVec();
    EXPECT_NE(ErrorsOf("v = Vec(1, 2)\nv.Dot(1, \"x\")").find("expected int, got str"), std::string::npos);
}

TEST_F(SigmaInteropTests, PropertyTypeIsChecked) {
    BindVec();
    EXPECT_NE(ErrorsOf("v = Vec(1, 2)\ns: str = v.x").find("expected str, got int"), std::string::npos);
}

TEST_F(SigmaInteropTests, TwoObjectsAreIndependent) {
    BindVec();
    EXPECT_EQ(host_->Eval<int>("a = Vec(1, 1)\nb = Vec(5, 5)\nb.Scale(2)\na.x * 100 + b.x"), 110);
}
