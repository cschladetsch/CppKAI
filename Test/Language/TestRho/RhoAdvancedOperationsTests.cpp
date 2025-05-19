#include <fstream>
#include <sstream>

#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include <gtest/gtest.h>
#include "KAI/Language/Rho/RhoParser.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "KAI/Core/BuiltinTypes/Stack.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixture for testing advanced Rho operations
struct RhoAdvancedTests : TestLangCommon
{
    // Helper method to execute a Rho script and verify the value on top of stack
    template <class T>
    void AssertResult(const char *script, T expected, bool verbose = false)
    {
        if (verbose)
        {
            KAI_LOG_INFO(std::string("Testing script: ") + script);
        }
        
        try
        {
            Registry reg;
            Console console(reg);
            console.SetScope(reg.GetGlobalScope());
            
            auto result = console.Execute(script);
            
            if (result.Failed)
            {
                if (verbose)
                {
                    KAI_LOG_ERROR("Execution failed: " + result.Error);
                }
                FAIL() << "Error executing script: " << result.Error;
                return;
            }
            
            if (console.GetExecutor().GetStack().Size() == 0)
            {
                if (verbose)
                {
                    KAI_LOG_WARNING("Stack is empty, cannot verify result");
                }
                FAIL() << "Stack is empty, cannot verify result";
                return;
            }
            
            auto val = console.GetExecutor().GetStack().Top();
            if (val.GetType() != Type::Traits<T>::TypeNumber)
            {
                if (verbose)
                {
                    KAI_LOG_ERROR("Type mismatch. Expected: " + 
                        std::to_string(Type::Traits<T>::TypeNumber) + 
                        ", Got: " + std::to_string(val.GetType()));
                }
                FAIL() << "Type mismatch. Expected: " << Type::Traits<T>::TypeNumber
                       << ", Got: " << val.GetType();
                return;
            }
            
            T actual = kai_cast<T>(val);
            if (verbose)
            {
                KAI_LOG_INFO("Result: " + std::to_string(actual));
            }
            ASSERT_EQ(expected, actual) << "Result doesn't match expected value";
        }
        catch (const Exception &e)
        {
            if (verbose)
            {
                KAI_LOG_ERROR("Exception: " + std::string(e.ToString()));
            }
            FAIL() << "Exception: " << e.ToString();
        }
        catch (const std::exception &e)
        {
            if (verbose)
            {
                KAI_LOG_ERROR("std::exception: " + std::string(e.what()));
            }
            FAIL() << "std::exception: " << e.what();
        }
        catch (...)
        {
            if (verbose)
            {
                KAI_LOG_ERROR("Unknown exception");
            }
            FAIL() << "Unknown exception";
        }
    }
};

// Advanced arithmetic operations with precedence
TEST_F(RhoAdvancedTests, AdvancedArithmetic)
{
    AssertResult<int>("2 + 3 * 4", 14);
    AssertResult<int>("(2 + 3) * 4", 20);
    AssertResult<int>("8 - 2 * 3", 2);
    AssertResult<int>("(8 - 2) * 3", 18);
    AssertResult<int>("16 / 4 / 2", 2);
    AssertResult<int>("16 / (4 / 2)", 8);
    AssertResult<int>("2 * 3 + 4 * 5", 26);
    AssertResult<int>("2 * (3 + 4) * 5", 70);
}

// Mixed type arithmetic
TEST_F(RhoAdvancedTests, MixedTypeArithmetic)
{
    AssertResult<float>("2 + 3.5", 5.5f);
    AssertResult<float>("3.5 + 2", 5.5f);
    AssertResult<float>("2.5 * 4", 10.0f);
    AssertResult<float>("4 * 2.5", 10.0f);
    AssertResult<float>("10 / 2.5", 4.0f);
    AssertResult<float>("10.0 / 4", 2.5f);
    AssertResult<float>("10.5 - 3", 7.5f);
    AssertResult<float>("10 - 2.5", 7.5f);
}

// Complex expressions with multiple operators
TEST_F(RhoAdvancedTests, ComplexExpressions)
{
    AssertResult<int>("2 + 3 * 4 - 6 / 2", 11);
    AssertResult<int>("(2 + 3) * (4 - 2)", 10);
    AssertResult<int>("8 - 4 + 2 * 5", 14);
    AssertResult<float>("3.5 * 2 + 10 / 2", 12.0f);
    AssertResult<float>("(10 + 6) / 8.0", 2.0f);
    AssertResult<int>("((2 + 3) * 4) / 2", 10);
    AssertResult<int>("2 * 3 + 4 * 5 - 6 / 2", 23);
}

// Modulus operations
TEST_F(RhoAdvancedTests, ModulusOperations)
{
    AssertResult<int>("10 % 3", 1);
    AssertResult<int>("20 % 7", 6);
    AssertResult<int>("100 % 10", 0);
    AssertResult<int>("7 % 10", 7);
    AssertResult<int>("(10 + 5) % 7", 1);
    AssertResult<int>("10 % (3 + 4)", 3);
    AssertResult<int>("10 + 20 % 7", 16);
    AssertResult<int>("(10 + 20) % 7", 2);
}

// Bitwise operations
TEST_F(RhoAdvancedTests, BitwiseOperations)
{
    AssertResult<int>("5 & 3", 1);     // 101 & 011 = 001
    AssertResult<int>("5 | 3", 7);     // 101 | 011 = 111
    AssertResult<int>("5 ^ 3", 6);     // 101 ^ 011 = 110
    AssertResult<int>("~5 & 15", 10);  // ~101 & 1111 = 1010
    AssertResult<int>("1 << 3", 8);    // 1 << 3 = 1000
    AssertResult<int>("8 >> 2", 2);    // 1000 >> 2 = 10
    AssertResult<int>("(5 & 3) | (4 & 2)", 1); // (101 & 011) | (100 & 010) = 001 | 000 = 001
}

// Boolean logic operations
TEST_F(RhoAdvancedTests, BooleanOperations)
{
    AssertResult<bool>("true && true", true);
    AssertResult<bool>("true && false", false);
    AssertResult<bool>("false && true", false);
    AssertResult<bool>("false && false", false);
    AssertResult<bool>("true || true", true);
    AssertResult<bool>("true || false", true);
    AssertResult<bool>("false || true", true);
    AssertResult<bool>("false || false", false);
    AssertResult<bool>("!true", false);
    AssertResult<bool>("!false", true);
    AssertResult<bool>("!(true && false)", true);
    AssertResult<bool>("!true || !false", true);
    AssertResult<bool>("!(false) && !(false)", true);
}

// Comparison operations
TEST_F(RhoAdvancedTests, ComparisonOperations)
{
    AssertResult<bool>("5 > 3", true);
    AssertResult<bool>("3 > 5", false);
    AssertResult<bool>("5 < 3", false);
    AssertResult<bool>("3 < 5", true);
    AssertResult<bool>("5 >= 5", true);
    AssertResult<bool>("5 >= 6", false);
    AssertResult<bool>("5 <= 5", true);
    AssertResult<bool>("5 <= 4", false);
    AssertResult<bool>("5 == 5", true);
    AssertResult<bool>("5 == 6", false);
    AssertResult<bool>("5 != 6", true);
    AssertResult<bool>("5 != 5", false);
}

// Mixed boolean and comparison operations
TEST_F(RhoAdvancedTests, MixedBooleanComparison)
{
    AssertResult<bool>("5 > 3 && 7 < 10", true);
    AssertResult<bool>("5 > 3 && 7 > 10", false);
    AssertResult<bool>("5 < 3 || 7 < 10", true);
    AssertResult<bool>("5 < 3 || 7 > 10", false);
    AssertResult<bool>("!(5 < 3) && 7 < 10", true);
    AssertResult<bool>("(5 > 3) == (7 < 10)", true);
    AssertResult<bool>("(5 > 3) != (7 > 10)", true);
    AssertResult<bool>("5 > 3 && 7 < 10 || 2 == 2", true);
    AssertResult<bool>("5 < 3 && (7 < 10 || 2 == 2)", false);
}

// String operations
TEST_F(RhoAdvancedTests, StringOperations)
{
    AssertResult<String>("\"Hello\" + \" \" + \"World\"", String("Hello World"));
    AssertResult<bool>("\"abc\" == \"abc\"", true);
    AssertResult<bool>("\"abc\" != \"def\"", true);
    AssertResult<bool>("\"abc\" == \"def\"", false);
    AssertResult<bool>("\"abc\" < \"def\"", true);
    AssertResult<bool>("\"def\" > \"abc\"", true);
    AssertResult<bool>("\"abc\" <= \"abc\"", true);
    AssertResult<bool>("\"def\" >= \"def\"", true);
}

// Complex string operations
TEST_F(RhoAdvancedTests, ComplexStringOperations)
{
    AssertResult<String>("\"prefix-\" + (\"middle\" + \"-suffix\")", String("prefix-middle-suffix"));
    AssertResult<bool>("(\"a\" + \"b\") == (\"a\" + \"b\")", true);
    AssertResult<bool>("(\"a\" + \"b\") != (\"a\" + \"c\")", true);
    AssertResult<bool>("\"a\" + \"b\" < \"a\" + \"c\"", true);
    AssertResult<bool>("\"a\" + \"c\" > \"a\" + \"b\"", true);
    AssertResult<bool>("(\"a\" + \"b\") == \"ab\"", true);
}

// String and number operations
TEST_F(RhoAdvancedTests, StringNumberOperations)
{
    AssertResult<String>("\"The answer is: \" + (40 + 2)", String("The answer is: 42"));
    AssertResult<String>("\"Pi: \" + 3.14159", String("Pi: 3.14159"));
    AssertResult<String>("\"Count: \" + 10", String("Count: 10"));
    AssertResult<String>("\"Bool: \" + true", String("Bool: true"));
    AssertResult<String>("\"Bool: \" + false", String("Bool: false"));
}

// Advanced compound expressions
TEST_F(RhoAdvancedTests, AdvancedCompoundExpressions)
{
    AssertResult<int>("(2 + 3) * 4 + 6 / 2", 23);
    AssertResult<float>("3.5 * (2 + 10) / 2", 21.0f);
    AssertResult<int>("((8 - 4) * 2 + 5) % 7", 3);
    AssertResult<int>("10 - (2 + 3) * (8 / 4)", 0);
    AssertResult<bool>("(5 > 3 && 7 < 10) || (2 == 3)", true);
    AssertResult<bool>("(5 < 3 || 7 > 10) && (2 != 2)", false);
    AssertResult<String>("\"Result: \" + ((5 + 3) * 2)", String("Result: 16"));
}