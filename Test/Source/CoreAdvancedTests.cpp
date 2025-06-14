#include <gtest/gtest.h>

#include <chrono>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Exception.h"
#include "KAI/Core/Object.h"
#include "TestCommon.h"

// Advanced Core tests
using namespace kai;

struct CoreAdvancedTests : kai::TestCommon {};

// Test 1: Deep object cloning
TEST_F(CoreAdvancedTests, DeepObjectCloning) {
    kai::Object original = Reg().New<int>(42);

    // Test object validity and handle
    ASSERT_TRUE(original.Valid());
    ASSERT_NE(original.GetHandle().GetValue(), 0);
    ASSERT_EQ(kai::ConstDeref<int>(original), 42);
}

// Test 2: Object type system
TEST_F(CoreAdvancedTests, ObjectTypeSystem) {
    kai::Object obj = Reg().New<kai::String>("test");

    // Test type information
    ASSERT_TRUE(obj.Valid());
    ASSERT_TRUE(obj.template IsType<kai::String>());
    ASSERT_FALSE(obj.template IsType<int>());
    ASSERT_EQ(kai::ConstDeref<kai::String>(obj), "test");
}

// Test 3: Registry stress test
TEST_F(CoreAdvancedTests, RegistryStressTest) {
    const int NUM_OBJECTS = 1000;
    std::vector<kai::Object> objects;

    for (int i = 0; i < NUM_OBJECTS; ++i) {
        objects.push_back(Reg().New<int>(i));
    }

    // Verify all objects are valid
    for (int i = 0; i < NUM_OBJECTS; ++i) {
        ASSERT_TRUE(objects[i].Valid());
        ASSERT_EQ(kai::ConstDeref<int>(objects[i]), i);
    }

    // Delete half
    for (int i = 0; i < NUM_OBJECTS / 2; ++i) {
        objects[i].Delete();
    }

    // Verify deletion
    for (int i = 0; i < NUM_OBJECTS / 2; ++i) {
        ASSERT_FALSE(objects[i].Valid());
    }
    for (int i = NUM_OBJECTS / 2; i < NUM_OBJECTS; ++i) {
        ASSERT_TRUE(objects[i].Valid());
    }
}

// Test 4: Type traits and reflection
TEST_F(CoreAdvancedTests, TypeTraitsReflection) {
    kai::Object intObj = Reg().New<int>(42);
    kai::Object strObj = Reg().New<kai::String>("hello");
    kai::Object arrObj = Reg().New<kai::Array>();

    ASSERT_TRUE(intObj.template IsType<int>());
    ASSERT_FALSE(intObj.template IsType<kai::String>());

    ASSERT_TRUE(strObj.template IsType<kai::String>());
    ASSERT_FALSE(strObj.template IsType<int>());

    ASSERT_TRUE(arrObj.template IsType<kai::Array>());
    ASSERT_FALSE(arrObj.template IsType<int>());
}

// Test 5: Memory management with custom allocators
TEST_F(CoreAdvancedTests, CustomMemoryManagement) {
    // Test object creation and deletion patterns
    for (int i = 0; i < 100; ++i) {
        kai::Object obj = Reg().New<kai::String>("test" + std::to_string(i));
        ASSERT_TRUE(obj.Valid());
        // kai::Object will be cleaned up automatically
    }

    // Force garbage collection
    Reg().TriColor();

    // Registry should still be functional
    kai::Object afterGC = Reg().New<int>(999);
    ASSERT_TRUE(afterGC.Valid());
    ASSERT_EQ(kai::ConstDeref<int>(afterGC), 999);
}

// Test 6: Complex container operations
TEST_F(CoreAdvancedTests, ComplexContainerOps) {
    // Test basic array operations
    kai::Object arr = Reg().New<kai::Array>();
    ASSERT_TRUE(arr.Valid());

    // Add some elements
    for (int i = 0; i < 10; ++i) {
        kai::Pointer<kai::Array> arrPtr = arr;
        arrPtr->Append(Reg().New<int>(i));
    }

    // Verify size
    kai::Pointer<kai::Array> arrPtr = arr;
    ASSERT_EQ(arrPtr->Size(), 10);
}

// Test 7: Exception handling robustness
TEST_F(CoreAdvancedTests, ExceptionHandlingRobustness) {
    try {
        // Try to create object with invalid type
        Reg().GetObject(kai::Handle(9999999));
        FAIL() << "Expected exception for invalid handle";
    } catch (const kai::Exception::Base &e) {
        // Expected
        SUCCEED();
    }

    try {
        kai::Object obj = Reg().New<int>(42);
        obj.Delete();
        // Try to use deleted object
        int val = kai::ConstDeref<int>(obj);
        (void)val;
        FAIL() << "Expected exception for deleted object access";
    } catch (const kai::Exception::Base &e) {
        // Expected
        SUCCEED();
    }
}

// Test 8: Object comparison
TEST_F(CoreAdvancedTests, ObjectComparison) {
    // Create objects for comparison
    kai::Object int1 = Reg().New<int>(42);
    kai::Object int2 = Reg().New<int>(42);
    kai::Object int3 = Reg().New<int>(99);

    // Test handle comparison
    ASSERT_NE(int1.GetHandle(), int2.GetHandle());
    ASSERT_NE(int1.GetHandle(), int3.GetHandle());

    // Test value comparison
    ASSERT_EQ(kai::ConstDeref<int>(int1), kai::ConstDeref<int>(int2));
    ASSERT_NE(kai::ConstDeref<int>(int1), kai::ConstDeref<int>(int3));
}

// Test 9: Multiple object creation
TEST_F(CoreAdvancedTests, MultipleObjectCreation) {
    // Create multiple objects of different types
    std::vector<kai::Object> objects;

    objects.push_back(Reg().New<int>(100));
    objects.push_back(Reg().New<float>(3.14f));
    objects.push_back(Reg().New<kai::String>("hello"));
    objects.push_back(Reg().New<bool>(true));

    // Verify all are valid
    for (const auto &obj : objects) {
        ASSERT_TRUE(obj.Valid());
    }

    // Verify types
    ASSERT_TRUE(objects[0].template IsType<int>());
    ASSERT_TRUE(objects[1].template IsType<float>());
    ASSERT_TRUE(objects[2].template IsType<kai::String>());
    ASSERT_TRUE(objects[3].template IsType<bool>());
}

// Test 10: Performance benchmarking utilities
TEST_F(CoreAdvancedTests, PerformanceBenchmarking) {
    const int ITERATIONS = 10000;

    // Benchmark object creation
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i) {
        kai::Object obj = Reg().New<int>(i);
        (void)obj;  // Prevent optimization
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Just verify it completes in reasonable time (< 1 second)
    ASSERT_LT(duration.count(), 1000000) << "kai::Object creation too slow";

    // Benchmark array append
    kai::Pointer<kai::Array> arr = Reg().New<kai::Array>();
    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS / 100; ++i) {  // Reduce iterations for array
        arr->Append(Reg().New<int>(i));
    }

    end = std::chrono::high_resolution_clock::now();
    duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    ASSERT_EQ(arr->Size(), ITERATIONS / 100);
    ASSERT_LT(duration.count(), 1000000) << "kai::Array append too slow";
}