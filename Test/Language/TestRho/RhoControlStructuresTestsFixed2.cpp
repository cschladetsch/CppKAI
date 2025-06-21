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

// Fixture for testing Rho control structures with correct program parsing
struct RhoControlTestsFixed2 : TestLangCommon {
    template <class T>
    void AssertDirectSimulation(const char *script, T expected,
                                bool verbose = false) {
        if (verbose) {
            KAI_LOG_INFO(std::string("Testing script: ") + script);
        }

        try {
            // Use the console from TestLangCommon which has translators set up
            console_.SetLanguage(Language::Rho);

            // Execute as a PROGRAM, not a single statement
            console_.Execute(script, Structure::Program);

            // Get the result from the data stack after execution
            auto executor = console_.GetExecutor();
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

// Simple if statements - using Rho indentation syntax
TEST_F(RhoControlTestsFixed2, BasicIfStatements) {
    // Python-style indentation
    AssertDirectSimulation<int>(
        "result = 0\n"
        "if true\n"
        "    result = 42\n"
        "result",
        42, true);

    AssertDirectSimulation<int>(
        "result = 0\n"
        "if false\n"
        "    result = 42\n"
        "result",
        0);
}

// If-else statements with indentation
TEST_F(RhoControlTestsFixed2, IfElseStatements) {
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

// While loops with Rho syntax
TEST_F(RhoControlTestsFixed2, BasicWhileLoops) {
    AssertDirectSimulation<int>(
        "sum = 0\n"
        "i = 1\n"
        "while i <= 5\n"
        "    sum = sum + i\n"
        "    i = i + 1\n"
        "sum",
        15);
}

// For loops with Rho syntax (semicolons between parts)
TEST_F(RhoControlTestsFixed2, BasicForLoops) {
    AssertDirectSimulation<int>(
        "sum = 0\n"
        "for i = 1; i <= 5; i = i + 1\n"
        "    sum = sum + i\n"
        "sum",
        15);
}

// Do-while loops with Rho syntax
TEST_F(RhoControlTestsFixed2, DoWhileLoops) {
    AssertDirectSimulation<int>(
        "sum = 0\n"
        "i = 1\n"
        "do\n"
        "    sum = sum + i\n"
        "    i = i + 1\n"
        "while i <= 5\n"
        "sum",
        15);
}

// Nested if statements with proper indentation
TEST_F(RhoControlTestsFixed2, NestedIfStatements) {
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
}

// Simple nested if without problematic outer else
TEST_F(RhoControlTestsFixed2, SimpleNestedIf) {
    AssertDirectSimulation<int>(
        "result = 0\n"
        "if true\n"
        "    if false\n"
        "        result = 24\n"
        "    else\n"
        "        result = 42\n"
        "result",
        42);
}

// Complex expressions in conditions
TEST_F(RhoControlTestsFixed2, ComplexConditions) {
    AssertDirectSimulation<int>(
        "result = 0\n"
        "if 5 > 3 && 10 < 20\n"
        "    result = 42\n"
        "else\n"
        "    result = 24\n"
        "result",
        42);
}