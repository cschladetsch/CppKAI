// Sigma calling into C++: free functions registered with AddFunction, objects
// the host placed in the tree, and native types.

#include <gtest/gtest.h>

#include "KAI/Language/Sigma/SigmaTranslator.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace kai;

namespace {
int gNoted = 0;
int AddInts(int a, int b) { return a + b; }
float HalfOf(float x) { return x / 2.0f; }
bool IsEven(int n) { return n % 2 == 0; }
String Shout(String s) { return s + String("!"); }
void Note(int n) { gNoted = n; }
}  // namespace

struct SigmaNativeTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Sigma);
        console_.GetTree().AddSearchPath(root_);
        AddFunction(root_, AddInts, Label("add_ints"));
        AddFunction(root_, HalfOf, Label("half_of"));
        AddFunction(root_, IsEven, Label("is_even"));
        AddFunction(root_, Shout, Label("shout"));
        AddFunction(root_, Note, Label("note"));
        gNoted = 0;
    }

    std::string Errors(const std::string &code) {
        SigmaTranslator sigma(*reg_);
        sigma.Compile(code.c_str());
        std::string all;
        for (auto const &e : sigma.GetErrors()) all += e + "\n";
        return all;
    }

    template <class T>
    T Run(const std::string &code) {
        data_->Clear();
        console_.Execute(code.c_str(), Structure::Program);
        EXPECT_FALSE(data_->Empty()) << code << "\n" << Errors(code);
        if (data_->Empty()) return T();
        Object top = data_->Top();
        EXPECT_TRUE(top.IsType<T>()) << "wrong result type for:\n" << code;
        return top.IsType<T>() ? ConstDeref<T>(top) : T();
    }
};

TEST_F(SigmaNativeTests, CallsAFreeFunction) {
    EXPECT_EQ(Run<int>("add_ints(2, 3)"), 5);
}

TEST_F(SigmaNativeTests, ResultFeedsFurtherExpressions) {
    EXPECT_EQ(Run<int>("add_ints(add_ints(1, 2), 4) * 2"), 14);
}

TEST_F(SigmaNativeTests, FloatFunctionWidensAnIntArgument) {
    EXPECT_FLOAT_EQ(Run<float>("half_of(5)"), 2.5f);
}

TEST_F(SigmaNativeTests, BoolFunctionDrivesAnIf) {
    EXPECT_EQ(Run<int>("r = 0\nif is_even(4)\n    r = 1\nr"), 1);
}

TEST_F(SigmaNativeTests, StringFunction) {
    EXPECT_EQ(Run<String>("shout(\"hey\")"), String("hey!"));
}

TEST_F(SigmaNativeTests, VoidFunctionRunsForItsEffect) {
    Run<int>("note(7)\n1");
    EXPECT_EQ(gNoted, 7);
}

TEST_F(SigmaNativeTests, ArgumentTypesAreChecked) {
    EXPECT_NE(Errors("add_ints(1, \"x\")").find("argument 2 of 'add_ints': expected int, got str"),
              std::string::npos)
        << Errors("add_ints(1, \"x\")");
}

TEST_F(SigmaNativeTests, ArityIsChecked) {
    EXPECT_NE(Errors("add_ints(1)").find("'add_ints' expects 2 arguments, got 1"), std::string::npos)
        << Errors("add_ints(1)");
}

TEST_F(SigmaNativeTests, ResultTypeIsChecked) {
    EXPECT_NE(Errors("s: str = add_ints(1, 2)").find("expected str, got int"), std::string::npos)
        << Errors("s: str = add_ints(1, 2)");
}

TEST_F(SigmaNativeTests, ACallWorksInsideASigmaFunction) {
    EXPECT_EQ(Run<int>("fun twice(n: int) -> int\n    return add_ints(n, n)\ntwice(21)"), 42);
}

TEST_F(SigmaNativeTests, ANativeFunctionCanBeAValue) {
    EXPECT_EQ(Run<int>("f: fun(int, int) -> int = add_ints\nf(4, 5)"), 9);
}

TEST_F(SigmaNativeTests, ANativeFunctionPassesToATemplate) {
    EXPECT_EQ(Run<int>(
        "fun fold[A, B](f: fun(B, A) -> B, init: B, xs: List[A]) -> B\n"
        "    acc = init\n"
        "    for x in xs\n"
        "        acc = f(acc, x)\n"
        "    return acc\n"
        "fold(add_ints, 0, [1, 2, 3, 4])"), 10);
}

TEST_F(SigmaNativeTests, UnknownNamesAreStillUndefined) {
    EXPECT_NE(Errors("x = nope(1)").find("undefined name 'nope'"), std::string::npos);
}

TEST_F(SigmaNativeTests, ASigmaNameShadowsANativeOne) {
    EXPECT_EQ(Run<int>("fun add_ints(a: int, b: int) -> int\n    return a * b\nadd_ints(3, 4)"), 12);
}

TEST_F(SigmaNativeTests, RhoCallsTheSameFunction) {
    console_.SetLanguage(Language::Rho);
    data_->Clear();
    console_.Execute("add_ints(2, 3)", Structure::Program);
    ASSERT_FALSE(data_->Empty());
    EXPECT_EQ(ConstDeref<int>(data_->Top()), 5);
}

TEST_F(SigmaNativeTests, MethodWithArgumentsRunsOnARegisteredObject) {
    MyStruct::Register(*reg_);
    Pointer<MyStruct> s = reg_->New<MyStruct>();
    root_["s"] = s;
    EXPECT_EQ(Run<String>("s.Method1(3, \"ab\")"), String("ababab"));
}

TEST_F(SigmaNativeTests, MethodResultFeedsAnExpression) {
    MyStruct::Register(*reg_);
    Pointer<MyStruct> s = reg_->New<MyStruct>();
    root_["s"] = s;
    EXPECT_EQ(Run<int>("s.Method0() + 1"), 43);
}

TEST_F(SigmaNativeTests, VoidMethodSetsItsFlag) {
    MyStruct::Register(*reg_);
    Pointer<MyStruct> s = reg_->New<MyStruct>();
    root_["s"] = s;
    Run<int>("s.Method2()\n1");
    EXPECT_TRUE(s->called[2]);
}

TEST_F(SigmaNativeTests, PropertyAssignmentReachesTheCppObject) {
    MyStruct::Register(*reg_);
    Pointer<MyStruct> s = reg_->New<MyStruct>();
    root_["s"] = s;
    EXPECT_EQ(Run<int>("s.num = 77\ns.num"), 77);
    EXPECT_EQ(s->num, 77);
}
