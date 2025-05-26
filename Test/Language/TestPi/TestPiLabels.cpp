#include "TestLangCommon.h"
#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/BuiltinTypes/Array.h>
#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/Object/Object.h>
#include <KAI/Executor/Continuation.h>

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
    // Pi doesn't support float literals, so we'll use another int
    ExecutePi("314 'intVal2 #");
    ExecutePi("\"hello\" 'stringVal #");
    
    // Retrieve first int
    ExecutePi("intVal");
    ASSERT_TRUE(stack()->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 42);
    stack()->Pop();
    
    // Retrieve second int
    ExecutePi("intVal2");
    ASSERT_TRUE(stack()->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 314);
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
    
    ASSERT_EQ(stack()->Size(), 1) << "Stack should have 1 element after addition";
    ASSERT_TRUE(stack()->Top().IsType<int>()) << "Result should be an int";
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 30) << "10 + 20 should equal 30";
}

TEST_F(PiLabelTest, LabelPushesItself) {
    stack()->Clear();
    ExecutePi("'myLabel");
    
    ASSERT_EQ(stack()->Size(), 1) << "Stack should have 1 element";
    ASSERT_TRUE(stack()->Top().IsType<Pathname>()) << "Should be a Pathname";
    ASSERT_EQ(ConstDeref<Pathname>(stack()->Top()).ToString(), "'myLabel") << "Should preserve quote";
}

TEST_F(PiLabelTest, NestedScopeResolution) {
    stack()->Clear();
    ExecutePi("100 'outerVal #");
    
    // Create a nested scope
    ExecutePi("{ 200 'innerVal # outerVal innerVal + } &");
    
    ASSERT_EQ(stack()->Size(), 1);
    ASSERT_TRUE(stack()->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack()->Top()), 300) << "Should add outer and inner values";
}

TEST_F(PiLabelTest, UndefinedLabelHandling) {
    stack()->Clear();
    
    // Try to access undefined label
    ExecutePi("undefinedLabel");
    
    // Should push an empty object
    ASSERT_EQ(stack()->Size(), 1);
    ASSERT_FALSE(stack()->Top().Exists()) << "Undefined label should push empty object";
}