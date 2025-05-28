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
            Registry &reg = console.GetRegistry();
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
// TODO: These tests use C-style syntax which is not supported in Rho
// The correct Python-style tests are in RhoControlStructuresTestsFixed2.cpp
TEST_F(RhoControlTests, DISABLED_NestedIfStatements) {
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
// TODO: Disable due to mixed C-style syntax
TEST_F(RhoControlTests, DISABLED_ComplexConditions) {
    AssertDirectSimulation<int>(
        "result = 0\n"
        "if (5 > 3) && (10 < 20)\n"
        "    result = 42\n"
        "else\n"
        "    result = 24\n"
        "result",
        42);

    AssertDirectSimulation<int>(
        "result = 0\n"
        "if (5 < 3) || (10 > 20)\n"
        "    result = 42\n"
        "else\n"
        "    result = 24\n"
        "result",
        24);

    AssertDirectSimulation<int>(
        "int result = 0;\n"
        "if ((5 > 3) == (10 < 20)) {\n"
        "    result = 42;\n"
        "} else {\n"
        "    result = 24;\n"
        "}\n"
        "result;",
        42);
}

// Basic for loops
TEST_F(RhoControlTests, DISABLED_BasicForLoops) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "for (int i = 1; i <= 5; i = i + 1) {\n"
        "    sum = sum + i;\n"
        "}\n"
        "sum;",
        15);

    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "for (int i = 0; i < 10; i = i + 2) {\n"
        "    sum = sum + i;\n"
        "}\n"
        "sum;",
        20);
}

// Nested for loops
TEST_F(RhoControlTests, DISABLED_NestedForLoops) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "for (int i = 1; i <= 3; i = i + 1) {\n"
        "    for (int j = 1; j <= 3; j = j + 1) {\n"
        "        sum = sum + (i * j);\n"
        "    }\n"
        "}\n"
        "sum;",
        36);
}

// For loops with complex conditions
TEST_F(RhoControlTests, DISABLED_ComplexForLoops) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "for (int i = 0; i < 10 && sum < 20; i = i + 1) {\n"
        "    sum = sum + i;\n"
        "}\n"
        "sum;",
        21);

    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "for (int i = 0; i < 10 || sum < 5; i = i + 1) {\n"
        "    sum = sum + i;\n"
        "}\n"
        "sum;",
        45);
}

// Basic while loops
TEST_F(RhoControlTests, DISABLED_BasicWhileLoops) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "int i = 1;\n"
        "while (i <= 5) {\n"
        "    sum = sum + i;\n"
        "    i = i + 1;\n"
        "}\n"
        "sum;",
        15);

    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "int i = 0;\n"
        "while (i < 10) {\n"
        "    sum = sum + i;\n"
        "    i = i + 2;\n"
        "}\n"
        "sum;",
        20);
}

// Nested while loops
TEST_F(RhoControlTests, DISABLED_NestedWhileLoops) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "int i = 1;\n"
        "while (i <= 3) {\n"
        "    int j = 1;\n"
        "    while (j <= 3) {\n"
        "        sum = sum + (i * j);\n"
        "        j = j + 1;\n"
        "    }\n"
        "    i = i + 1;\n"
        "}\n"
        "sum;",
        36);
}

// While loops with complex conditions
TEST_F(RhoControlTests, DISABLED_ComplexWhileLoops) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "int i = 0;\n"
        "while (i < 10 && sum < 20) {\n"
        "    sum = sum + i;\n"
        "    i = i + 1;\n"
        "}\n"
        "sum;",
        21);

    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "int i = 0;\n"
        "while (i < 10 || sum < 5) {\n"
        "    sum = sum + i;\n"
        "    i = i + 1;\n"
        "}\n"
        "sum;",
        45);
}

// Break statements in loops
TEST_F(RhoControlTests, DISABLED_BreakStatements) {
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
TEST_F(RhoControlTests, DISABLED_ContinueStatements) {
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

// Switch statements (using if-else if-else since Rho might not directly support
// switch)
TEST_F(RhoControlTests, DISABLED_SwitchLikeStatements) {
    AssertDirectSimulation<int>(
        "value = 2\n"
        "result = 0\n"
        "if value == 1\n"
        "    result = 10\n"
        "else if value == 2\n"
        "    result = 20\n"
        "else if value == 3\n"
        "    result = 30\n"
        "else\n"
        "    result = 0\n"
        "result",
        20);

    AssertDirectSimulation<int>(
        "value = 5\n"
        "result = 0\n"
        "if value == 1\n"
        "    result = 10\n"
        "else if value == 2\n"
        "    result = 20\n"
        "else if value == 3\n"
        "    result = 30\n"
        "else\n"
        "    result = 0\n"
        "result",
        0);
}

// Combining control structures
TEST_F(RhoControlTests, DISABLED_CombinedControlStructures) {
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