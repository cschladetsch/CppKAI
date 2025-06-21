#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Logger.h"
#include "KAI/Language/Rho/RhoParser.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "TestConsoleHelper.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixture for testing Rho control structures
struct RhoControlTests : TestLangCommon {
    template <class T>
    void AssertDirectSimulation(const char *script, T expected,
                                bool verbose = false) {
        if (verbose) {
            KAI_LOG_INFO(std::string("Testing script: ") + script);
        }

        try {
            Console console;
            test::SetupConsoleTranslators(console);
            console.SetLanguage(Language::Rho);

            console.Execute(script, Structure::Program);

            // Get the result from the data stack after execution
            auto executor = console.GetExecutor();
            auto dataStack = executor->GetDataStack();

            if (dataStack->Empty()) {
                FAIL() << "No result on stack after script execution";
                return;
            }

            auto val = dataStack->Top();
            if (!val.IsType<T>()) {
                std::string expectedTypeName = typeid(T).name();
                std::string actualTypeName =
                    val.GetClass()
                        ? std::string(
                              val.GetClass()->GetName().ToString().c_str())
                        : "unknown";
                KAI_LOG_ERROR("Type mismatch. Expected: " + expectedTypeName +
                              ", Got: " + actualTypeName);
                FAIL() << "Type mismatch. Expected: " << expectedTypeName
                       << ", Got: " << actualTypeName;
                return;
            }

            T actual = ConstDeref<T>(val);
            if (verbose) {
                // Convert result to string for logging (handle different types)
                std::string resultStr;
                if constexpr (std::is_same_v<T, String>) {
                    resultStr = actual.StdString();
                } else if constexpr (std::is_arithmetic_v<T>) {
                    resultStr = std::to_string(actual);
                } else {
                    resultStr = "(complex type)";
                }
                KAI_LOG_INFO("Result: " + resultStr);
            }
            ASSERT_EQ(expected, actual)
                << "Result doesn't match expected value";
        } catch (const Exception::Base &e) {
            KAI_LOG_ERROR("Exception: " + std::string(e.ToString()));
            FAIL() << "Exception: " << e.ToString();
        } catch (const std::exception &e) {
            KAI_LOG_ERROR("std::exception: " + std::string(e.what()));
            FAIL() << "std::exception: " << e.what();
        } catch (...) {
            KAI_LOG_ERROR("Unknown exception");
            FAIL() << "Unknown exception";
        }
    }
};

// Simple if statements
TEST_F(RhoControlTests, BasicIfStatements) {
    // Using direct simulation to skip exec issues
    AssertDirectSimulation<int>(
        "result = 0\n"
        "if true\n"
        "    result = 42\n"
        "result",
        42);

    AssertDirectSimulation<int>(
        "result = 0\n"
        "if false\n"
        "    result = 42\n"
        "result",
        0);
}

// If-else statements
TEST_F(RhoControlTests, IfElseStatements) {
    AssertDirectSimulation<int>(
        "result = 0\n"
        "if true\n"
        "    result = 42\n"
        "else\n"
        "    result = 24\n"
        "result",
        42);

    AssertDirectSimulation<int>(
        "result = 0\n"
        "if false\n"
        "    result = 42\n"
        "else\n"
        "    result = 24\n"
        "result",
        24);
}

// Nested if statements
// These now work with proper test setup - see RhoControlTestsProperlyFixed.cpp
TEST_F(RhoControlTests, NestedIfStatements) {
    AssertDirectSimulation<int>(
        "result = 0\n"
        "if true\n"
        "    if true\n"
        "        result = 42\n"
        "    else\n"
        "        result = 24\n"
        "else\n"
        "    result = 10\n"
        "result",
        42);

    AssertDirectSimulation<int>(
        "result = 0\n"
        "if true\n"
        "    if false\n"
        "        result = 42\n"
        "    else\n"
        "        result = 24\n"
        "else\n"
        "    result = 10\n"
        "result",
        24);

    AssertDirectSimulation<int>(
        "result = 0\n"
        "if false\n"
        "    if true\n"
        "        result = 42\n"
        "    else\n"
        "        result = 24\n"
        "else\n"
        "    result = 10\n"
        "result",
        10);
}

// If with complex conditions
// These work without parentheses - see RhoControlTestsProperlyFixed.cpp
TEST_F(RhoControlTests, ComplexConditions) {
    AssertDirectSimulation<int>(
        "result = 0\n"
        "a = 5 > 3\n"
        "b = 10 < 20\n"
        "if a && b\n"
        "    result = 42\n"
        "else\n"
        "    result = 24\n"
        "result",
        42);

    AssertDirectSimulation<int>(
        "result = 0\n"
        "a = 5 < 3\n"
        "b = 10 > 20\n"
        "if a || b\n"
        "    result = 42\n"
        "else\n"
        "    result = 24\n"
        "result",
        24);

    AssertDirectSimulation<int>(
        "result = 0\n"
        "a = 5 > 3\n"
        "b = 10 < 20\n"
        "c = a == b\n"
        "if c\n"
        "    result = 42\n"
        "else\n"
        "    result = 24\n"
        "result",
        42);
}

// Basic for loops
TEST_F(RhoControlTests, BasicForLoops) {
    AssertDirectSimulation<int>(R"(
sum = 0
for i = 1; i <= 5; i = i + 1
    sum = sum + i
sum
)",
                                15);

    AssertDirectSimulation<int>(R"(
sum = 0
for i = 0; i < 10; i = i + 2
    sum = sum + i
sum
)",
                                20);
}

// Nested for loops
TEST_F(RhoControlTests, NestedForLoops) {
    AssertDirectSimulation<int>(R"(
sum = 0
for i = 1; i <= 3; i = i + 1
    for j = 1; j <= 3; j = j + 1
        sum = sum + (i * j)
sum
)",
                                36);
}

// For loops with complex conditions
TEST_F(RhoControlTests, ComplexForLoops) {
    AssertDirectSimulation<int>(R"(
sum = 0
for i = 0; i < 10 && sum < 20; i = i + 1
    sum = sum + i
sum
)",
                                21);

    AssertDirectSimulation<int>(R"(
sum = 0
for i = 0; i < 10 || sum < 5; i = i + 1
    sum = sum + i
sum
)",
                                45);
}

// Basic while loops
TEST_F(RhoControlTests, BasicWhileLoops) {
    AssertDirectSimulation<int>(R"(
sum = 0
i = 1
while i <= 5
    sum = sum + i
    i = i + 1
sum
)",
                                15);

    AssertDirectSimulation<int>(R"(
sum = 0
i = 0
while i < 10
    sum = sum + i
    i = i + 2
sum
)",
                                20);
}

// Nested while loops
TEST_F(RhoControlTests, NestedWhileLoops) {
    AssertDirectSimulation<int>(R"(
sum = 0
i = 1
while i <= 3
    j = 1
    while j <= 3
        sum = sum + (i * j)
        j = j + 1
    i = i + 1
sum
)",
                                36);
}

// While loops with complex conditions
TEST_F(RhoControlTests, ComplexWhileLoops) {
    AssertDirectSimulation<int>(R"(
sum = 0
i = 0
while i < 10 && sum < 20
    sum = sum + i
    i = i + 1
sum
)",
                                21);

    AssertDirectSimulation<int>(R"(
sum = 0
i = 0
while i < 10 || sum < 5
    sum = sum + i
    i = i + 1
sum
)",
                                45);
}

// Break statements in loops
TEST_F(RhoControlTests, BreakStatements) {
    AssertDirectSimulation<int>(
        "sum = 0\n"
        "for i = 1; i <= 10; i = i + 1\n"
        "    sum = sum + i\n"
        "    if sum > 10\n"
        "        break\n"
        "sum",
        15);

    AssertDirectSimulation<int>(
        "sum = 0\n"
        "i = 1\n"
        "while i <= 10\n"
        "    sum = sum + i\n"
        "    if sum > 10\n"
        "        break\n"
        "    i = i + 1\n"
        "sum",
        15);
}

// Continue statements in loops
TEST_F(RhoControlTests,
       DISABLED_ContinueStatements) {  // TODO: Debug continue implementation
    AssertDirectSimulation<int>(
        "sum = 0\n"
        "for i = 1; i <= 10; i = i + 1\n"
        "    if i % 2 == 0\n"
        "        continue\n"
        "    sum = sum + i\n"
        "sum",
        25);

    AssertDirectSimulation<int>(
        "sum = 0\n"
        "i = 0\n"
        "while i < 10\n"
        "    i = i + 1\n"
        "    if i % 2 == 0\n"
        "        continue\n"
        "    sum = sum + i\n"
        "sum",
        25);
}

// Do-while loops
TEST_F(RhoControlTests, DoWhileLoops) {
    AssertDirectSimulation<int>(
        "sum = 0\n"
        "i = 1\n"
        "do\n"
        "    sum = sum + i\n"
        "    i = i + 1\n"
        "while i <= 5\n"
        "sum",
        15);

    AssertDirectSimulation<int>(
        "sum = 0\n"
        "i = 1\n"
        "do\n"
        "    sum = sum + i\n"
        "    i = i + 1\n"
        "while false\n"
        "sum",
        1);
}

// Switch statements (using nested if-else since Rho doesn't support else-if)
TEST_F(RhoControlTests, SwitchLikeStatements) {
    AssertDirectSimulation<int>(
        "value = 2\n"
        "result = 0\n"
        "if value == 1\n"
        "    result = 10\n"
        "else\n"
        "    if value == 2\n"
        "        result = 20\n"
        "    else\n"
        "        if value == 3\n"
        "            result = 30\n"
        "        else\n"
        "            result = 0\n"
        "result",
        20);

    AssertDirectSimulation<int>(
        "value = 5\n"
        "result = 0\n"
        "if value == 1\n"
        "    result = 10\n"
        "else\n"
        "    if value == 2\n"
        "        result = 20\n"
        "    else\n"
        "        if value == 3\n"
        "            result = 30\n"
        "        else\n"
        "            result = 0\n"
        "result",
        0);
}

// Combining control structures
TEST_F(RhoControlTests, CombinedControlStructures) {
    AssertDirectSimulation<int>(
        "sum = 0\n"
        "for i = 1; i <= 10; i = i + 1\n"
        "    if i % 2 == 0\n"
        "        sum = sum + i\n"
        "    else\n"
        "        j = 0\n"
        "        while j < i\n"
        "            sum = sum + 1\n"
        "            j = j + 1\n"
        "sum",
        2 + 4 + 6 + 8 + 10 + 1 + 3 + 5 + 7 + 9);
}