#include "EventHelper.h"
#include "TestCommon.h"

USING_NAMESPACE_KAI

struct TestEvents : TestCommon {};

static bool funCalled[4];

void function0() { funCalled[0] = true; }
void function1(int) { funCalled[1] = true; }
void function2(int, String) { funCalled[2] = true; }
void function3(int, String, float) { funCalled[3] = true; }

struct Foobar {
    int methodCalled[4];
    int r1, r2, r3;
    String s2, s3;
    float f3;

    Foobar() { std::fill(methodCalled, methodCalled + 4, false); }

    void method0() { methodCalled[0] = true; }
    void method1(int r) {
        r1 = r;
        methodCalled[1] = true;
    }

    void method2(int r, String s) {
        r2 = r;
        s2 = s;
        methodCalled[2] = true;
    }
    void method3(int r, String s, float f) {
        r3 = r;
        s3 = s;
        f3 = f;
        methodCalled[3] = true;
    }
};

struct Grok {
    bool called;

    Grok() { called = false; }
    void method0() { called = true; };
};

KAI_BEGIN
KAI_TYPE_TRAITS_BASIC(Grok, 666);
KAI_END

TEST_F(TestEvents, TestFunctionsAndMethods) {
    Event<> event0;
    Event<int> event1;
    Event<int, String> event2;
    Event<int, String, float> event3;

    Foobar foo;

    event0.AddMethod(&foo, &Foobar::method0);
    event1.AddMethod(&foo, &Foobar::method1);
    event2.AddMethod(&foo, &Foobar::method2);
    event3.AddMethod(&foo, &Foobar::method3);

    event0.Add(function0);
    event1.Add(function1);
    event2.Add(function2);
    event3.Add(function3);

    event0();
    event1(42);
    event2(123, "hello");
    event3(-2, "world", 3.14f);

    for (int n = 0; n < 4; ++n) {
        ASSERT_TRUE(funCalled[n]);
        ASSERT_TRUE(foo.methodCalled[n]);
    }

    ASSERT_EQ(42, foo.r1);
    ASSERT_EQ(123, foo.r2);
    ASSERT_EQ(-2, foo.r3);
    ASSERT_EQ(3.14f, foo.f3);
    ASSERT_STREQ("hello", foo.s2.c_str());
    ASSERT_STREQ("world", foo.s3.c_str());

    Registry reg;
    reg.AddClass<void>();
    reg.AddClass<Grok>();

    Pointer<Grok> grok = reg.New<Grok>();
    event0.AddMethod(&*grok, &Grok::method0);
    event0.AddMethod<Grok>(grok, &Grok::method0);
    event0();
    grok.Delete();
    reg.GarbageCollect();
    event0();
}
