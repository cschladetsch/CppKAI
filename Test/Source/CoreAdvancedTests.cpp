#include <gtest/gtest.h>

#include <chrono>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Exception.h"
#include "KAI/Core/Object.h"
#include "TestCommon.h"

// Advanced Core tests
struct CoreAdvancedTests : TestCommon {};

// Test 1: Deep object cloning
TEST_F(CoreAdvancedTests, DeepObjectCloning) {
    kai::Object original = reg.New<int>(42);
    original.SetLabel("original");

    kai::Object clone = original.Clone();
    ASSERT_TRUE(clone.Valid());
    ASSERT_EQ(kai::ConstDeref<int>(original), kai::ConstDeref<int>(clone));
    ASSERT_NE(original.GetHandle(), clone.GetHandle());
}

// Test 2: kai::Object property system
TEST_F(CoreAdvancedTests, ObjectProperties) {
    kai::Object obj = reg.New<kai::String>("test");
    obj.SetProperty("custom", reg.New<int>(123));

    ASSERT_TRUE(obj.HasProperty("custom"));
    kai::Object prop = obj.GetProperty("custom");
    ASSERT_EQ(kai::ConstDeref<int>(prop), 123);
}

// Test 3: Registry stress test
TEST_F(CoreAdvancedTests, RegistryStressTest) {
    const int NUM_OBJECTS = 1000;
    std::vector<kai::Object> objects;

    for (int i = 0; i < NUM_OBJECTS; ++i) {
        objects.push_back(reg.New<int>(i));
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
    kai::Object intObj = reg.New<int>(42);
    kai::Object strObj = reg.New<kai::String>("hello");
    kai::Object vecObj = reg.New<kai::Vector>();

    ASSERT_TRUE(intObj.IsType<int>());
    ASSERT_FALSE(intObj.IsType<kai::String>());

    ASSERT_TRUE(strObj.IsType<kai::String>());
    ASSERT_FALSE(strObj.IsType<int>());

    ASSERT_TRUE(vecObj.IsType<kai::Vector>());
    ASSERT_FALSE(vecObj.IsType<int>());
}

// Test 5: Memory management with custom allocators
TEST_F(CoreAdvancedTests, CustomMemoryManagement) {
    // Test object creation and deletion patterns
    for (int i = 0; i < 100; ++i) {
        kai::Object obj = reg.New<kai::String>("test" + std::to_string(i));
        ASSERT_TRUE(obj.Valid());
        // kai::Object will be cleaned up automatically
    }

    // Force garbage collection
    reg.TriColor();

    // Registry should still be functional
    kai::Object afterGC = reg.New<int>(999);
    ASSERT_TRUE(afterGC.Valid());
    ASSERT_EQ(kai::ConstDeref<int>(afterGC), 999);
}

// Test 6: Complex container operations
TEST_F(CoreAdvancedTests, ComplexContainerOps) {
    kai::Object dict = reg.New<kai::Dictionary>();

    // Add nested structures
    for (int i = 0; i < 10; ++i) {
        kai::Object innerVec = reg.New<kai::Vector>();
        for (int j = 0; j < 5; ++j) {
            innerVec.GetClass()->Append(innerVec, reg.New<int>(i * 10 + j));
        }
        dict.GetClass()->Insert(
            dict, reg.New<kai::String>("key" + std::to_string(i)), innerVec);
    }

    // Verify structure
    ASSERT_EQ(dict.GetClass()->Size(dict), 10);

    kai::Object vec = dict.GetClass()->At(dict, reg.New<kai::String>("key5"));
    ASSERT_TRUE(vec.Valid());
    ASSERT_EQ(vec.GetClass()->Size(vec), 5);
}

// Test 7: Exception handling robustness
TEST_F(CoreAdvancedTests, ExceptionHandlingRobustness) {
    try {
        // Try to create object with invalid type
        kai::Object invalid = reg.GetObject(kai::Handle(9999999));
        FAIL() << "Expected exception for invalid handle";
    } catch (const kai::Exception::Base &e) {
        // Expected
        SUCCEED();
    }

    try {
        kai::Object obj = reg.New<int>(42);
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

// Test 8: Binary serialization round-trip
TEST_F(CoreAdvancedTests, BinarySerializationRoundTrip) {
    // Create complex object graph
    kai::Object root = reg.New<kai::Dictionary>();
    root.SetLabel("root");

    kai::Object arr = reg.New<kai::Vector>();
    for (int i = 0; i < 5; ++i) {
        arr.GetClass()->Append(arr, reg.New<float>(i * 1.5f));
    }

    root.GetClass()->Insert(root, reg.New<kai::String>("array"), arr);
    root.GetClass()->Insert(root, reg.New<kai::String>("name"),
                            reg.New<kai::String>("test"));
    root.GetClass()->Insert(root, reg.New<kai::String>("count"),
                            reg.New<int>(42));

    // Serialize
    kai::BinaryStream stream(reg);
    stream << root;

    // Deserialize
    stream.ToStart();
    kai::Object loaded;
    stream >> loaded;

    ASSERT_TRUE(loaded.Valid());
    ASSERT_TRUE(loaded.IsType<kai::Dictionary>());
    ASSERT_EQ(loaded.GetClass()->Size(loaded), 3);
}

// Test 9: Thread-local storage simulation
TEST_F(CoreAdvancedTests, ThreadLocalStorage) {
    // Simulate thread-local storage using object properties
    kai::Object threadContext = reg.New<kai::Dictionary>();

    // Set thread-specific data
    threadContext.SetProperty("threadId", reg.New<int>(12345));
    threadContext.SetProperty("userName", reg.New<kai::String>("testuser"));
    threadContext.SetProperty("permissions", reg.New<int>(0x755));

    // Retrieve thread-specific data
    ASSERT_TRUE(threadContext.HasProperty("threadId"));
    ASSERT_EQ(kai::ConstDeref<int>(threadContext.GetProperty("threadId")),
              12345);

    ASSERT_TRUE(threadContext.HasProperty("userName"));
    ASSERT_EQ(
        kai::ConstDeref<kai::String>(threadContext.GetProperty("userName")),
        "testuser");
}

// Test 10: Performance benchmarking utilities
TEST_F(CoreAdvancedTests, PerformanceBenchmarking) {
    const int ITERATIONS = 10000;

    // Benchmark object creation
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i) {
        kai::Object obj = reg.New<int>(i);
        (void)obj;  // Prevent optimization
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Just verify it completes in reasonable time (< 1 second)
    ASSERT_LT(duration.count(), 1000000) << "kai::Object creation too slow";

    // Benchmark method calls
    kai::Object vec = reg.New<kai::Vector>();
    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i) {
        vec.GetClass()->Append(vec, reg.New<int>(i));
    }

    end = std::chrono::high_resolution_clock::now();
    duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    ASSERT_EQ(vec.GetClass()->Size(vec), ITERATIONS);
    ASSERT_LT(duration.count(), 1000000) << "kai::Vector append too slow";
}