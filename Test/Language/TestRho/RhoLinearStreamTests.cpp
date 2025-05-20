#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Object/ClassBuilder.h"
#include "KAI/Executor/Operation.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Class for running Rho scripts in our test environment
class RhoScriptTest : public TestLangCommon {
   protected:
    bool RunRhoScript(const std::string& script) {
        console_.SetLanguage(Language::Rho);
        data_->Clear();

        try {
            console_.Execute(script.c_str());

            // Process the stack after execution to extract values
            UnwrapStackValues();

            return true;
        } catch (const Exception::Base& e) {
            std::cerr << "Exception running script: " << e.ToString()
                      << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "std::exception: " << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cerr << "Unknown exception" << std::endl;
            return false;
        }
    }
};

// This file contains additional tests for the Rho language implementation
// focusing on operations that benefit from the linear stream approach

// Test suite for basic arithmetic operations in Rho
TEST_F(RhoScriptTest, IntAddition) {
    ASSERT_TRUE(RunRhoScript("2 + 3"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 5);
}

TEST_F(RhoScriptTest, IntSubtraction) {
    ASSERT_TRUE(RunRhoScript("10 - 4"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 6);
}

TEST_F(RhoScriptTest, IntMultiplication) {
    ASSERT_TRUE(RunRhoScript("6 * 7"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 42);
}

TEST_F(RhoScriptTest, IntDivision) {
    ASSERT_TRUE(RunRhoScript("20 / 4"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 5);
}

TEST_F(RhoScriptTest, IntModulo) {
    ASSERT_TRUE(RunRhoScript("17 % 5"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 2);
}

// Test suite for floating point arithmetic in Rho
TEST_F(RhoScriptTest, FloatAddition) {
    ASSERT_TRUE(RunRhoScript("3.5 + 2.25"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(AtData<float>(0), 5.75f);
}

TEST_F(RhoScriptTest, FloatSubtraction) {
    ASSERT_TRUE(RunRhoScript("7.5 - 2.5"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(AtData<float>(0), 5.0f);
}

TEST_F(RhoScriptTest, FloatMultiplication) {
    ASSERT_TRUE(RunRhoScript("3.5 * 2.0"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(AtData<float>(0), 7.0f);
}

TEST_F(RhoScriptTest, FloatDivision) {
    ASSERT_TRUE(RunRhoScript("10.0 / 2.5"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(AtData<float>(0), 4.0f);
}

// Test suite for mixed type arithmetic in Rho
TEST_F(RhoScriptTest, MixedAddition) {
    ASSERT_TRUE(RunRhoScript("5 + 2.5"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(AtData<float>(0), 7.5f);
}

TEST_F(RhoScriptTest, MixedSubtraction) {
    ASSERT_TRUE(RunRhoScript("5.5 - 2"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(AtData<float>(0), 3.5f);
}

TEST_F(RhoScriptTest, MixedMultiplication) {
    ASSERT_TRUE(RunRhoScript("4.5 * 2"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(AtData<float>(0), 9.0f);
}

TEST_F(RhoScriptTest, MixedDivision) {
    ASSERT_TRUE(RunRhoScript("10 / 2.5"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(AtData<float>(0), 4.0f);
}

// Test suite for comparison operations
TEST_F(RhoScriptTest, IntEquality) {
    ASSERT_TRUE(RunRhoScript("5 == 5"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(AtData<bool>(0));
}

TEST_F(RhoScriptTest, IntInequality) {
    ASSERT_TRUE(RunRhoScript("5 != 7"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(AtData<bool>(0));
}

TEST_F(RhoScriptTest, IntLessThan) {
    ASSERT_TRUE(RunRhoScript("5 < 10"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(AtData<bool>(0));
}

TEST_F(RhoScriptTest, IntGreaterThan) {
    ASSERT_TRUE(RunRhoScript("15 > 10"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(AtData<bool>(0));
}

// Test suite for logical operations
TEST_F(RhoScriptTest, LogicalAnd) {
    ASSERT_TRUE(RunRhoScript("true && true"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(AtData<bool>(0));
}

TEST_F(RhoScriptTest, LogicalOr) {
    ASSERT_TRUE(RunRhoScript("false || true"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(AtData<bool>(0));
}

TEST_F(RhoScriptTest, LogicalNot) {
    ASSERT_TRUE(RunRhoScript("!false"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(AtData<bool>(0));
}

// Test suite for string operations
TEST_F(RhoScriptTest, StringConcatenation) {
    ASSERT_TRUE(RunRhoScript("\"Hello \" + \"World\""));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<String>());
    ASSERT_EQ(AtData<String>(0), "Hello World");
}

// Test suite for complex expressions
TEST_F(RhoScriptTest, ComplexExpression1) {
    ASSERT_TRUE(RunRhoScript("(2 + 3) * 4"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 20);
}

TEST_F(RhoScriptTest, ComplexExpression2) {
    ASSERT_TRUE(RunRhoScript("10 - 2 * 3"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 4);
}

// Test suite for Pi operations embedded in Rho
TEST_F(RhoScriptTest, SimplePiOperation) {
    ASSERT_TRUE(RunRhoScript("'2 3 +'"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 5);
}

TEST_F(RhoScriptTest, PiStackOperations) {
    ASSERT_TRUE(RunRhoScript("'5 dup +'"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 10);
}

// Test suite for assertion operations
TEST_F(RhoScriptTest, BasicAssertion) {
    ASSERT_TRUE(RunRhoScript("assert true"));
    // Successful assertion leaves nothing on stack
    ASSERT_TRUE(data_->Empty());
}

// Test suite for mixed Rho and Pi operations
TEST_F(RhoScriptTest, MixedRhoPiExpressions) {
    ASSERT_TRUE(RunRhoScript("2 + '3 4 +'"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 9);
}

// Test suite for multi-operation expressions
TEST_F(RhoScriptTest, MultiOperation) {
    ASSERT_TRUE(RunRhoScript("2 + 3 * 4 - 5"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 9);
}

// Test suite for boolean expressions with mixed operations
TEST_F(RhoScriptTest, BooleanExpression) {
    ASSERT_TRUE(RunRhoScript("(5 > 3) && (4 < 10)"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(AtData<bool>(0));
}

// Test suite for complex nested expressions
TEST_F(RhoScriptTest, NestedExpression) {
    ASSERT_TRUE(RunRhoScript("((2 + 3) * 4) / 2"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(AtData<int>(0), 10);
}

// Test suite for type conversion and preservation
TEST_F(RhoScriptTest, TypeConversion) {
    ASSERT_TRUE(RunRhoScript("2 + 3.5"));
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<float>());
    ASSERT_FLOAT_EQ(AtData<float>(0), 5.5f);
}