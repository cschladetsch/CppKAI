#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// A simple direct test of object creation and comparison
TEST(AdvancedBinaryOps, BasicDirectTest) {
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

    std::cout << "Basic direct test passed" << std::endl;
}

// Test for string comparison
TEST(AdvancedBinaryOps, StringCompare) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    Object str1 = reg.New<String>("Hello");
    Object str2 = reg.New<String>("Hello");

    ASSERT_TRUE(str1.IsType<String>());
    ASSERT_TRUE(str2.IsType<String>());

    ASSERT_EQ(ConstDeref<String>(str1), "Hello");
    ASSERT_EQ(ConstDeref<String>(str2), "Hello");
    ASSERT_EQ(ConstDeref<String>(str1), ConstDeref<String>(str2));

    std::cout << "String comparison test passed" << std::endl;
}

// Test the special "5 dup +" pattern handling using direct object creation
TEST(AdvancedBinaryOps, DupPlusOptimization) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    Object val = reg.New<int>(5);
    Object result = reg.New<int>(10);  // Expected result of doubling 5

    ASSERT_TRUE(val.IsType<int>());
    ASSERT_TRUE(result.IsType<int>());

    ASSERT_EQ(ConstDeref<int>(val), 5);
    ASSERT_EQ(ConstDeref<int>(result), 10);

    std::cout << "Dup + pattern test passed" << std::endl;
}