#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
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
            Registry reg;
            Console console(reg);
            console.SetScope(reg.GetGlobalScope());

            auto result = console.Execute(script);

            if (result.Failed) {
                KAI_LOG_ERROR("Execution failed: " + result.Error);
                FAIL() << "Error executing script: " << result.Error;
                return;
            }

            // We'll use a separate mechanism to extract the result
            // to handle cases where the control structure doesn't leave
            // a value on the stack
            auto val = result.Value;
            if (val.GetType() != Type::Traits<T>::TypeNumber) {
                KAI_LOG_ERROR("Type mismatch. Expected: " +
                              std::to_string(Type::Traits<T>::TypeNumber) +
                              ", Got: " + std::to_string(val.GetType()));
                FAIL() << "Type mismatch. Expected: "
                       << Type::Traits<T>::TypeNumber
                       << ", Got: " << val.GetType();
                return;
            }

            T actual = kai_cast<T>(val);
            if (verbose) {
                KAI_LOG_INFO("Result: " + std::to_string(actual));
            }
            ASSERT_EQ(expected, actual)
                << "Result doesn't match expected value";
        } catch (const Exception &e) {
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
        "int result = 0;\n"
        "if (true) {\n"
        "    result = 42;\n"
        "}\n"
        "result;",
        42);

    AssertDirectSimulation<int>(
        "int result = 0;\n"
        "if (false) {\n"
        "    result = 42;\n"
        "}\n"
        "result;",
        0);
}

// If-else statements
TEST_F(RhoControlTests, IfElseStatements) {
    AssertDirectSimulation<int>(
        "int result = 0;\n"
        "if (true) {\n"
        "    result = 42;\n"
        "} else {\n"
        "    result = 24;\n"
        "}\n"
        "result;",
        42);

    AssertDirectSimulation<int>(
        "int result = 0;\n"
        "if (false) {\n"
        "    result = 42;\n"
        "} else {\n"
        "    result = 24;\n"
        "}\n"
        "result;",
        24);
}

// Nested if statements
TEST_F(RhoControlTests, NestedIfStatements) {
    AssertDirectSimulation<int>(
        "int result = 0;\n"
        "if (true) {\n"
        "    if (true) {\n"
        "        result = 42;\n"
        "    } else {\n"
        "        result = 24;\n"
        "    }\n"
        "} else {\n"
        "    result = 10;\n"
        "}\n"
        "result;",
        42);

    AssertDirectSimulation<int>(
        "int result = 0;\n"
        "if (true) {\n"
        "    if (false) {\n"
        "        result = 42;\n"
        "    } else {\n"
        "        result = 24;\n"
        "    }\n"
        "} else {\n"
        "    result = 10;\n"
        "}\n"
        "result;",
        24);

    AssertDirectSimulation<int>(
        "int result = 0;\n"
        "if (false) {\n"
        "    if (true) {\n"
        "        result = 42;\n"
        "    } else {\n"
        "        result = 24;\n"
        "    }\n"
        "} else {\n"
        "    result = 10;\n"
        "}\n"
        "result;",
        10);
}

// If with complex conditions
TEST_F(RhoControlTests, ComplexConditions) {
    AssertDirectSimulation<int>(
        "int result = 0;\n"
        "if (5 > 3 && 10 < 20) {\n"
        "    result = 42;\n"
        "} else {\n"
        "    result = 24;\n"
        "}\n"
        "result;",
        42);

    AssertDirectSimulation<int>(
        "int result = 0;\n"
        "if (5 < 3 || 10 > 20) {\n"
        "    result = 42;\n"
        "} else {\n"
        "    result = 24;\n"
        "}\n"
        "result;",
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
TEST_F(RhoControlTests, BasicForLoops) {
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
TEST_F(RhoControlTests, NestedForLoops) {
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
TEST_F(RhoControlTests, ComplexForLoops) {
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
TEST_F(RhoControlTests, BasicWhileLoops) {
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
TEST_F(RhoControlTests, NestedWhileLoops) {
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
TEST_F(RhoControlTests, ComplexWhileLoops) {
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
TEST_F(RhoControlTests, BreakStatements) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "for (int i = 1; i <= 10; i = i + 1) {\n"
        "    sum = sum + i;\n"
        "    if (sum > 10) {\n"
        "        break;\n"
        "    }\n"
        "}\n"
        "sum;",
        15);

    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "int i = 1;\n"
        "while (i <= 10) {\n"
        "    sum = sum + i;\n"
        "    if (sum > 10) {\n"
        "        break;\n"
        "    }\n"
        "    i = i + 1;\n"
        "}\n"
        "sum;",
        15);
}

// Continue statements in loops
TEST_F(RhoControlTests, ContinueStatements) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "for (int i = 1; i <= 10; i = i + 1) {\n"
        "    if (i % 2 == 0) {\n"
        "        continue;\n"
        "    }\n"
        "    sum = sum + i;\n"
        "}\n"
        "sum;",
        25);

    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "int i = 0;\n"
        "while (i < 10) {\n"
        "    i = i + 1;\n"
        "    if (i % 2 == 0) {\n"
        "        continue;\n"
        "    }\n"
        "    sum = sum + i;\n"
        "}\n"
        "sum;",
        25);
}

// Do-while loops
TEST_F(RhoControlTests, DoWhileLoops) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "int i = 1;\n"
        "do {\n"
        "    sum = sum + i;\n"
        "    i = i + 1;\n"
        "} while (i <= 5);\n"
        "sum;",
        15);

    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "int i = 1;\n"
        "do {\n"
        "    sum = sum + i;\n"
        "    i = i + 1;\n"
        "} while (false);\n"
        "sum;",
        1);
}

// Switch statements (using if-else if-else since Rho might not directly support
// switch)
TEST_F(RhoControlTests, SwitchLikeStatements) {
    AssertDirectSimulation<int>(
        "int value = 2;\n"
        "int result = 0;\n"
        "if (value == 1) {\n"
        "    result = 10;\n"
        "} else if (value == 2) {\n"
        "    result = 20;\n"
        "} else if (value == 3) {\n"
        "    result = 30;\n"
        "} else {\n"
        "    result = 0;\n"
        "}\n"
        "result;",
        20);

    AssertDirectSimulation<int>(
        "int value = 5;\n"
        "int result = 0;\n"
        "if (value == 1) {\n"
        "    result = 10;\n"
        "} else if (value == 2) {\n"
        "    result = 20;\n"
        "} else if (value == 3) {\n"
        "    result = 30;\n"
        "} else {\n"
        "    result = 0;\n"
        "}\n"
        "result;",
        0);
}

// Combining control structures
TEST_F(RhoControlTests, CombinedControlStructures) {
    AssertDirectSimulation<int>(
        "int sum = 0;\n"
        "for (int i = 1; i <= 10; i = i + 1) {\n"
        "    if (i % 2 == 0) {\n"
        "        sum = sum + i;\n"
        "    } else {\n"
        "        int j = 0;\n"
        "        while (j < i) {\n"
        "            sum = sum + 1;\n"
        "            j = j + 1;\n"
        "        }\n"
        "    }\n"
        "}\n"
        "sum;",
        2 + 4 + 6 + 8 + 10 + 1 + 3 + 5 + 7 + 9);
}