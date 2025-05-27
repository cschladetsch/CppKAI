#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/BuiltinTypes/Array.h>
#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Object/Object.h>
#include <KAI/Executor/Continuation.h>

#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Force linking by exporting a symbol
void ForcePiLabelTestLink() {
    // This function exists solely to ensure this compilation unit is linked
}

struct PiLabelTest : TestLangCommon {
    void ExecutePi(const string& code) {
        KAI_TRACE() << "Executing Pi code: " << code;
        console_.Execute(code);
    }

    Value<Stack> stack() { return console_.GetExecutor()->GetDataStack(); }
};

// Basic tests for storing and retrieving values with labels
TEST_F(PiLabelTest, StoreWithLabel) {
    stack()->Clear();
    ExecutePi("5 'x #");
    ASSERT_EQ(stack()->Size(), 0) << "Stack should be empty after store";

    ExecutePi("x");
    ASSERT_EQ(stack()->Size(), 1) << "Stack should have 1 element";
    ASSERT_TRUE(stack()->Top().IsType<int>()) << "Should be an int";
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 5) << "Value should be 5";
}

TEST_F(PiLabelTest, MultipleStores) {
    stack()->Clear();
    ExecutePi("10 'x #");
    ExecutePi("20 'y #");

    ExecutePi("x");
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 10);
    stack()->Pop();

    ExecutePi("y");
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 20);
}

TEST_F(PiLabelTest, StoreRetrieveMultipleTypes) {
    stack()->Clear();
    ExecutePi("42 'intVal #");
    ExecutePi("3.14 'floatVal #");
    ExecutePi("\"hello\" 'stringVal #");

    // Retrieve int
    ExecutePi("intVal");
    ASSERT_TRUE(stack()->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 42);
    stack()->Pop();

    // Retrieve float
    ExecutePi("floatVal");
    ASSERT_TRUE(stack()->Top().IsType<float>());
    ASSERT_FLOAT_EQ(ConstDeref<float>(stack()->Top()), 3.14f);
    stack()->Pop();

    // Retrieve string
    ExecutePi("stringVal");
    ASSERT_TRUE(stack()->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(stack()->Top()), "hello");
}

TEST_F(PiLabelTest, Reassignment) {
    stack()->Clear();
    ExecutePi("10 'x #");
    ExecutePi("x");
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 10);
    stack()->Clear();

    ExecutePi("20 'x #");
    ExecutePi("x");
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 20);
}

TEST_F(PiLabelTest, CompoundOperations) {
    stack()->Clear();
    ExecutePi("10 'x #");
    ExecutePi("20 'y #");
    ExecutePi("x y +");

    ASSERT_EQ(stack()->Size(), 1)
        << "Stack should have 1 element after addition";
    ASSERT_TRUE(stack()->Top().IsType<int>()) << "Result should be an int";
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 30) << "10 + 20 should equal 30";
}

TEST_F(PiLabelTest, LabelPushesItself) {
    stack()->Clear();
    ExecutePi("'myLabel");

    ASSERT_EQ(stack()->Size(), 1) << "Stack should have 1 element";
    ASSERT_TRUE(stack()->Top().IsType<Pathname>()) << "Should be a Pathname";
    ASSERT_EQ(ConstDeref<Pathname>(stack()->Top()).ToString(), "'myLabel")
        << "Should preserve quote";
}

// TODO: Nested scope resolution with & operation needs investigation
// The & (Suspend) operation doesn't properly inherit parent scope
// TEST_F(PiLabelTest, NestedScopeResolution) {
//     stack()->Clear();
//     ExecutePi("100 'outerVal #");
//
//     // Create a nested scope
//     ExecutePi("{ 200 'innerVal # outerVal innerVal + } &");
//
//     ASSERT_EQ(stack()->Size(), 1);
//     ASSERT_TRUE(stack()->Top().IsType<int>());
//     ASSERT_EQ(ConstDeref<int>(stack()->Top()), 300) << "Should add outer and
//     inner values";
// }

TEST_F(PiLabelTest, UndefinedLabelHandling) {
    stack()->Clear();

    // Try to access undefined label
    ExecutePi("undefinedLabel");

    // Should push an empty object
    ASSERT_EQ(stack()->Size(), 1);
    ASSERT_FALSE(stack()->Top().Exists())
        << "Undefined label should push empty object";
}

TEST_F(PiLabelTest, FloatOperations) {
    stack()->Clear();

    // Test basic float arithmetic
    ExecutePi("2.5 3.5 +");
    ASSERT_TRUE(stack()->Top().IsType<float>());
    ASSERT_FLOAT_EQ(ConstDeref<float>(stack()->Top()), 6.0f);
    stack()->Pop();

    // Test float multiplication
    ExecutePi("1.5 2.0 *");
    ASSERT_TRUE(stack()->Top().IsType<float>());
    ASSERT_FLOAT_EQ(ConstDeref<float>(stack()->Top()), 3.0f);
    stack()->Pop();

    // Test float with integer operations (should promote to float)
    ExecutePi("3.14 2 *");
    ASSERT_TRUE(stack()->Top().IsType<float>());
    ASSERT_FLOAT_EQ(ConstDeref<float>(stack()->Top()), 6.28f);
}