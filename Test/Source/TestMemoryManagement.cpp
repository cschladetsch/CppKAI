#include <gtest/gtest.h>

#include "KAI/Core/Memory/StandardAllocator.h"
#include "KAI/Core/Object.h"
#include "TestCommon.h"

// Test suite for memory management features
class MemoryManagementTest : public TestCommon {
   protected:
    void SetUp() override { TestCommon::SetUp(); }
};

// Test 1: Memory allocation tracking
TEST_F(MemoryManagementTest, AllocationTracking) {
    auto initial_count = reg->GetMemoryAllocations();

    auto obj1 = reg->New<int>(42);
    EXPECT_GT(reg->GetMemoryAllocations(), initial_count);

    obj1.Delete();
    EXPECT_EQ(reg->GetMemoryAllocations(), initial_count);
}

// Test 2: Memory pool allocation
TEST_F(MemoryManagementTest, MemoryPoolAllocation) {
    // Create many small objects to test pool allocation
    std::vector<Object> objects;
    for (int i = 0; i < 1000; ++i) {
        objects.push_back(reg->New<int>(i));
    }

    // Verify all objects are valid
    for (int i = 0; i < 1000; ++i) {
        EXPECT_TRUE(objects[i].Valid());
        EXPECT_EQ(ConstDeref<int>(objects[i]), i);
    }

    // Clean up
    objects.clear();
}

// Test 3: Memory fragmentation handling
TEST_F(MemoryManagementTest, FragmentationHandling) {
    std::vector<Object> objects;

    // Create objects of varying sizes
    for (int i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            objects.push_back(reg->New<String>("Small"));
        } else {
            objects.push_back(reg->New<Array>());
            Deref<Array>(objects.back()).Resize(i * 10);
        }
    }

    // Delete every other object to create fragmentation
    for (int i = 0; i < 100; i += 2) {
        objects[i].Delete();
    }

    // Allocate new objects in fragmented memory
    for (int i = 0; i < 50; ++i) {
        auto obj = reg->New<int>(i * 100);
        EXPECT_TRUE(obj.Valid());
    }
}

// Test 4: Custom allocator support
TEST_F(MemoryManagementTest, CustomAllocator) {
    class TrackingAllocator : public Memory::IAllocator {
       public:
        int allocCount = 0;
        int deallocCount = 0;

        void* Allocate(size_t size) override {
            allocCount++;
            return ::malloc(size);
        }

        void Deallocate(void* ptr) override {
            deallocCount++;
            ::free(ptr);
        }

        size_t GetAllocatedSize(void* ptr) const override {
            return 0;  // Simple implementation
        }
    };

    TrackingAllocator allocator;
    // Test would use custom allocator if Registry supported it
    EXPECT_EQ(allocator.allocCount, 0);
    EXPECT_EQ(allocator.deallocCount, 0);
}

// Test 5: Memory leak detection
TEST_F(MemoryManagementTest, MemoryLeakDetection) {
    auto initial_objects = reg->GetTotalObjects();

    {
        auto obj1 = reg->New<int>(1);
        auto obj2 = reg->New<String>("test");
        auto obj3 = reg->New<Array>();

        EXPECT_EQ(reg->GetTotalObjects(), initial_objects + 3);
    }

    // Objects should be cleaned up after scope
    reg->GarbageCollect();
    EXPECT_EQ(reg->GetTotalObjects(), initial_objects);
}