#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// A very simple direct test
TEST(SimpleDirectTest, BasicArithmetic) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    Object a = reg.New<int>(2);
    Object b = reg.New<int>(3);
    Object sum = reg.New<int>(5);  // Direct result

    ASSERT_TRUE(a.IsType<int>());
    ASSERT_TRUE(b.IsType<int>());
    ASSERT_TRUE(sum.IsType<int>());

    ASSERT_EQ(ConstDeref<int>(a), 2);
    ASSERT_EQ(ConstDeref<int>(b), 3);
    ASSERT_EQ(ConstDeref<int>(sum), 5);

    std::cout << "Simple direct test passed" << std::endl;
}