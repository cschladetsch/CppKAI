// More C++ <> Sigma interop: side effects, re-entrancy, strings, object
// lifetimes and compile-time checks on native signatures.

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <functional>

#include "KAI/Language/Sigma/SigmaHost.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace kai;

KAI_BEGIN
struct Pt {
    int x = 0;
    int y = 0;
    int Dot2(int a, int b) const { return x * a + y * b; }
    void Scale(int k) { x *= k; y *= k; }
    static void Register(Registry &r) {
        ClassBuilder<Pt>(r, "Pt")
            .methods("Dot2", &Pt::Dot2, "dot product")("Scale", &Pt::Scale, "scale in place")
            .properties("x", &Pt::x, "x")("y", &Pt::y, "y");
    }
};
KAI_TYPE_TRAITS(Pt, 779, Properties::Nothing);
KAI_END

struct SigmaInteropMoreTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Sigma);
        host_ = std::make_unique<SigmaHost>(console_);
    }

    void BindPt() {
        Pt::Register(*reg_);
        host_->Bind("Pt", [](int x, int y) {
            Pt v;
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


// --- evaluation order and side effects -----------------------------------------

TEST_F(SigmaInteropMoreTests, AndSkipsANativeCallOnTheRight) {
    int calls = 0;
    host_->Bind("probe", [&calls]() { ++calls; return true; });
    host_->Run("b = false && probe()");
    EXPECT_EQ(calls, 0);
}

TEST_F(SigmaInteropMoreTests, OrSkipsANativeCallOnTheRight) {
    int calls = 0;
    host_->Bind("probe", [&calls]() { ++calls; return true; });
    host_->Run("b = true || probe()");
    EXPECT_EQ(calls, 0);
}

TEST_F(SigmaInteropMoreTests, AndRunsTheNativeCallWhenLeftIsTrue) {
    int calls = 0;
    host_->Bind("probe", [&calls]() { ++calls; return true; });
    host_->Run("b = true && probe()");
    EXPECT_EQ(calls, 1);
}

TEST_F(SigmaInteropMoreTests, ArgumentsAreEvaluatedLeftToRight) {
    std::string order;
    host_->Bind("a", [&order]() { order += "a"; return 1; });
    host_->Bind("b", [&order]() { order += "b"; return 2; });
    host_->Bind("c", [&order]() { order += "c"; return 3; });
    host_->Bind("sum3", [](int x, int y, int z) { return x + y + z; });
    host_->Run("r = sum3(a(), b(), c())");
    EXPECT_EQ(order, "abc");
}

TEST_F(SigmaInteropMoreTests, TernaryRunsOnlyTheChosenNativeBranch) {
    int calls = 0;
    host_->Bind("costly", [&calls]() { ++calls; return 1; });
    host_->Run("r = true ? 5 : costly()");
    EXPECT_EQ(calls, 0);
}

TEST_F(SigmaInteropMoreTests, WhileConditionCallsNativeOncePerTest) {
    int calls = 0;
    host_->Bind("more", [&calls]() { return ++calls <= 3; });
    host_->Run("while more()\n    n = 1");
    EXPECT_EQ(calls, 4);
}

TEST_F(SigmaInteropMoreTests, ThousandNativeCallsInALoop) {
    long total = 0;
    host_->Bind("add", [&total](int n) { total += n; });
    host_->Run("for i = 1; i <= 1000; i += 1\n    add(i)");
    EXPECT_EQ(total, 500500);
}

// --- bigger signatures and mixed types ----------------------------------------

TEST_F(SigmaInteropMoreTests, FiveParameterNative) {
    host_->Bind("poly", [](int a, int b, int c, int d, int e) { return a + 2 * b + 3 * c + 4 * d + 5 * e; });
    EXPECT_EQ(host_->Eval<int>("poly(1, 1, 1, 1, 1)"), 15);
}

TEST_F(SigmaInteropMoreTests, MixedParameterTypes) {
    host_->Bind("describe", [](String name, int n, float scale, bool on) {
        return on ? name + String("!") : name;
    });
    EXPECT_EQ(host_->Eval<String>("describe(\"x\", 1, 2.5, true)"), String("x!"));
    EXPECT_EQ(host_->Eval<String>("describe(\"x\", 1, 2, false)"), String("x"));
}

TEST_F(SigmaInteropMoreTests, NativeReturningNegativeAndZero) {
    host_->Bind("neg", [](int n) { return -n; });
    EXPECT_EQ(host_->Eval<int>("neg(0)"), 0);
    EXPECT_EQ(host_->Eval<int>("neg(-8)"), 8);
}

TEST_F(SigmaInteropMoreTests, NativeResultInArithmeticWithFloat) {
    host_->Bind("two", []() { return 2; });
    EXPECT_FLOAT_EQ(host_->Eval<float>("two() * 1.5"), 3.0f);
}

TEST_F(SigmaInteropMoreTests, NativeFloatResultAssignedToFloatVariable) {
    host_->Bind("three_f", []() { return 3.0f; });
    EXPECT_FLOAT_EQ(host_->Eval<float>("x: float = three_f()\nx"), 3.0f);
}

TEST_F(SigmaInteropMoreTests, NativeIntResultWidensIntoAFloatVariable) {
    host_->Bind("three", []() { return 3; });
    EXPECT_FLOAT_EQ(host_->Eval<float>("x: float = three()\nx"), 3.0f);
}

TEST_F(SigmaInteropMoreTests, NativeFloatResultCannotNarrowToInt) {
    host_->Bind("three_f", []() { return 3.0f; });
    EXPECT_THROW(host_->Run("x: int = three_f()"), SigmaError);
}

TEST_F(SigmaInteropMoreTests, NativeStringsKeyAMap) {
    host_->Bind("key", []() { return String("k"); });
    EXPECT_EQ(host_->Eval<int>("m = {\"k\": 7}\nm[key()]"), 7);
}

TEST_F(SigmaInteropMoreTests, NativeStringsFillAList) {
    host_->Bind("word", [](int n) { return n % 2 ? String("odd") : String("even"); });
    EXPECT_EQ(host_->Eval<String>("ws: List[str] = []\nfor i = 0; i < 3; i += 1\n    ws.push(word(i))\nws[1]"),
              String("odd"));
}

// --- Sigma-defined code meeting native code -----------------------------------

TEST_F(SigmaInteropMoreTests, RedefiningWithTheSameSignatureReplacesTheFunction) {
    host_->Run("fun f() -> int\n    return 1");
    host_->Run("fun f() -> int\n    return 2");
    EXPECT_EQ((host_->Call<int>("f")), 2);
}

TEST_F(SigmaInteropMoreTests, RedefiningWithAnotherSignatureIsRejected) {
    host_->Run("fun f() -> int\n    return 1");
    EXPECT_THROW(host_->Run("fun f() -> str\n    return \"s\""), SigmaError);
}

TEST_F(SigmaInteropMoreTests, ATemplateOnlyProgramRunsAsANoOp) {
    EXPECT_NO_THROW(host_->Run("fun id[T](x: T) -> T\n    return x"));
    EXPECT_EQ(host_->Eval<int>("id(5)"), 5);
}

TEST_F(SigmaInteropMoreTests, AVariadicTemplateOnlyProgramRunsAsANoOp) {
    EXPECT_NO_THROW(host_->Run("fun total[...Ts](xs: Ts...) -> int\n    return (xs + ... + 0)"));
}

TEST_F(SigmaInteropMoreTests, ResetSessionKeepsValuesTheRuntimeStillHolds) {
    host_->Run("x = 1");
    host_->ResetSession();
    EXPECT_EQ(host_->Eval<int>("x + 1"), 2);
}

TEST_F(SigmaInteropMoreTests, ResetSessionAllowsAChangedDeclaration) {
    host_->Run("fun f() -> int\n    return 1");
    host_->ResetSession();
    EXPECT_NO_THROW(host_->Run("fun f() -> str\n    return \"s\""));
}

TEST_F(SigmaInteropMoreTests, SessionRemembersTypesBetweenRuns) {
    host_->Run("x = 1");
    EXPECT_THROW(host_->Run("x = \"s\""), SigmaError);
}

TEST_F(SigmaInteropMoreTests, NativeCalledFromAFunctionDefinedEarlier) {
    EXPECT_THROW(host_->Run("fun f(n: int) -> int\n    return helper(n) + 1"), SigmaError)
        << "helper is not bound yet";
}

TEST_F(SigmaInteropMoreTests, BindBeforeDefineThenCall) {
    host_->Bind("helper", [](int n) { return n * 10; });
    host_->Run("fun f(n: int) -> int\n    return helper(n) + 1");
    EXPECT_EQ((host_->Call<int>("f", 4)), 41);
}

TEST_F(SigmaInteropMoreTests, GenericHelperOverNativeAndSigmaFunctions) {
    host_->Bind("native_inc", [](int n) { return n + 1; });
    host_->Run("fun sigma_inc(n: int) -> int\n    return n + 1\n"
               "fun apply[T](f: fun(T) -> T, x: T) -> T\n    return f(x)");
    EXPECT_EQ(host_->Eval<int>("apply(native_inc, 1) + apply(sigma_inc, 1)"), 4);
}

TEST_F(SigmaInteropMoreTests, VariadicSumWithNativeInTheMiddle) {
    host_->Bind("ten", []() { return 10; });
    host_->Run("fun total[...Ts](xs: Ts...) -> int\n    return (xs + ... + 0)");
    EXPECT_EQ(host_->Eval<int>("total(1, ten(), 2)"), 13);
}

TEST_F(SigmaInteropMoreTests, CallbackCapturedBeforeLaterRunsStillWorks) {
    host_->Run("fun f(n: int) -> int\n    return n + 1");
    auto f = host_->Callback<int(int)>("f");
    host_->Run("unrelated = 5");
    EXPECT_EQ(f(1), 2);
}

TEST_F(SigmaInteropMoreTests, CallbackUsedManyTimes) {
    host_->Run("fun f(n: int) -> int\n    return n + 1");
    auto f = host_->Callback<int(int)>("f");
    int v = 0;
    for (int i = 0; i < 200; ++i) v = f(v);
    EXPECT_EQ(v, 200);
}

// --- objects and lifetimes -----------------------------------------------------

TEST_F(SigmaInteropMoreTests, FunctionReturningANativeObject) {
    BindPt();
    host_->Run("fun make(n: int) -> Pt\n    return Pt(n, n * 2)");
    EXPECT_EQ(host_->Eval<int>("make(3).y"), 6) << "indexing a call result is not supported";
}

TEST_F(SigmaInteropMoreTests, FunctionReturningANativeObjectUsedViaVariable) {
    BindPt();
    host_->Run("fun make(n: int) -> Pt\n    return Pt(n, n * 2)");
    EXPECT_EQ(host_->Eval<int>("p = make(3)\np.y"), 6);
}

TEST_F(SigmaInteropMoreTests, SigmaFunctionMutatesItsNativeArgument) {
    BindPt();
    host_->Run("fun grow(p: Pt)\n    p.x = p.x + 1");
    EXPECT_EQ(host_->Eval<int>("p = Pt(1, 0)\ngrow(p)\ngrow(p)\np.x"), 3);
}

TEST_F(SigmaInteropMoreTests, NativeObjectInAMap) {
    BindPt();
    EXPECT_EQ(host_->Eval<int>("m: Map[str, Pt] = {\"a\": Pt(1, 2)}\nq = m[\"a\"]\nq.y"), 2);
}

TEST_F(SigmaInteropMoreTests, ObjectPublishedWithSetIsUsableByName) {
    BindPt();
    Pointer<Pt> shared = reg_->New<Pt>();
    shared->x = 9;
    host_->Set("shared", shared);
    EXPECT_EQ(host_->Eval<int>("shared.x"), 9);
}

TEST_F(SigmaInteropMoreTests, SigmaWritesToAPublishedObjectAreVisibleInCpp) {
    BindPt();
    Pointer<Pt> shared = reg_->New<Pt>();
    host_->Set("shared", shared);
    host_->Run("shared.x = 12\nshared.y = 34");
    EXPECT_EQ(shared->x, 12);
    EXPECT_EQ(shared->y, 34);
}

TEST_F(SigmaInteropMoreTests, CppWritesToAPublishedObjectAreVisibleInSigma) {
    BindPt();
    Pointer<Pt> shared = reg_->New<Pt>();
    host_->Set("shared", shared);
    shared->x = 5;
    EXPECT_EQ(host_->Eval<int>("shared.x * 2"), 10);
}

TEST_F(SigmaInteropMoreTests, MethodOnAPublishedObject) {
    BindPt();
    Pointer<Pt> shared = reg_->New<Pt>();
    shared->x = 2;
    shared->y = 3;
    host_->Set("shared", shared);
    EXPECT_EQ(host_->Eval<int>("shared.Dot2(10, 100)"), 320);
}

TEST_F(SigmaInteropMoreTests, MyStructStringPropertyRoundTrips) {
    MyStruct::Register(*reg_);
    Pointer<MyStruct> s = reg_->New<MyStruct>();
    host_->Set("s", s);
    host_->Run("s.string = \"hello\"");
    EXPECT_EQ(s->string, String("hello"));
    EXPECT_EQ(host_->Eval<String>("s.string"), String("hello"));
}

TEST_F(SigmaInteropMoreTests, MyStructMethodWithStringAndInt) {
    MyStruct::Register(*reg_);
    Pointer<MyStruct> s = reg_->New<MyStruct>();
    host_->Set("s", s);
    EXPECT_EQ(host_->Eval<String>("s.Method1(2, \"xy\")"), String("xyxy"));
}

// --- compile-time checking of native signatures --------------------------------

TEST_F(SigmaInteropMoreTests, NativeFunctionValueHasItsSignatureType) {
    host_->Bind("add", [](int a, int b) { return a + b; });
    EXPECT_THROW(host_->Run("f: fun(int) -> int = add"), SigmaError);
    EXPECT_NO_THROW(host_->Run("g: fun(int, int) -> int = add"));
}

TEST_F(SigmaInteropMoreTests, NativeFunctionCannotBeUsedAsAnInt) {
    host_->Bind("add", [](int a, int b) { return a + b; });
    EXPECT_THROW(host_->Run("n: int = add"), SigmaError);
}

TEST_F(SigmaInteropMoreTests, CallingANativeWithTooManyArgumentsFailsToCompile) {
    host_->Bind("one", [](int a) { return a; });
    EXPECT_THROW(host_->Run("one(1, 2)"), SigmaError);
}

TEST_F(SigmaInteropMoreTests, BindingTwiceReplacesTheFunction) {
    host_->Bind("v", []() { return 1; });
    EXPECT_EQ(host_->Eval<int>("v()"), 1);
    host_->Bind("v", []() { return 2; });
    EXPECT_EQ(host_->Eval<int>("v()"), 2);
}

TEST_F(SigmaInteropMoreTests, ErrorsFromOneRunDoNotPoisonTheNext) {
    EXPECT_THROW(host_->Run("x: int = \"s\""), SigmaError);
    EXPECT_EQ(host_->Eval<int>("1 + 1"), 2);
}

TEST_F(SigmaInteropMoreTests, EveryDiagnosticIsReportedTogether) {
    try {
        host_->Run("a: int = \"s\"\nb: str = 1\n");
        FAIL();
    } catch (const SigmaError &e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("1:10"), std::string::npos) << what;
        EXPECT_NE(what.find("2:10"), std::string::npos) << what;
    }
}
