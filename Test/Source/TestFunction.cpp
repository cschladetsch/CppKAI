#include "TestCommon.h"

using namespace kai;
using namespace std;

namespace {
static bool called[8];

void VF0() { called[0] = true; }

void VF1(int) { called[1] = true; }

void VF2(int, int) { called[2] = true; }

void VF3(int, int, int) { called[3] = true; }

// TODO: Uncomment when F0 test is re-enabled

int F1(int) {
    called[5] = true;
    return 1;
}

int F2(int, int) {
    called[6] = true;
    return 2;
}

int F3(int, int, int) {
    called[7] = true;
    return 3;
}

[[maybe_unused]] void G0(Pointer<int>) {}

// TODO: Uncomment when G1 test is re-enabled
}  // namespace

struct FunctionTest : ::testing::Test {
   protected:
    virtual void SetUp() override {
        _reg.AddClass<void>(Label("void"));
        _reg.AddClass<int>(Label("int"));
        _reg.AddClass<String>(Label("String"));
        _reg.AddClass<Stack>(Label("stack"));
        _reg.AddClass<BasePointer<FunctionBase> >(Label("Function"));
        _stack = _reg.New<Stack>();
        std::fill(called, called + sizeof(called) / sizeof(called[0]), false);
    }

    virtual void TearDown() override {}

    Registry _reg;
    Value<Stack> _stack;
};

TEST_F(FunctionTest, TestConstruction) {
    FunctionBase *vf0 = MakeFunction(VF0, Label("VF0"));
    FunctionBase *vf1 = MakeFunction(VF1);
    FunctionBase *vf2 = MakeFunction(VF2);
    FunctionBase *vf3 = MakeFunction(VF3);
    FunctionBase *f1 = MakeFunction(F1);
    FunctionBase *f2 = MakeFunction(F2);
    FunctionBase *f3 = MakeFunction(F3);

    // TODO: Uncomment when G0 test is re-enabled

    EXPECT_EQ(vf0->GetReturnType(), Type::Traits<void>::Number);
    EXPECT_EQ(vf1->GetReturnType(), Type::Traits<void>::Number);
    EXPECT_EQ(vf2->GetReturnType(), Type::Traits<void>::Number);
    EXPECT_EQ(vf3->GetReturnType(), Type::Traits<void>::Number);

    EXPECT_EQ(vf0->GetArgumentTypes().size(), std::size_t(0));
    EXPECT_EQ(vf1->GetArgumentTypes().size(), std::size_t(1));
    EXPECT_EQ(vf2->GetArgumentTypes().size(), std::size_t(2));
    EXPECT_EQ(vf3->GetArgumentTypes().size(), std::size_t(3));

    EXPECT_EQ(vf1->GetArgumentTypes()[0], Type::Traits<int>::Number);
    EXPECT_EQ(vf1->GetArgumentType(0), Type::Traits<int>::Number);

    EXPECT_EQ(vf2->GetArgumentType(0), Type::Traits<int>::Number);
    EXPECT_EQ(vf2->GetArgumentType(1), Type::Traits<int>::Number);

    EXPECT_EQ(vf3->GetArgumentType(0), Type::Traits<int>::Number);
    EXPECT_EQ(vf3->GetArgumentType(1), Type::Traits<int>::Number);
    EXPECT_EQ(vf3->GetArgumentType(2), Type::Traits<int>::Number);

    vf0->Invoke(_reg, *_stack);
    ASSERT_TRUE(called[0]);

    _stack->Push(_reg.New(42));
    vf1->Invoke(_reg, *_stack);
    ASSERT_TRUE(called[1]);

    _stack->Push(_reg.New(5));
    _stack->Push(_reg.New(234));
    vf2->Invoke(_reg, *_stack);
    ASSERT_TRUE(called[2]);

    _stack->Push(_reg.New(5));
    _stack->Push(_reg.New(234));
    _stack->Push(_reg.New(123));
    vf3->Invoke(_reg, *_stack);
    ASSERT_TRUE(called[3]);

    _stack->Push(_reg.New(42));
    f1->Invoke(_reg, *_stack);
    ASSERT_TRUE(called[5]);
    EXPECT_EQ(ConstDeref<int>(_stack->Pop()), 1);

    _stack->Push(_reg.New(5));
    _stack->Push(_reg.New(234));
    f2->Invoke(_reg, *_stack);
    ASSERT_TRUE(called[6]);
    EXPECT_EQ(ConstDeref<int>(_stack->Pop()), 2);

    _stack->Push(_reg.New(5));
    _stack->Push(_reg.New(234));
    _stack->Push(_reg.New(123));
    f3->Invoke(_reg, *_stack);
    ASSERT_TRUE(called[7]);
    EXPECT_EQ(ConstDeref<int>(_stack->Pop()), 3);

    _stack->Push(_reg.New(456));
}

// TODO: Test leaking of base pointers

// void FunctionTest::TestExecutor()
//{
//     // TODO
//     //registry.AddClass<Compiler>("Compiler");
//     //Value<Compiler> compiler = registry.New<Compiler>();
// }
