#include <KAI/Core/Event.h>

#include <string>
#include <vector>

#include "TestCommon.h"

USING_NAMESPACE_KAI

// Tests for KAI's variadic-template machinery:
//   - function_detail::AddArgType   (Detail/AddArgType.h)
//   - detail::CallFun / CallMethod  (Detail/CallableBase.h)
//   - Function / MakeFunction       (Detail/Function.h)
//   - Method / MakeMethod           (Object/Method.h)
//   - Event<Args...>                (Event.h)
//
// Arguments are always distinct values so a swapped or shifted argument
// fails the test; several existing tests used identical ints and could not.

namespace {

struct Counter {
    int value = 0;
    String label;

    void Bump() { ++value; }
    void Add(int n) { value += n; }
    void Set(int a, int b, int c) { value = a * 100 + b * 10 + c; }
    void SetLabel(String const& s) { label = s; }
    int Get() const { return value; }
    int Scaled(int k) const { return value * k; }
    int Combine(int a, int b) const { return value + a * 10 + b; }
    int AddAndGet(int n) {
        value += n;
        return value;
    }
    String Describe(String const& prefix, int n) const {
        return prefix + String(std::to_string(n).c_str());
    }
    void Peek(int n) const { lastPeek = n; }
    mutable int lastPeek = 0;
};

struct Listener {
    std::vector<int> seen;
    std::vector<std::string> strings;
    void OnInt(int n) { seen.push_back(n); }
    void OnPair(int a, int b) { seen.push_back(a * 10 + b); }
    void OnString(std::string s) { strings.push_back(s); }
};

struct OtherListener {
    std::vector<int> seen;
    void OnInt(int n) { seen.push_back(n + 1000); }
};

}  // namespace

KAI_BEGIN
KAI_TYPE_TRAITS_BASIC(Counter, 1701);
KAI_TYPE_TRAITS_BASIC(Listener, 1702);
KAI_END

namespace {

// Free functions for MakeFunction. Each records its arguments so tests can
// check order, not just that a call happened.
std::vector<int> g_ints;
std::vector<std::string> g_strings;
int g_calls = 0;

void V0() { ++g_calls; }
void V1(int a) { g_ints = {a}; }
void V2(int a, int b) { g_ints = {a, b}; }
void V3(int a, int b, int c) { g_ints = {a, b, c}; }
void V4(int a, int b, int c, int d) { g_ints = {a, b, c, d}; }
void VMixed(int a, String s, float f, bool b) {
    g_ints = {a, static_cast<int>(f * 10), b ? 1 : 0};
    g_strings = {s.CStr()};
}

int R0() { return 7; }
int R1(int a) { return a * 2; }
int R2(int a, int b) { return a * 10 + b; }
int R3(int a, int b, int c) { return a * 100 + b * 10 + c; }
int R5(int a, int b, int c, int d, int e) {
    return a * 10000 + b * 1000 + c * 100 + d * 10 + e;
}
float RFloat(float a, int b) { return a * b; }
bool RBool(int a, int b) { return a < b; }
String RString(String a, String b) { return a + b; }
int RSub(int a, int b) { return a - b; }

void Push(Registry& reg, Stack& s, int v) { s.Push(reg.New<int>(v)); }

std::vector<int> g_eventInts;
std::vector<std::string> g_eventStrings;
void EvA(int n) { g_eventInts.push_back(n); }
void EvB(int n) { g_eventInts.push_back(n * 100); }
void EvStrA(std::string s) { g_eventStrings.push_back("a:" + s); }
void EvStrB(std::string s) { g_eventStrings.push_back("b:" + s); }
void EvStrC(std::string s) { g_eventStrings.push_back("c:" + s); }
int g_ev0 = 0;
void Ev0() { ++g_ev0; }
void Ev3(int a, int b, int c) { g_eventInts.push_back(a * 100 + b * 10 + c); }

}  // namespace

struct TestVariadicTemplates : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<Stack>();
        Reg().AddClass<Counter>();
        Reg().AddClass<Listener>();
    }

    void SetUp() override {
        TestCommon::SetUp();
        stack_ = Reg().New<Stack>();
        Root().Set(Label("__stack"), stack_);
        g_ints.clear();
        g_strings.clear();
        g_calls = 0;
        g_eventInts.clear();
        g_eventStrings.clear();
        g_ev0 = 0;
    }

    Stack& S() { return *stack_; }
    void PushInts(std::initializer_list<int> values) {
        for (int v : values) Push(Reg(), S(), v);
    }
    int PopInt() { return ConstDeref<int>(S().Pop()); }

    Pointer<Counter> NewCounter(int value) {
        Pointer<Counter> c = Reg().New<Counter>();
        c->value = value;
        return c;
    }

    Pointer<Stack> stack_;
};

// ---------------------------------------------------------------------------
// AddArgType: records one Type::Number per argument, in declaration order.

TEST_F(TestVariadicTemplates, AddArgTypeZeroArgsRecordsNothing) {
    std::vector<Type::Number> types;
    function_detail::AddArgType<0>::Add(types);
    EXPECT_TRUE(types.empty());
}

TEST_F(TestVariadicTemplates, AddArgTypeSingleArg) {
    std::vector<Type::Number> types;
    function_detail::AddArgType<1, float>::Add(types);
    ASSERT_EQ(types.size(), 1u);
    EXPECT_EQ(types[0], Type::Traits<float>::Number);
}

TEST_F(TestVariadicTemplates, AddArgTypePreservesOrder) {
    std::vector<Type::Number> types;
    function_detail::AddArgType<4, int, String, float, bool>::Add(types);
    ASSERT_EQ(types.size(), 4u);
    EXPECT_EQ(types[0], Type::Traits<int>::Number);
    EXPECT_EQ(types[1], Type::Traits<String>::Number);
    EXPECT_EQ(types[2], Type::Traits<float>::Number);
    EXPECT_EQ(types[3], Type::Traits<bool>::Number);
}

// ---------------------------------------------------------------------------
// CallFun / CallMethod: expand a tuple into a call, preserving order.

TEST_F(TestVariadicTemplates, CallFunEmptyTuple) {
    auto result = detail::CallFun([] { return 42; }, std::tuple<>{});
    EXPECT_EQ(result, 42);
}

TEST_F(TestVariadicTemplates, CallFunExpandsInOrder) {
    auto result = detail::CallFun(R3, std::make_tuple(1, 2, 3));
    EXPECT_EQ(result, 123);
}

TEST_F(TestVariadicTemplates, CallMethodExpandsInOrder) {
    Counter c;
    c.value = 5;
    EXPECT_EQ(detail::CallMethod(c, &Counter::Combine, std::make_tuple(3, 4)), 5 + 30 + 4);
}

TEST_F(TestVariadicTemplates, CallMethodMutates) {
    Counter c;
    detail::CallMethod(c, &Counter::Set, std::make_tuple(7, 8, 9));
    EXPECT_EQ(c.value, 789);
}

// ---------------------------------------------------------------------------
// Function / MakeFunction: binding free functions of any arity.

TEST_F(TestVariadicTemplates, FunctionVoidZeroArgs) {
    std::unique_ptr<FunctionBase> f(MakeFunction(V0));
    f->Invoke(Reg(), S());
    EXPECT_EQ(g_calls, 1);
    EXPECT_EQ(S().Size(), 0);
}

TEST_F(TestVariadicTemplates, FunctionVoidArgumentOrder) {
    // The last argument is on top of the stack.
    std::unique_ptr<FunctionBase> f(MakeFunction(V3));
    PushInts({1, 2, 3});
    f->Invoke(Reg(), S());
    EXPECT_EQ(g_ints, (std::vector<int>{1, 2, 3}));
}

TEST_F(TestVariadicTemplates, FunctionVoidFourArgs) {
    std::unique_ptr<FunctionBase> f(MakeFunction(V4));
    PushInts({9, 8, 7, 6});
    f->Invoke(Reg(), S());
    EXPECT_EQ(g_ints, (std::vector<int>{9, 8, 7, 6}));
}

TEST_F(TestVariadicTemplates, FunctionVoidConsumesExactlyItsArgs) {
    std::unique_ptr<FunctionBase> f(MakeFunction(V2));
    PushInts({100, 1, 2});
    f->Invoke(Reg(), S());
    ASSERT_EQ(S().Size(), 1);
    EXPECT_EQ(PopInt(), 100);
}

TEST_F(TestVariadicTemplates, FunctionNonVoidZeroArgs) {
    std::unique_ptr<FunctionBase> f(MakeFunction(R0));
    f->Invoke(Reg(), S());
    ASSERT_EQ(S().Size(), 1);
    EXPECT_EQ(PopInt(), 7);
}

TEST_F(TestVariadicTemplates, FunctionNonVoidPushesResult) {
    std::unique_ptr<FunctionBase> f(MakeFunction(R1));
    PushInts({21});
    f->Invoke(Reg(), S());
    ASSERT_EQ(S().Size(), 1);
    EXPECT_EQ(PopInt(), 42);
}

TEST_F(TestVariadicTemplates, FunctionNonVoidArgumentOrder) {
    std::unique_ptr<FunctionBase> f(MakeFunction(RSub));
    PushInts({10, 3});
    f->Invoke(Reg(), S());
    EXPECT_EQ(PopInt(), 7);  // 10 - 3, not 3 - 10
}

TEST_F(TestVariadicTemplates, FunctionFiveArgs) {
    std::unique_ptr<FunctionBase> f(MakeFunction(R5));
    PushInts({1, 2, 3, 4, 5});
    f->Invoke(Reg(), S());
    EXPECT_EQ(PopInt(), 12345);
}

TEST_F(TestVariadicTemplates, FunctionMixedArgumentTypes) {
    std::unique_ptr<FunctionBase> f(MakeFunction(VMixed));
    S().Push(Reg().New<int>(3));
    S().Push(Reg().New<String>("text"));
    S().Push(Reg().New<float>(2.5f));
    S().Push(Reg().New<bool>(true));
    f->Invoke(Reg(), S());
    EXPECT_EQ(g_ints, (std::vector<int>{3, 25, 1}));
    EXPECT_EQ(g_strings, (std::vector<std::string>{"text"}));
}

TEST_F(TestVariadicTemplates, FunctionFloatReturn) {
    std::unique_ptr<FunctionBase> f(MakeFunction(RFloat));
    S().Push(Reg().New<float>(1.5f));
    S().Push(Reg().New<int>(4));
    f->Invoke(Reg(), S());
    EXPECT_FLOAT_EQ(ConstDeref<float>(S().Pop()), 6.0f);
}

TEST_F(TestVariadicTemplates, FunctionBoolReturn) {
    std::unique_ptr<FunctionBase> f(MakeFunction(RBool));
    PushInts({1, 2});
    f->Invoke(Reg(), S());
    EXPECT_TRUE(ConstDeref<bool>(S().Pop()));
    PushInts({2, 1});
    f->Invoke(Reg(), S());
    EXPECT_FALSE(ConstDeref<bool>(S().Pop()));
}

TEST_F(TestVariadicTemplates, FunctionStringArgsAndReturn) {
    std::unique_ptr<FunctionBase> f(MakeFunction(RString));
    S().Push(Reg().New<String>("foo"));
    S().Push(Reg().New<String>("bar"));
    f->Invoke(Reg(), S());
    EXPECT_STREQ(ConstDeref<String>(S().Pop()).CStr(), "foobar");
}

TEST_F(TestVariadicTemplates, FunctionRecordsSignature) {
    std::unique_ptr<FunctionBase> f(MakeFunction(RFloat));
    EXPECT_EQ(f->GetReturnType(), Type::Traits<float>::Number);
    ASSERT_EQ(f->GetArgumentTypes().size(), 2u);
    EXPECT_EQ(f->GetArgumentType(0), Type::Traits<float>::Number);
    EXPECT_EQ(f->GetArgumentType(1), Type::Traits<int>::Number);
}

TEST_F(TestVariadicTemplates, FunctionVoidRecordsVoidReturn) {
    std::unique_ptr<FunctionBase> f(MakeFunction(V2));
    EXPECT_EQ(f->GetReturnType(), Type::Traits<void>::Number);
    EXPECT_EQ(f->GetArgumentTypes().size(), 2u);
}

TEST_F(TestVariadicTemplates, FunctionNameAndDescription) {
    std::unique_ptr<FunctionBase> f(MakeFunction(R2, Label("combine"), "joins digits"));
    EXPECT_EQ(f->GetName(), Label("combine"));
    EXPECT_STREQ(f->description.CStr(), "joins digits");
}

TEST_F(TestVariadicTemplates, FunctionFromStdFunctionWithCapture) {
    int offset = 1000;
    std::function<int(int, int)> fn = [offset](int a, int b) { return offset + a * 10 + b; };
    std::unique_ptr<FunctionBase> f(MakeFunction(fn));
    PushInts({4, 2});
    f->Invoke(Reg(), S());
    EXPECT_EQ(PopInt(), 1042);
}

TEST_F(TestVariadicTemplates, FunctionRepeatedInvocationsUseFreshArgs) {
    // NonVoidFun keeps its argument tuple as a member; values from one call
    // must not leak into the next.
    std::unique_ptr<FunctionBase> f(MakeFunction(R2));
    PushInts({1, 2});
    f->Invoke(Reg(), S());
    EXPECT_EQ(PopInt(), 12);
    PushInts({3, 4});
    f->Invoke(Reg(), S());
    EXPECT_EQ(PopInt(), 34);
}

// ---------------------------------------------------------------------------
// Method / MakeMethod: binding member functions, const and non-const.

TEST_F(TestVariadicTemplates, MethodVoidNoArgsMutates) {
    auto c = NewCounter(0);
    auto m = MakeMethod(&Counter::Bump, Label("Bump"));
    m->Invoke(c, S());
    m->Invoke(c, S());
    EXPECT_EQ(c->value, 2);
    EXPECT_EQ(S().Size(), 0);
}

TEST_F(TestVariadicTemplates, MethodVoidArgumentOrder) {
    auto c = NewCounter(0);
    auto m = MakeMethod(&Counter::Set, Label("Set"));
    PushInts({1, 2, 3});
    m->Invoke(c, S());
    EXPECT_EQ(c->value, 123);
}

TEST_F(TestVariadicTemplates, MethodConstReturnNoArgs) {
    auto c = NewCounter(77);
    auto m = MakeMethod(&Counter::Get, Label("Get"));
    m->Invoke(c, S());
    ASSERT_EQ(S().Size(), 1);
    EXPECT_EQ(PopInt(), 77);
}

TEST_F(TestVariadicTemplates, MethodConstReturnOneArg) {
    auto c = NewCounter(6);
    auto m = MakeMethod(&Counter::Scaled, Label("Scaled"));
    PushInts({7});
    m->Invoke(c, S());
    EXPECT_EQ(PopInt(), 42);
}

TEST_F(TestVariadicTemplates, MethodConstReturnArgumentOrder) {
    auto c = NewCounter(500);
    auto m = MakeMethod(&Counter::Combine, Label("Combine"));
    PushInts({3, 4});
    m->Invoke(c, S());
    EXPECT_EQ(PopInt(), 534);
}

TEST_F(TestVariadicTemplates, MethodNonConstReturnMutatesAndPushes) {
    auto c = NewCounter(1);
    auto m = MakeMethod(&Counter::AddAndGet, Label("AddAndGet"));
    PushInts({9});
    m->Invoke(c, S());
    EXPECT_EQ(PopInt(), 10);
    EXPECT_EQ(c->value, 10);
}

TEST_F(TestVariadicTemplates, MethodConstVoid) {
    auto c = NewCounter(0);
    auto m = MakeMethod(&Counter::Peek, Label("Peek"));
    PushInts({31});
    m->Invoke(c, S());
    EXPECT_EQ(c->lastPeek, 31);
    EXPECT_EQ(S().Size(), 0);
}

TEST_F(TestVariadicTemplates, MethodConstRefStringArg) {
    // Method decays its argument types, so a const& parameter binds.
    auto c = NewCounter(0);
    auto m = MakeMethod(&Counter::SetLabel, Label("SetLabel"));
    S().Push(Reg().New<String>("named"));
    m->Invoke(c, S());
    EXPECT_STREQ(c->label.CStr(), "named");
}

TEST_F(TestVariadicTemplates, MethodMixedArgsAndStringReturn) {
    auto c = NewCounter(0);
    auto m = MakeMethod(&Counter::Describe, Label("Describe"));
    S().Push(Reg().New<String>("n="));
    PushInts({42});
    m->Invoke(c, S());
    EXPECT_STREQ(ConstDeref<String>(S().Pop()).CStr(), "n=42");
}

TEST_F(TestVariadicTemplates, MethodConsumesExactlyItsArgs) {
    auto c = NewCounter(0);
    auto m = MakeMethod(&Counter::Set, Label("Set"));
    PushInts({99, 1, 2, 3});
    m->Invoke(c, S());
    ASSERT_EQ(S().Size(), 1);
    EXPECT_EQ(PopInt(), 99);
}

TEST_F(TestVariadicTemplates, MethodRecordsSignature) {
    auto m = MakeMethod(&Counter::Describe, Label("Describe"));
    EXPECT_EQ(m->GetClassType(), Type::Traits<Counter>::Number);
    EXPECT_EQ(m->GetReturnType(), Type::Traits<String>::Number);
    ASSERT_EQ(m->GetArgumentTypes().size(), 2u);
    EXPECT_EQ(m->GetArgumentType(0), Type::Traits<String>::Number);  // decayed
    EXPECT_EQ(m->GetArgumentType(1), Type::Traits<int>::Number);
}

TEST_F(TestVariadicTemplates, MethodRecordsConstness) {
    EXPECT_EQ(MakeMethod(&Counter::Get, Label("Get"))->GetConstness(), Constness::Const);
    EXPECT_EQ(MakeMethod(&Counter::Bump, Label("Bump"))->GetConstness(), Constness::Mutable);
}

TEST_F(TestVariadicTemplates, ConstMethodCallableOnConstObject) {
    auto c = NewCounter(8);
    c.SetConst();
    auto m = MakeMethod(&Counter::Scaled, Label("Scaled"));
    PushInts({3});
    m->Invoke(c, S());
    EXPECT_EQ(PopInt(), 24);
}

TEST_F(TestVariadicTemplates, MethodRepeatedInvocationsUseFreshArgs) {
    auto c = NewCounter(0);
    auto m = MakeMethod(&Counter::Combine, Label("Combine"));
    PushInts({1, 2});
    m->Invoke(c, S());
    EXPECT_EQ(PopInt(), 12);
    PushInts({5, 6});
    m->Invoke(c, S());
    EXPECT_EQ(PopInt(), 56);
}

// ---------------------------------------------------------------------------
// Event<Args...>: the real multi-cast event in KAI/Core/Event.h. (TestEvents
// exercises a test-local mock in EventHelper.h, not this class.)

TEST_F(TestVariadicTemplates, EventNoArgs) {
    Event<> e;
    e += Ev0;
    e();
    e();
    EXPECT_EQ(g_ev0, 2);
}

TEST_F(TestVariadicTemplates, EventInvokesSinksInRegistrationOrder) {
    Event<int> e;
    e += EvA;
    e += EvB;
    e(3);
    EXPECT_EQ(g_eventInts, (std::vector<int>{3, 300}));
}

TEST_F(TestVariadicTemplates, EventThreeArgsInOrder) {
    Event<int, int, int> e;
    e += Ev3;
    e(4, 5, 6);
    EXPECT_EQ(g_eventInts, (std::vector<int>{456}));
}

TEST_F(TestVariadicTemplates, EventEverySinkReceivesTheArguments) {
    // Arguments were std::forward-ed inside the loop over sinks, so every
    // listener after the first received a moved-from string.
    Event<std::string> e;
    e += EvStrA;
    e += EvStrB;
    e += EvStrC;
    e(std::string("hello"));
    EXPECT_EQ(g_eventStrings,
              (std::vector<std::string>{"a:hello", "b:hello", "c:hello"}));
}

TEST_F(TestVariadicTemplates, EventCapturingLambda) {
    int total = 0;
    Event<int, int> e;
    e += [&total](int a, int b) { total += a * b; };
    e(6, 7);
    EXPECT_EQ(total, 42);
}

TEST_F(TestVariadicTemplates, EventRemoveFunction) {
    Event<int> e;
    e += EvA;
    e += EvB;
    e -= EvA;
    e(2);
    EXPECT_EQ(g_eventInts, (std::vector<int>{200}));
}

TEST_F(TestVariadicTemplates, EventRemoveOnlyOneOfDuplicates) {
    Event<int> e;
    e += EvA;
    e += EvA;
    e -= EvA;
    e(5);
    EXPECT_EQ(g_eventInts, (std::vector<int>{5}));
}

TEST_F(TestVariadicTemplates, EventRemovingALambdaDoesNotCrash) {
    // FunctionDelegate::equals dereferenced target<void(*)(Args...)>()
    // unconditionally; for a lambda that is null. std::function cannot
    // compare closures, so removal by lambda is a no-op.
    int calls = 0;
    auto f = [&calls](int) { ++calls; };
    Event<int> e;
    e += f;
    e -= f;
    e(1);
    EXPECT_EQ(calls, 1);
}

TEST_F(TestVariadicTemplates, EventBoundMethod) {
    Listener l;
    Event<int, int> e;
    e += std::pair(&l, &Listener::OnPair);
    e(3, 4);
    EXPECT_EQ(l.seen, (std::vector<int>{34}));
}

TEST_F(TestVariadicTemplates, EventRemoveBoundMethodLeavesOtherClasses) {
    Listener l;
    OtherListener o;
    Event<int> e;
    e += std::pair(&l, &Listener::OnInt);
    e += std::pair(&o, &OtherListener::OnInt);
    e -= std::pair(&o, &OtherListener::OnInt);
    e(9);
    EXPECT_EQ(l.seen, (std::vector<int>{9}));
    EXPECT_TRUE(o.seen.empty());
}

TEST_F(TestVariadicTemplates, EventObjectMethodSkipsDeletedObject) {
    Pointer<Listener> l = Reg().New<Listener>();
    Root().Set(Label("listener"), l);
    Event<int> e;
    e += std::pair(Object(l), &Listener::OnInt);
    e(1);
    ASSERT_EQ(l->seen, (std::vector<int>{1}));

    Root().Remove(Label("listener"));
    l.Delete();
    for (int n = 0; n < 100; ++n) Reg().GarbageCollect();
    ASSERT_FALSE(l.Exists());
    e(2);  // must not touch the dead object
}

TEST_F(TestVariadicTemplates, EventRemoveObjectMethodMatchesIdentityNotValue) {
    // Two distinct Listener objects compare equal by value (no Equiv, same
    // shape), so removing one by value removed whichever came first.
    Pointer<Listener> first = Reg().New<Listener>();
    Pointer<Listener> second = Reg().New<Listener>();
    Root().Set(Label("first"), first);
    Root().Set(Label("second"), second);
    Event<int> e;
    e += std::pair(Object(first), &Listener::OnInt);
    e += std::pair(Object(second), &Listener::OnInt);
    e -= std::pair(Object(second), &Listener::OnInt);
    e(7);
    EXPECT_EQ(first->seen, (std::vector<int>{7}));
    EXPECT_TRUE(second->seen.empty());
}
