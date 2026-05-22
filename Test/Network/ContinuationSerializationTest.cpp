#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/All.h"
#include "KAI/Core/Registry.h"
#include "KAI/Core/Tree.h"
#include "KAI/Executor/BinBase.h"
#include "KAI/Executor/Continuation.h"
#include "KAI/Console/Console.h"

using namespace kai;

class ContinuationSerializationTest : public ::testing::Test {
protected:
    Console console_;

    void SetUp() override {
        console_.SetLanguage(Language::Pi);
    }

    // Compile a Pi program and return the top-level continuation.
    Value<Continuation> Compile(const char *src) {
        auto cont = console_.Compile(src, Structure::Program);
        EXPECT_TRUE(cont.Exists()) << "Compile failed for: " << src;
        return cont;
    }

    // Execute src and return the int on top of stack.
    int Exec(const char *src) {
        console_.Execute(src);
        auto stack = console_.GetExecutor()->GetDataStack();
        EXPECT_FALSE(stack->Empty()) << "Stack empty after: " << src;
        return ConstDeref<int>(stack->Top());
    }
};

// Freeze a continuation and confirm the result is a BinaryStream.
TEST_F(ContinuationSerializationTest, FreezeReturnsBinaryStream) {
    auto cont = Compile("5 2 *");
    Object frozen = Bin::Freeze(cont);
    EXPECT_TRUE(frozen.Exists());
    EXPECT_TRUE(frozen.IsType<BinaryStream>());
}

// Round-trip: freeze then thaw returns an Object of type Continuation.
TEST_F(ContinuationSerializationTest, ThawReturnsContinuation) {
    auto cont = Compile("5 2 *");
    Object frozen = Bin::Freeze(cont);
    Object thawed = Bin::Thaw(frozen);
    EXPECT_TRUE(thawed.Exists());
    EXPECT_TRUE(thawed.IsType<Continuation>());
}

// Thawed continuation has code.
TEST_F(ContinuationSerializationTest, ThawedContinuationHasCode) {
    auto cont = Compile("5 2 *");
    Object frozen = Bin::Freeze(cont);
    Object thawed = Bin::Thaw(frozen);
    ASSERT_TRUE(thawed.IsType<Continuation>());
    const Continuation &c = ConstDeref<Continuation>(thawed);
    EXPECT_TRUE(c.HasCode());
    EXPECT_EQ(c.GetCode()->Size(), cont->GetCode()->Size());
}

// Thawed continuation executes and yields the correct result.
TEST_F(ContinuationSerializationTest, ThawedContinuationExecutesCorrectly) {
    auto cont = Compile("5 2 *");
    Object frozen = Bin::Freeze(cont);
    Object thawed = Bin::Thaw(frozen);
    ASSERT_TRUE(thawed.IsType<Continuation>());

    console_.GetExecutor()->Continue(Value<Continuation>(thawed));

    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// Multiple round-trips preserve executability.
TEST_F(ContinuationSerializationTest, DoubleRoundTripExecutesCorrectly) {
    auto cont = Compile("3 4 +");
    Object frozen1 = Bin::Freeze(cont);
    Object thawed1 = Bin::Thaw(frozen1);
    Object frozen2 = Bin::Freeze(thawed1);
    Object thawed2 = Bin::Thaw(frozen2);

    ASSERT_TRUE(thawed2.IsType<Continuation>());
    console_.GetExecutor()->Continue(Value<Continuation>(thawed2));

    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 7);
}

// BinaryStream round-trip via operator<< / operator>>.
TEST_F(ContinuationSerializationTest, BinaryStreamRoundTrip) {
    auto cont = Compile("10 3 -");

    BinaryStream stream;
    stream << *cont;
    stream.SetRegistry(&console_.GetRegistry());

    Continuation restored;
    stream >> restored;

    EXPECT_TRUE(restored.HasCode());
    EXPECT_EQ(restored.GetCode()->Size(), cont->GetCode()->Size());
}

