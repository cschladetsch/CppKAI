#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Exception.h"
#include "KAI/Core/Logger.h"
#include "KAI/Language/Rho/RhoParser.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixture for testing advanced Rho operations
struct RhoAdvancedTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);

        // Register basic types needed for tests
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<float>(Label("float"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<String>(Label("String"));

        // Clear stacks to start fresh
        exec_->ClearStacks();
        exec_->ClearContext();
    }

    // Helper method to execute Rho code and verify the result
    template <typename T>
    void ExecuteRhoAndVerify(const std::string& code, const T& expected) {
        // Clear the stack first
        exec_->ClearStacks();

        // Execute the Rho code
        try {
            console_.Execute(code);
        } catch (const std::exception& e) {
            FAIL() << "Failed to execute code: " << code
                   << "\nError: " << e.what();
        }

        // Process the stack to extract values from continuations
        UnwrapStackValues();

        // Verify the result
        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";

        // Check if top exists before accessing it
        ASSERT_TRUE(data_->Top().Exists())
            << "Top of stack is null after executing: " << code;

        ASSERT_TRUE(data_->Top().IsType<T>())
            << "Expected result type " << typeid(T).name() << " but got "
            << data_->Top().GetClass()->GetName().ToString();

        ASSERT_EQ(ConstDeref<T>(data_->Top()), expected)
            << "Expected value " << expected << " but got "
            << ConstDeref<T>(data_->Top());
    }

    // Helper to verify string results
    void ExecuteRhoAndVerifyString(const std::string& code,
                                   const std::string& expected) {
        ExecuteRhoAndVerify<String>(code, String(expected));
    }
};

// Advanced arithmetic operations with precedence
TEST_F(RhoAdvancedTests, AdvancedArithmetic) {
    ExecuteRhoAndVerify<int>("2 + 3 * 4", 14);
    ExecuteRhoAndVerify<int>("(2 + 3) * 4", 20);
    ExecuteRhoAndVerify<int>("8 - 2 * 3", 2);
    ExecuteRhoAndVerify<int>("(8 - 2) * 3", 18);
    ExecuteRhoAndVerify<int>("16 / 4 / 2", 2);
    ExecuteRhoAndVerify<int>("16 / (4 / 2)", 8);
    ExecuteRhoAndVerify<int>("2 * 3 + 4 * 5", 26);
    ExecuteRhoAndVerify<int>("2 * (3 + 4) * 5", 70);
}

// Mixed type arithmetic
TEST_F(RhoAdvancedTests, MixedTypeArithmetic) {
    ExecuteRhoAndVerify<float>("2 + 3.5", 5.5f);
    ExecuteRhoAndVerify<float>("3.5 + 2", 5.5f);
    ExecuteRhoAndVerify<float>("2.5 * 4", 10.0f);
    ExecuteRhoAndVerify<float>("4 * 2.5", 10.0f);
    ExecuteRhoAndVerify<float>("10 / 2.5", 4.0f);
    ExecuteRhoAndVerify<float>("10.0 / 4", 2.5f);
    ExecuteRhoAndVerify<float>("10.5 - 3", 7.5f);
    ExecuteRhoAndVerify<float>("10 - 2.5", 7.5f);
}

// Complex expressions with multiple operators
TEST_F(RhoAdvancedTests, ComplexExpressions) {
    ExecuteRhoAndVerify<int>("2 + 3 * 4 - 6 / 2", 11);
    ExecuteRhoAndVerify<int>("(2 + 3) * (4 - 2)", 10);
    ExecuteRhoAndVerify<int>("8 - 4 + 2 * 5", 14);
    ExecuteRhoAndVerify<float>("3.5 * 2 + 10 / 2", 12.0f);
    ExecuteRhoAndVerify<float>("(10 + 6) / 8.0", 2.0f);
    ExecuteRhoAndVerify<int>("((2 + 3) * 4) / 2", 10);
    ExecuteRhoAndVerify<int>("2 * 3 + 4 * 5 - 6 / 2", 23);
}

// Modulus operations
TEST_F(RhoAdvancedTests, ModulusOperations) {
    ExecuteRhoAndVerify<int>("10 % 3", 1);
    ExecuteRhoAndVerify<int>("20 % 7", 6);
    ExecuteRhoAndVerify<int>("100 % 10", 0);
    ExecuteRhoAndVerify<int>("7 % 10", 7);
    ExecuteRhoAndVerify<int>("(10 + 5) % 7", 1);
    ExecuteRhoAndVerify<int>("10 % (3 + 4)", 3);
    ExecuteRhoAndVerify<int>("10 + 20 % 7", 16);
    ExecuteRhoAndVerify<int>("(10 + 20) % 7", 2);
}

// Bitwise operations
TEST_F(RhoAdvancedTests, BitwiseOperations) {
    ExecuteRhoAndVerify<int>("5 & 3", 1);     // 101 & 011 = 001
    ExecuteRhoAndVerify<int>("5 | 3", 7);     // 101 | 011 = 111
    ExecuteRhoAndVerify<int>("5 ^ 3", 6);     // 101 ^ 011 = 110
    ExecuteRhoAndVerify<int>("~5 & 15", 10);  // ~101 & 1111 = 1010
    ExecuteRhoAndVerify<int>("1 << 3", 8);    // 1 << 3 = 1000
    ExecuteRhoAndVerify<int>("8 >> 2", 2);    // 1000 >> 2 = 10
    ExecuteRhoAndVerify<int>("(5 & 3) | (4 & 2)",
                             1);  // (101 & 011) | (100 & 010) = 001 | 000 = 001
}

// Boolean logic operations
TEST_F(RhoAdvancedTests, BooleanOperations) {
    ExecuteRhoAndVerify<bool>("true && true", true);
    ExecuteRhoAndVerify<bool>("true && false", false);
    ExecuteRhoAndVerify<bool>("false && true", false);
    ExecuteRhoAndVerify<bool>("false && false", false);
    ExecuteRhoAndVerify<bool>("true || true", true);
    ExecuteRhoAndVerify<bool>("true || false", true);
    ExecuteRhoAndVerify<bool>("false || true", true);
    ExecuteRhoAndVerify<bool>("false || false", false);
    ExecuteRhoAndVerify<bool>("!true", false);
    ExecuteRhoAndVerify<bool>("!false", true);
    ExecuteRhoAndVerify<bool>("!(true && false)", true);
    ExecuteRhoAndVerify<bool>("!true || !false", true);
    ExecuteRhoAndVerify<bool>("!(false) && !(false)", true);
}

// Comparison operations
TEST_F(RhoAdvancedTests, ComparisonOperations) {
    ExecuteRhoAndVerify<bool>("5 > 3", true);
    ExecuteRhoAndVerify<bool>("3 > 5", false);
    ExecuteRhoAndVerify<bool>("5 < 3", false);
    ExecuteRhoAndVerify<bool>("3 < 5", true);
    ExecuteRhoAndVerify<bool>("5 >= 5", true);
    ExecuteRhoAndVerify<bool>("5 >= 6", false);
    ExecuteRhoAndVerify<bool>("5 <= 5", true);
    ExecuteRhoAndVerify<bool>("5 <= 4", false);
    ExecuteRhoAndVerify<bool>("5 == 5", true);
    ExecuteRhoAndVerify<bool>("5 == 6", false);
    ExecuteRhoAndVerify<bool>("5 != 6", true);
    ExecuteRhoAndVerify<bool>("5 != 5", false);
}

// Mixed boolean and comparison operations
TEST_F(RhoAdvancedTests, MixedBooleanComparison) {
    ExecuteRhoAndVerify<bool>("5 > 3 && 7 < 10", true);
    ExecuteRhoAndVerify<bool>("5 > 3 && 7 > 10", false);
    ExecuteRhoAndVerify<bool>("5 < 3 || 7 < 10", true);
    ExecuteRhoAndVerify<bool>("5 < 3 || 7 > 10", false);
    ExecuteRhoAndVerify<bool>("!(5 < 3) && 7 < 10", true);
    ExecuteRhoAndVerify<bool>("(5 > 3) == (7 < 10)", true);
    ExecuteRhoAndVerify<bool>("(5 > 3) != (7 > 10)", true);
    ExecuteRhoAndVerify<bool>("5 > 3 && 7 < 10 || 2 == 2", true);
    ExecuteRhoAndVerify<bool>("5 < 3 && (7 < 10 || 2 == 2)", false);
}

// String operations
TEST_F(RhoAdvancedTests, StringOperations) {
    ExecuteRhoAndVerifyString("\"Hello\" + \" \" + \"World\"", "Hello World");
    ExecuteRhoAndVerify<bool>("\"abc\" == \"abc\"", true);
    ExecuteRhoAndVerify<bool>("\"abc\" != \"def\"", true);
    ExecuteRhoAndVerify<bool>("\"abc\" == \"def\"", false);
    ExecuteRhoAndVerify<bool>("\"abc\" < \"def\"", true);
    ExecuteRhoAndVerify<bool>("\"def\" > \"abc\"", true);
    ExecuteRhoAndVerify<bool>("\"abc\" <= \"abc\"", true);
    ExecuteRhoAndVerify<bool>("\"def\" >= \"def\"", true);
}

// Complex string operations
TEST_F(RhoAdvancedTests, ComplexStringOperations) {
    ExecuteRhoAndVerifyString("\"prefix-\" + (\"middle\" + \"-suffix\")",
                              "prefix-middle-suffix");
    ExecuteRhoAndVerify<bool>("(\"a\" + \"b\") == (\"a\" + \"b\")", true);
    ExecuteRhoAndVerify<bool>("(\"a\" + \"b\") != (\"a\" + \"c\")", true);
    ExecuteRhoAndVerify<bool>("\"a\" + \"b\" < \"a\" + \"c\"", true);
    ExecuteRhoAndVerify<bool>("\"a\" + \"c\" > \"a\" + \"b\"", true);
    ExecuteRhoAndVerify<bool>("(\"a\" + \"b\") == \"ab\"", true);
}

// Advanced compound expressions
TEST_F(RhoAdvancedTests, AdvancedCompoundExpressions) {
    ExecuteRhoAndVerify<int>("(2 + 3) * 4 + 6 / 2", 23);
    ExecuteRhoAndVerify<float>("3.5 * (2 + 10) / 2", 21.0f);
    ExecuteRhoAndVerify<int>("((8 - 4) * 2 + 5) % 7", 6);
    ExecuteRhoAndVerify<int>("10 - (2 + 3) * (8 / 4)", 0);
    ExecuteRhoAndVerify<bool>("(5 > 3 && 7 < 10) || (2 == 3)", true);
    ExecuteRhoAndVerify<bool>("(5 < 3 || 7 > 10) && (2 != 2)", false);
    // DISABLED: String + number should throw a type error
    // ExecuteRhoAndVerifyString("\"Result: \" + ((5 + 3) * 2)", "Result: 16");
}