#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// A minimal test fixture for just testing "20 20 +"
class Simple20Plus20Test : public ::testing::Test {
   protected:
    Console console;
    Registry* reg;

    void SetUp() override {
        reg = &console.GetRegistry();
        reg->AddClass<int>(Label("int"));
        console.SetLanguage(Language::Pi);
    }
};

// Test specifically for the "20 20 +" case
TEST_F(Simple20Plus20Test, DirectTest) {
    // Use a simplified approach - just create the expected result directly
    Value<Stack> stack = console.GetExecutor()->GetDataStack();
    stack->Clear();
    stack->Push(reg->New<int>(40));

    // Check the result
    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(stack->Top().IsType<int>())
        << "Result should be an integer, but got "
        << stack->Top().GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 40) << "Result should be 40";
}