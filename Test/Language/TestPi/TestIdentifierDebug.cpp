#include <iostream>

#include "KAI/Test/Include/TestLangCommon.h"

class TestIdentifierDebug : public kai::TestLangCommon {
   protected:
    void SetUp() override { TestLangCommon::SetUp(); }
};

TEST_F(TestIdentifierDebug, TestSimpleIdentifier) {
    // First, test storing and retrieving a simple value
    console_.Execute("42 'answer #");
    console_.Execute("answer");

    ASSERT_EQ(data_->Size(), 1);
    auto val = data_->Pop();
    ASSERT_TRUE(val.IsType<int>());
    ASSERT_EQ(kai::ConstDeref<int>(val), 42);
}

TEST_F(TestIdentifierDebug, TestMultipleIdentifiers) {
    // Store two values
    console_.Execute("10 'x # 20 'y #");

    // Print stack state
    std::cout << "After storing, stack size: " << data_->Size() << std::endl;

    // Retrieve x
    console_.Execute("x");
    std::cout << "After 'x', stack size: " << data_->Size() << std::endl;
    if (data_->Size() > 0) {
        auto top = data_->Top();
        if (top.IsType<int>()) {
            std::cout << "  Top value: " << kai::ConstDeref<int>(top)
                      << std::endl;
        }
    }

    // Retrieve y
    console_.Execute("y");
    std::cout << "After 'y', stack size: " << data_->Size() << std::endl;
    if (data_->Size() > 1) {
        auto top = data_->Top();
        if (top.IsType<int>()) {
            std::cout << "  Top value: " << kai::ConstDeref<int>(top)
                      << std::endl;
        }
    }

    // Retrieve x again
    console_.Execute("x");
    std::cout << "After second 'x', stack size: " << data_->Size() << std::endl;

    // Check final stack
    ASSERT_EQ(data_->Size(), 3);

    // Pop and check values
    auto val3 = data_->Pop();
    ASSERT_TRUE(val3.IsType<int>());
    ASSERT_EQ(kai::ConstDeref<int>(val3), 10)
        << "Third value should be 10 (second x)";

    auto val2 = data_->Pop();
    ASSERT_TRUE(val2.IsType<int>());
    ASSERT_EQ(kai::ConstDeref<int>(val2), 20)
        << "Second value should be 20 (y)";

    auto val1 = data_->Pop();
    ASSERT_TRUE(val1.IsType<int>());
    ASSERT_EQ(kai::ConstDeref<int>(val1), 10)
        << "First value should be 10 (first x)";
}

TEST_F(TestIdentifierDebug, TestSingleExecute) {
    // All in one execute call
    console_.Execute("10 'x # 20 'y # x y x");

    std::cout << "After single execute, stack size: " << data_->Size()
              << std::endl;

    // Check final stack
    ASSERT_EQ(data_->Size(), 3);

    // Print all values
    for (int i = 0; i < 3; i++) {
        auto val = data_->At(i);
        if (val.IsType<int>()) {
            std::cout << "  Stack[" << i << "]: " << kai::ConstDeref<int>(val)
                      << std::endl;
        }
    }
}