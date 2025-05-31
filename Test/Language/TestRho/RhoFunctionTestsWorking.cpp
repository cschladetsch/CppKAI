#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

// Test fixture for working Rho function tests
struct RhoFunctionTestsWorking : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }

    template <typename T>
    void RunAndExpect(const std::string& code, T expected) {
        try {
            // Clear the data stack
            auto executor = console_.GetExecutor();
            auto dataStack = executor->GetDataStack();
            dataStack->Clear();

            // Execute as a complete program
            console_.Execute(code, kai::Structure::Program);

            // Check result
            ASSERT_FALSE(dataStack->Empty())
                << "No result on stack after execution";

            auto result = dataStack->Top();
            ASSERT_TRUE(result.IsType<T>())
                << "Result type mismatch. Expected " << typeid(T).name()
                << " but got "
                << (result.GetClass() ? result.GetClass()->GetName().ToString()
                                      : "null");

            T actual = kai::ConstDeref<T>(result);
            ASSERT_EQ(actual, expected) << "Value mismatch";

        } catch (const kai::Exception::Base& e) {
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception& e) {
            FAIL() << "std::exception: " << e.what();
        }
    }
};

// Basic function definition and call - using correct syntax
TEST_F(RhoFunctionTestsWorking, BasicFunction) {
    RunAndExpect<int>(R"(
fun add(a, b)
    return a + b

add(2, 3)
)",
                      5);

    RunAndExpect<int>(R"(
fun multiply(a, b)
    return a * b

multiply(4, 5)
)",
                      20);
}

// Functions with multiple statements
TEST_F(RhoFunctionTestsWorking, MultiStatementFunction) {
    RunAndExpect<int>(R"(
fun computeSum(n)
    sum = 0
    i = 1
    while i <= n
        sum = sum + i
        i = i + 1
    return sum

computeSum(5)
)",
                      15);
}

// Functions without explicit return
TEST_F(RhoFunctionTestsWorking, ImplicitReturn) {
    RunAndExpect<int>(R"(
fun double(x)
    x * 2

double(21)
)",
                      42);
}
