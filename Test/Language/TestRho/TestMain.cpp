#include <gtest/gtest.h>

#include <iostream>

// Custom main file to set up the test environment
int main(int argc, char **argv) {
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Enable all tests - remove filters that were disabling tests
    // All tests should now run including the previously filtered ones

    // Run the tests
    return RUN_ALL_TESTS();
}