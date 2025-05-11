#include <algorithm>

#include "TestCommon.h"

USING_NAMESPACE_KAI

struct TestAdditionalStringOps : TestCommon {
   protected:
    void AddRequiredClasses() override {
        Reg().AddClass<bool>();
        Reg().AddClass<int>();
        Reg().AddClass<String>();
    }
};

// Test string operations using manual string comparisons
TEST_F(TestAdditionalStringOps, TestContentChecks) {
    Pointer<String> str = Reg().New<String>("Hello, World!");
    std::string stdStr = str->StdString();

    // Test if string contains substrings (manual implementation)
    ASSERT_TRUE(stdStr.find("Hello") != std::string::npos);
    ASSERT_TRUE(stdStr.find("World") != std::string::npos);
    ASSERT_TRUE(stdStr.find(", ") != std::string::npos);
    ASSERT_FALSE(stdStr.find("Goodbye") != std::string::npos);

    // Test if string starts with prefix (manual implementation)
    ASSERT_TRUE(stdStr.find("Hello") == 0);
    ASSERT_FALSE(stdStr.find("World") == 0);

    // Test if string ends with suffix (manual implementation)
    ASSERT_TRUE(stdStr[stdStr.length() - 1] == '!');
    ASSERT_TRUE(stdStr.rfind("World!") == stdStr.length() - 6);
    ASSERT_FALSE(stdStr.rfind("Hello") == stdStr.length() - 5);
}

// Test string case conversion using manual transformations
TEST_F(TestAdditionalStringOps, TestCaseConversion) {
    Pointer<String> str1 = Reg().New<String>("Hello, World!");
    std::string stdStr = str1->StdString();

    // Manually create uppercase version
    std::string upperStr = stdStr;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(),
                   ::toupper);
    ASSERT_EQ(upperStr, "HELLO, WORLD!");

    // Manually create lowercase version
    std::string lowerStr = stdStr;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   ::tolower);
    ASSERT_EQ(lowerStr, "hello, world!");
}

// Test string size and empty operations
TEST_F(TestAdditionalStringOps, TestSizeAndEmpty) {
    Pointer<String> str1 = Reg().New<String>("Hello, World!");
    Pointer<String> str2 = Reg().New<String>("");

    // Test size methods
    ASSERT_EQ(str1->size(), 13);
    ASSERT_EQ(str1->Size(), 13);
    ASSERT_EQ(str2->size(), 0);

    // Test empty methods
    ASSERT_FALSE(str1->empty());
    ASSERT_FALSE(str1->Empty());
    ASSERT_TRUE(str2->empty());
    ASSERT_TRUE(str2->Empty());
}