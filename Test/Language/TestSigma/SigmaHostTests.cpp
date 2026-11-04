// The C++ side of Sigma: SigmaHost binds functions and types, sets values,
// runs programs, reads typed results and wraps Sigma functions as callbacks.

#include <gtest/gtest.h>

#include <algorithm>

#include "KAI/Language/Sigma/SigmaHost.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace kai;

KAI_BEGIN
struct Point {
    int x = 0;
    int y = 0;
    int Sum() const { return x + y; }
    static void Register(Registry &r) {
        ClassBuilder<Point>(r, "Point")
            .methods("Sum", &Point::Sum, "x + y")
            .properties("x", &Point::x, "x")("y", &Point::y, "y");
    }
};
KAI_TYPE_TRAITS(Point, 777, Properties::Nothing);
KAI_END

struct SigmaHostTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Sigma);
        host_ = std::make_unique<SigmaHost>(console_);
    }
    std::unique_ptr<SigmaHost> host_;
};

TEST_F(SigmaHostTests, EvalReturnsATypedResult) {
    EXPECT_EQ(host_->Eval<int>("6 * 7"), 42);
    EXPECT_FLOAT_EQ(host_->Eval<float>("7 / 2.0"), 3.5f);
    EXPECT_TRUE(host_->Eval<bool>("1 < 2"));
    EXPECT_EQ(host_->Eval<String>("\"a\" + \"b\""), String("ab"));
}

TEST_F(SigmaHostTests, EvalWithTheWrongTypeThrows) {
    EXPECT_THROW(host_->Eval<float>("1 + 2"), SigmaError);
}

TEST_F(SigmaHostTests, CompileErrorsThrowWithPositions) {
    try {
        host_->Run("x: int = \"s\"");
        FAIL() << "expected SigmaError";
    } catch (const SigmaError &e) {
        EXPECT_NE(std::string(e.what()).find("1:10: cannot initialise 'x': expected int, got str"), std::string::npos)
            << e.what();
    }
}

TEST_F(SigmaHostTests, BindsALambda) {
    host_->Bind("clamp", [](int v, int lo, int hi) { return std::clamp(v, lo, hi); });
    EXPECT_EQ(host_->Eval<int>("clamp(99, 0, 15)"), 15);
}

TEST_F(SigmaHostTests, BindsAFreeFunction) {
    host_->Bind("negate", +[](int v) { return -v; });
    EXPECT_EQ(host_->Eval<int>("negate(5)"), -5);
}

TEST_F(SigmaHostTests, BoundFunctionsAreTypeChecked) {
    host_->Bind("clamp", [](int v, int lo, int hi) { return std::clamp(v, lo, hi); });
    EXPECT_THROW(host_->Run("clamp(1, 2)"), SigmaError);
    EXPECT_THROW(host_->Run("clamp(1, 2, \"x\")"), SigmaError);
}

TEST_F(SigmaHostTests, BoundVoidFunctionRunsForItsEffect) {
    int seen = 0;
    host_->Bind("record", [&seen](int v) { seen = v; });
    host_->Run("record(11)");
    EXPECT_EQ(seen, 11);
}

TEST_F(SigmaHostTests, SetPublishesAValue) {
    host_->Set("limit", 10);
    EXPECT_EQ(host_->Eval<int>("limit * 3"), 30);
}

TEST_F(SigmaHostTests, SetStringIsUsable) {
    host_->Set("who", String("world"));
    EXPECT_EQ(host_->Eval<String>("\"hello \" + who"), String("hello world"));
}

TEST_F(SigmaHostTests, ProgramsShareDefinitions) {
    host_->Run("fun double_it(n: int) -> int\n    return n * 2");
    EXPECT_EQ(host_->Eval<int>("double_it(21)"), 42);
}

TEST_F(SigmaHostTests, CallInvokesASigmaFunction) {
    host_->Run("fun area(w: int, h: int) -> int\n    return w * h");
    EXPECT_EQ((host_->Call<int>("area", 6, 7)), 42);
}

TEST_F(SigmaHostTests, CallPassesFloatsAndStrings) {
    host_->Run("fun scale(x: float, k: float) -> float\n    return x * k\n"
               "fun tag(s: str) -> str\n    return s + \"!\"");
    EXPECT_FLOAT_EQ((host_->Call<float>("scale", 1.5f, 4.0f)), 6.0f);
    EXPECT_EQ((host_->Call<String>("tag", "hey")), String("hey!"));
}

TEST_F(SigmaHostTests, CallbackWrapsASigmaFunction) {
    host_->Run("fun square(n: int) -> int\n    return n * n");
    auto square = host_->Callback<int(int)>("square");
    int total = 0;
    for (int i = 1; i <= 4; ++i) total += square(i);
    EXPECT_EQ(total, 30);
}

TEST_F(SigmaHostTests, CallbackCanCallBackIntoBoundCpp) {
    host_->Bind("offset", [] { return 100; });
    host_->Run("fun shifted(n: int) -> int\n    return n + offset()");
    EXPECT_EQ((host_->Callback<int(int)>("shifted")(5)), 105);
}

TEST_F(SigmaHostTests, ConstructsANativeObject) {
    Point::Register(*reg_);
    host_->Construct<Point>("Point");
    EXPECT_EQ(host_->Eval<int>("p = Point()\np.x"), 0);
}

TEST_F(SigmaHostTests, ConstructorWithArgumentsIsAFactory) {
    Point::Register(*reg_);
    host_->Bind("Point", [](int x, int y) {
        Point p;
        p.x = x;
        p.y = y;
        return p;
    });
    EXPECT_EQ(host_->Eval<int>("p = Point(3, 4)\np.x * 10 + p.y"), 34);
}

TEST_F(SigmaHostTests, ConstructorArgumentsAreChecked) {
    Point::Register(*reg_);
    host_->Bind("Point", [](int x, int y) {
        Point p;
        p.x = x;
        p.y = y;
        return p;
    });
    EXPECT_THROW(host_->Run("p = Point(1)"), SigmaError);
}

TEST_F(SigmaHostTests, NativeObjectPassesToSigmaFunction) {
    Point::Register(*reg_);
    host_->Bind("Point", [](int x, int y) {
        Point p;
        p.x = x;
        p.y = y;
        return p;
    });
    host_->Run("fun manhattan(p: Point) -> int\n    return p.x + p.y");
    EXPECT_EQ(host_->Eval<int>("manhattan(Point(3, 4))"), 7);
}

TEST_F(SigmaHostTests, GetReadsATopLevelValue) {
    host_->Run("total = 5 + 6");
    EXPECT_EQ(host_->Get<int>("total"), 11);
}

TEST_F(SigmaHostTests, NativeMethodRunsOnAConstructedObject) {
    Point::Register(*reg_);
    host_->Bind("Point", [](int x, int y) {
        Point p;
        p.x = x;
        p.y = y;
        return p;
    });
    EXPECT_EQ(host_->Eval<int>("p = Point(3, 4)\np.Sum()"), 7);
}

TEST_F(SigmaHostTests, NativePropertyCanBeAssigned) {
    Point::Register(*reg_);
    host_->Construct<Point>("Point");
    EXPECT_EQ(host_->Eval<int>("p = Point()\np.x = 9\np.x"), 9);
}

