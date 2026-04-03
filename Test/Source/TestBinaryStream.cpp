#include "TestCommon.h"

using namespace kai;
using namespace std;

TEST(TestBinaryStream, Builtins) {
    BinaryStream S;

    S << 42;
    EXPECT_EQ(S.Size(), sizeof(int));
    ASSERT_TRUE(S.CanRead(sizeof(int)));
    int N;
    S >> N;
    EXPECT_EQ(N, 42);
    EXPECT_FALSE(S.CanRead(1));

    S.Clear();
    EXPECT_EQ(S.Size(), 0);
    EXPECT_FALSE(S.CanRead(1));

    S << String("Hello, world");
    String T;
    S >> T;
    EXPECT_FALSE(S.CanRead(1));
    EXPECT_EQ(T, "Hello, world");
}

TEST(TestBinaryStream, TestObject) {
    Registry R;
    R.AddClass<int>();

    BinaryStream S;
    S.SetRegistry(&R);

    Pointer<int> N = R.New(42);

    S << N;
    Object Q;
    S >> Q;
    ASSERT_FALSE(S.CanRead(1));
    EXPECT_EQ(Q.GetTypeNumber(), Type::Traits<int>::Number);
    EXPECT_EQ(ConstDeref<int>(Q), 42);

    S.Clear();
    ASSERT_TRUE(S.Empty());

    auto lab = Label("child0");
    N.Set(lab, R.New<int>(123));
    S << N;
    Object M;
    S >> M;
    ASSERT_FALSE(S.CanRead(1));
    EXPECT_EQ(ConstDeref<int>(M), 42);
    ASSERT_TRUE(M.Has(lab));
    EXPECT_EQ(ConstDeref<int>(M.Get(lab)), 123);
}

TEST(TestBinaryStream, TestArray) {
    Registry R;
    R.AddClass<Array>();
    R.AddClass<int>();

    Pointer<Array> A = R.New<Array>();
    A->Append(R.New<int>(0));
    A->Append(R.New<int>(1));
    A->Append(R.New<int>(2));
    A->Append(R.New<int>(3));

    BinaryStream S;
    S.SetRegistry(&R);

    S << A;
    Object Q;
    S >> Q;

    ASSERT_FALSE(S.CanRead(1));
    EXPECT_EQ(Q.GetTypeNumber(), Type::Traits<Array>::Number);
    const Array &B = ConstDeref<Array>(Q);
    EXPECT_EQ(B.Size(), 4);
    EXPECT_EQ(ConstDeref<int>(B.At(0)), 0);
    EXPECT_EQ(ConstDeref<int>(B.At(1)), 1);
    EXPECT_EQ(ConstDeref<int>(B.At(2)), 2);
    EXPECT_EQ(ConstDeref<int>(B.At(3)), 3);
}

struct TestPropertiesStruct {
    int num;
    String str;
    bool foo;
};

KAI_BEGIN

KAI_TYPE_TRAITS(TestPropertiesStruct, 555, 0);

KAI_END

TEST(TestBinaryStream, TestProperties) {
    Registry R;
    R.AddClass<void>();
    R.AddClass<int>();
    R.AddClass<bool>();
    R.AddClass<String>();
    ClassBuilder<TestPropertiesStruct>(R, "TestPropertiesStruct")
        .Methods.Properties("num", &TestPropertiesStruct::num)(
            "str", &TestPropertiesStruct::str)("foo",
                                               &TestPropertiesStruct::foo);
    Pointer<TestPropertiesStruct> tps = R.New<TestPropertiesStruct>();
    tps->num = 42;
    tps->str = "hello";
    // tps->foo = R.New<TestPropertiesStruct>();
    tps->foo = true;
    KAI_TRACE_1(tps);

    BinaryStream stream(R);
    stream << tps;
    KAI_TRACE_1(stream);
    Object result;
    stream >> result;
    KAI_TRACE_1(result);

    ASSERT_TRUE(result.Exists());
    ASSERT_TRUE(result.IsType<TestPropertiesStruct>());
    EXPECT_EQ(ConstDeref<int>(result.Get(Label("num"))), 42);
    EXPECT_STREQ(ConstDeref<String>(result.Get(Label("str"))).c_str(), "hello");
    // ASSERT_TRUE(result.Get("foo").IsType<TestPropertiesStruct>());
    ASSERT_TRUE(result.Get("foo").IsType<bool>());
}

TEST(TestBinaryStream, TestList) {
    Registry R;
    List::Register(R);
    R.AddClass<void>();
    R.AddClass<int>();
    R.AddClass<String>();

    Pointer<List> list = R.New<List>();
    list->Append(R.New(123));
    list->Append(R.New(456));
    list->Append(R.New(String("Hello")));
    BinaryStream stream(R);
    stream << list;
    Object result;
    stream >> result;

    KAI_TRACE_1(list);
    KAI_TRACE_1(stream);
    KAI_TRACE_1(result);

    ASSERT_TRUE(result.Exists());
    ASSERT_TRUE(result.IsType<List>());
    EXPECT_EQ(ConstDeref<List>(result).Size(), 3);
    EXPECT_EQ(result, list);

    Pointer<List> result_list = result;
    EXPECT_EQ(ConstDeref<String>(result_list->Back()), "Hello");
    result_list->PopBack();
    EXPECT_EQ(ConstDeref<int>(result_list->Back()), 456);
    result_list->PopBack();
    EXPECT_EQ(ConstDeref<int>(result_list->Back()), 123);
    result_list->PopBack();
    EXPECT_EQ(ConstDeref<List>(result_list).Size(), 0);
}

TEST(TestBinaryStream, TestMap) {}

TEST(TestBinaryStream, TestSet) {}

TEST(TestBinaryStream, TestStreams) {}
