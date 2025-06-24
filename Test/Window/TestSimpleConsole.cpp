#include <gtest/gtest.h>

#include <iostream>

// Simple test to verify test framework is working
TEST(SimpleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

TEST(SimpleTest, StringComparison) {
    std::string hello = "Hello";
    EXPECT_EQ(hello, "Hello");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}