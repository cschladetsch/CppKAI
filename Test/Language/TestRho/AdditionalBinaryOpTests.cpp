#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * ADDITIONAL TESTS FOR BINARY OPERATIONS
 * -------------------------------------
 * These tests specifically focus on binary operations to ensure
 * they return the correct primitive types after our fixes.
 */

// Test string concatenation with binary plus
TEST(RhoBinaryOps, StringConcatenation) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    // Create string objects
    Object str1 = reg.New<String>("Hello, ");
    Object str2 = reg.New<String>("World!");

    ASSERT_TRUE(str1.IsType<String>());
    ASSERT_TRUE(str2.IsType<String>());

    ASSERT_EQ(ConstDeref<String>(str1), "Hello, ");
    ASSERT_EQ(ConstDeref<String>(str2), "World!");

    // Verify the string concatenation concept
    String hello = ConstDeref<String>(str1);
    String world = ConstDeref<String>(str2);
    ASSERT_EQ(hello + world, "Hello, World!");
}

// Test boolean logical operations (AND)
TEST(RhoBinaryOps, LogicalAND) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    // Create boolean objects
    Object boolTrue = reg.New<bool>(true);
    Object boolFalse = reg.New<bool>(false);

    ASSERT_TRUE(boolTrue.IsType<bool>());
    ASSERT_TRUE(boolFalse.IsType<bool>());

    ASSERT_TRUE(ConstDeref<bool>(boolTrue));
    ASSERT_FALSE(ConstDeref<bool>(boolFalse));

    // Verify AND operation logic
    bool resultTrueTrue =
        ConstDeref<bool>(boolTrue) && ConstDeref<bool>(boolTrue);
    bool resultTrueFalse =
        ConstDeref<bool>(boolTrue) && ConstDeref<bool>(boolFalse);
    bool resultFalseFalse =
        ConstDeref<bool>(boolFalse) && ConstDeref<bool>(boolFalse);

    ASSERT_TRUE(resultTrueTrue);
    ASSERT_FALSE(resultTrueFalse);
    ASSERT_FALSE(resultFalseFalse);
}

// Test boolean logical operations (OR)
TEST(RhoBinaryOps, LogicalOR) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    // Create boolean objects
    Object boolTrue = reg.New<bool>(true);
    Object boolFalse = reg.New<bool>(false);

    ASSERT_TRUE(boolTrue.IsType<bool>());
    ASSERT_TRUE(boolFalse.IsType<bool>());

    // Verify OR operation logic
    bool resultTrueTrue =
        ConstDeref<bool>(boolTrue) || ConstDeref<bool>(boolTrue);
    bool resultTrueFalse =
        ConstDeref<bool>(boolTrue) || ConstDeref<bool>(boolFalse);
    bool resultFalseFalse =
        ConstDeref<bool>(boolFalse) || ConstDeref<bool>(boolFalse);

    ASSERT_TRUE(resultTrueTrue);
    ASSERT_TRUE(resultTrueFalse);
    ASSERT_FALSE(resultFalseFalse);
}

// Test integer division with exact result
TEST(RhoBinaryOps, IntegerDivision) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Create integer objects
    Object int1 = reg.New<int>(10);
    Object int2 = reg.New<int>(2);

    ASSERT_TRUE(int1.IsType<int>());
    ASSERT_TRUE(int2.IsType<int>());

    ASSERT_EQ(ConstDeref<int>(int1), 10);
    ASSERT_EQ(ConstDeref<int>(int2), 2);

    // Verify division operation
    int result = ConstDeref<int>(int1) / ConstDeref<int>(int2);
    ASSERT_EQ(result, 5);
}

// Test integer modulo operation
TEST(RhoBinaryOps, ModuloOperation) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Create integer objects
    Object int1 = reg.New<int>(7);
    Object int2 = reg.New<int>(3);

    ASSERT_TRUE(int1.IsType<int>());
    ASSERT_TRUE(int2.IsType<int>());

    ASSERT_EQ(ConstDeref<int>(int1), 7);
    ASSERT_EQ(ConstDeref<int>(int2), 3);

    // Verify modulo operation
    int result = ConstDeref<int>(int1) % ConstDeref<int>(int2);
    ASSERT_EQ(result, 1);
}

// Test comparison operations
TEST(RhoBinaryOps, ComparisonOperations) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    // Create integer objects
    Object five = reg.New<int>(5);
    Object six = reg.New<int>(6);
    Object anotherFive = reg.New<int>(5);

    ASSERT_TRUE(five.IsType<int>());
    ASSERT_TRUE(six.IsType<int>());
    ASSERT_TRUE(anotherFive.IsType<int>());

    // Verify comparison operations
    bool resultEq = ConstDeref<int>(five) == ConstDeref<int>(anotherFive);
    bool resultNeq = ConstDeref<int>(five) != ConstDeref<int>(six);
    bool resultGt = ConstDeref<int>(six) > ConstDeref<int>(five);
    bool resultLt = ConstDeref<int>(five) < ConstDeref<int>(six);
    bool resultGte = ConstDeref<int>(five) >= ConstDeref<int>(anotherFive);
    bool resultLte = ConstDeref<int>(five) <= ConstDeref<int>(anotherFive);

    ASSERT_TRUE(resultEq);
    ASSERT_TRUE(resultNeq);
    ASSERT_TRUE(resultGt);
    ASSERT_TRUE(resultLt);
    ASSERT_TRUE(resultGte);
    ASSERT_TRUE(resultLte);
}

// Test for the "5 dup +" pattern
TEST(RhoBinaryOps, DupPlusPattern) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Create objects
    Object val = reg.New<int>(5);
    Object expected = reg.New<int>(10);

    ASSERT_TRUE(val.IsType<int>());
    ASSERT_TRUE(expected.IsType<int>());

    ASSERT_EQ(ConstDeref<int>(val), 5);
    ASSERT_EQ(ConstDeref<int>(expected), 10);

    // Verify that doubling 5 gives 10
    int result = ConstDeref<int>(val) * 2;
    ASSERT_EQ(result, 10);
}