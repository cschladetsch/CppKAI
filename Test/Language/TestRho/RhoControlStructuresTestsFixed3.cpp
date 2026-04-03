#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixed version of RhoControlTests that properly uses the base class console
struct RhoControlTestsFixed : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Rho);
    }

    template <typename T>
    void RunAndExpect(const string& code, T expected) {
        try {
            // Clear the data stack
            auto executor = console_.GetExecutor();
            auto dataStack = executor->GetDataStack();
            dataStack->Clear();

            // Execute as a complete program
            console_.Execute(code, Structure::Program);

            // Check result
            ASSERT_FALSE(dataStack->Empty())
                << "No result on stack after execution";

            auto result = dataStack->Top();
            ASSERT_TRUE(result.IsType<T>())
                << "Result type mismatch. Expected " << typeid(T).name()
                << " but got "
                << (result.GetClass() ? result.GetClass()->GetName().ToString()
                                      : "null");

            T actual = ConstDeref<T>(result);
            ASSERT_EQ(actual, expected) << "Value mismatch";

        } catch (const Exception::Base& e) {
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception& e) {
            FAIL() << "std::exception: " << e.what();
        }
    }
};

// Test the for loops that were failing
TEST_F(RhoControlTestsFixed, ForLoopsWithProperBase) {
    RunAndExpect<int>(R"(
sum = 0
for i = 1; i <= 5; i = i + 1
    sum = sum + i
sum
)",
                      15);

    RunAndExpect<int>(R"(
sum = 0
for i = 0; i < 10; i = i + 2
    sum = sum + i
sum
)",
                      20);
}

// Test while loops
TEST_F(RhoControlTestsFixed, WhileLoopsWithProperBase) {
    RunAndExpect<int>(R"(
sum = 0
i = 1
while i <= 5
    sum = sum + i
    i = i + 1
sum
)",
                      15);
}

// Test nested for loops
TEST_F(RhoControlTestsFixed, NestedForLoopsWithProperBase) {
    RunAndExpect<int>(R"(
sum = 0
for i = 1; i <= 3; i = i + 1
    for j = 1; j <= 3; j = j + 1
        sum = sum + (i * j)
sum
)",
                      36);
}