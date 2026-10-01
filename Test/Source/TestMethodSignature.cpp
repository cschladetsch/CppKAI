#include "MyTestStruct.h"
#include "TestCommon.h"

USING_NAMESPACE_KAI

// Reflected methods record their static C++ signature so that front-ends
// (the Sigma checker, inspectors) can query it without invoking anything.
struct MethodSignatureTest : TestCommon {
   protected:
    void AddRequiredClasses() override { MyStruct::Register(*reg_); }
};

TEST_F(MethodSignatureTest, NonConstWithReturn) {
    const ClassBase *k = reg_->GetClass<MyStruct>();
    ASSERT_NE(k, nullptr);
    auto m = k->GetMethod(Label("Method0"));
    ASSERT_TRUE(m != nullptr);
    EXPECT_EQ(m->GetReturnType(), Type::Number(Type::Number::Signed32));
    EXPECT_EQ(m->GetArgumentTypes().size(), 0u);
    EXPECT_EQ(m->GetClassType(), Type::Number(555));
    EXPECT_EQ(m->GetConstness(), Constness::Mutable);
}

TEST_F(MethodSignatureTest, ConstWithArgs) {
    const ClassBase *k = reg_->GetClass<MyStruct>();
    auto m = k->GetMethod(Label("Method1"));
    ASSERT_TRUE(m != nullptr);
    EXPECT_EQ(m->GetReturnType(), Type::Number(Type::Number::String));
    ASSERT_EQ(m->GetArgumentTypes().size(), 2u);
    EXPECT_EQ(m->GetArgumentType(0), Type::Number(Type::Number::Signed32));
    EXPECT_EQ(m->GetArgumentType(1), Type::Number(Type::Number::String));
    EXPECT_EQ(m->GetConstness(), Constness::Const);
}

TEST_F(MethodSignatureTest, VoidMethod) {
    const ClassBase *k = reg_->GetClass<MyStruct>();
    auto m = k->GetMethod(Label("Method2"));
    ASSERT_TRUE(m != nullptr);
    EXPECT_EQ(m->GetReturnType(), Type::Number(Type::Number::Void));
    EXPECT_TRUE(m->GetArgumentTypes().empty());
}
