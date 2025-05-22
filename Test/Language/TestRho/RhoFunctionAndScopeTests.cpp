#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "KAI/Core/BuiltinTypes/Stack.h"
#include "KAI/Core/Config/Base.h"
#include "KAI/Core/Debug.h"
#include "KAI/Core/Logger.h"
#include "KAI/Console/Console.h"
#include "KAI/Language/Rho/RhoParser.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Fixture for testing Rho functions and scope
struct RhoFunctionTests : TestLangCommon {
    template <class T>
    void AssertDirectSimulation(const char *script, T expected,
                                bool verbose = false) {
        if (verbose) {
            KAI_LOG_INFO(std::string("Testing script: ") + script);
        }

        try {
            Console console;
            Registry &reg = console.GetRegistry();

            console.Execute(script);

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
                std::string actualTypeName = val.GetClass() ? std::string(val.GetClass()->GetName().ToString().c_str()) : "unknown";
                KAI_LOG_ERROR("Type mismatch. Expected: " + expectedTypeName + ", Got: " + actualTypeName);
                FAIL() << "Type mismatch. Expected: " << expectedTypeName << ", Got: " << actualTypeName;
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

// Basic function definition and call
TEST_F(RhoFunctionTests, BasicFunction) {
    AssertDirectSimulation<int>(
        "function add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "add(2, 3);",
        5);

    AssertDirectSimulation<int>(
        "function multiply(int a, int b) {\n"
        "    return a * b;\n"
        "}\n"
        "multiply(4, 5);",
        20);
}

// Functions with multiple statements
TEST_F(RhoFunctionTests, MultiStatementFunction) {
    AssertDirectSimulation<int>(
        "function computeSum(int n) {\n"
        "    int sum = 0;\n"
        "    for (int i = 1; i <= n; i = i + 1) {\n"
        "        sum = sum + i;\n"
        "    }\n"
        "    return sum;\n"
        "}\n"
        "computeSum(5);",
        15);

    AssertDirectSimulation<int>(
        "function factorial(int n) {\n"
        "    int result = 1;\n"
        "    for (int i = 2; i <= n; i = i + 1) {\n"
        "        result = result * i;\n"
        "    }\n"
        "    return result;\n"
        "}\n"
        "factorial(5);",
        120);
}

// Functions with different return types
TEST_F(RhoFunctionTests, DifferentReturnTypes) {
    AssertDirectSimulation<int>(
        "function returnInt() {\n"
        "    return 42;\n"
        "}\n"
        "returnInt();",
        42);

    AssertDirectSimulation<float>(
        "function returnFloat() {\n"
        "    return 3.14159;\n"
        "}\n"
        "returnFloat();",
        3.14159f);

    AssertDirectSimulation<bool>(
        "function returnBool() {\n"
        "    return true;\n"
        "}\n"
        "returnBool();",
        true);

    AssertDirectSimulation<String>(
        "function returnString() {\n"
        "    return \"Hello World\";\n"
        "}\n"
        "returnString();",
        String("Hello World"));
}

// Function parameter passing
TEST_F(RhoFunctionTests, ParameterPassing) {
    AssertDirectSimulation<int>(
        "function addThree(int a, int b, int c) {\n"
        "    return a + b + c;\n"
        "}\n"
        "addThree(1, 2, 3);",
        6);

    AssertDirectSimulation<float>(
        "function averageThree(float a, float b, float c) {\n"
        "    return (a + b + c) / 3.0;\n"
        "}\n"
        "averageThree(1.0, 2.0, 3.0);",
        2.0f);

    AssertDirectSimulation<String>(
        "function joinStrings(string a, string b) {\n"
        "    return a + b;\n"
        "}\n"
        "joinStrings(\"Hello \", \"World\");",
        String("Hello World"));
}

// Nested function calls
TEST_F(RhoFunctionTests, NestedFunctionCalls) {
    AssertDirectSimulation<int>(
        "function square(int n) {\n"
        "    return n * n;\n"
        "}\n"
        "function sumOfSquares(int a, int b) {\n"
        "    return square(a) + square(b);\n"
        "}\n"
        "sumOfSquares(3, 4);",
        25);

    AssertDirectSimulation<int>(
        "function increment(int n) {\n"
        "    return n + 1;\n"
        "}\n"
        "function double(int n) {\n"
        "    return n * 2;\n"
        "}\n"
        "double(increment(increment(double(2))));",
        10);
}

// Recursion
TEST_F(RhoFunctionTests, Recursion) {
    AssertDirectSimulation<int>(
        "function factorial(int n) {\n"
        "    if (n <= 1) {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return n * factorial(n - 1);\n"
        "    }\n"
        "}\n"
        "factorial(5);",
        120);

    AssertDirectSimulation<int>(
        "function fibonacci(int n) {\n"
        "    if (n <= 1) {\n"
        "        return n;\n"
        "    } else {\n"
        "        return fibonacci(n - 1) + fibonacci(n - 2);\n"
        "    }\n"
        "}\n"
        "fibonacci(7);",
        13);
}

// Mutual recursion
TEST_F(RhoFunctionTests, MutualRecursion) {
    AssertDirectSimulation<bool>(
        "function isEven(int n) {\n"
        "    if (n == 0) {\n"
        "        return true;\n"
        "    } else {\n"
        "        return isOdd(n - 1);\n"
        "    }\n"
        "}\n"
        "function isOdd(int n) {\n"
        "    if (n == 0) {\n"
        "        return false;\n"
        "    } else {\n"
        "        return isEven(n - 1);\n"
        "    }\n"
        "}\n"
        "isEven(4);",
        true);

    AssertDirectSimulation<bool>(
        "function isEven(int n) {\n"
        "    if (n == 0) {\n"
        "        return true;\n"
        "    } else {\n"
        "        return isOdd(n - 1);\n"
        "    }\n"
        "}\n"
        "function isOdd(int n) {\n"
        "    if (n == 0) {\n"
        "        return false;\n"
        "    } else {\n"
        "        return isEven(n - 1);\n"
        "    }\n"
        "}\n"
        "isOdd(5);",
        true);
}

// Scoping tests
TEST_F(RhoFunctionTests, BasicScoping) {
    AssertDirectSimulation<int>(
        "int x = 10;\n"
        "{\n"
        "    int x = 20;\n"
        "    x = x + 5;\n"
        "}\n"
        "x;",
        10);

    AssertDirectSimulation<int>(
        "int x = 10;\n"
        "{\n"
        "    x = x + 5;\n"
        "}\n"
        "x;",
        15);
}

// Function scoping
TEST_F(RhoFunctionTests, FunctionScoping) {
    AssertDirectSimulation<int>(
        "int x = 10;\n"
        "function mutateX() {\n"
        "    x = x + 5;\n"
        "}\n"
        "mutateX();\n"
        "x;",
        15);

    AssertDirectSimulation<int>(
        "int x = 10;\n"
        "function shadowX() {\n"
        "    int x = 20;\n"
        "    x = x + 5;\n"
        "    return x;\n"
        "}\n"
        "shadowX();\n"
        "x;",
        10);
}

// Nested scoping
TEST_F(RhoFunctionTests, NestedScoping) {
    AssertDirectSimulation<int>(
        "int x = 10;\n"
        "{\n"
        "    int y = 20;\n"
        "    {\n"
        "        int z = 30;\n"
        "        x = x + y + z;\n"
        "    }\n"
        "}\n"
        "x;",
        60);

    AssertDirectSimulation<int>(
        "int x = 10;\n"
        "{\n"
        "    int x = 20;\n"
        "    {\n"
        "        int x = 30;\n"
        "        x = x + 5;\n"
        "    }\n"
        "    x = x + 2;\n"
        "}\n"
        "x;",
        10);
}

// Function scoping with parameters
TEST_F(RhoFunctionTests, FunctionScopingWithParams) {
    AssertDirectSimulation<int>(
        "int x = 10;\n"
        "function updateX(int x) {\n"
        "    x = x + 5;\n"
        "    return x;\n"
        "}\n"
        "updateX(x);\n"
        "x;",
        10);

    AssertDirectSimulation<int>(
        "int x = 10;\n"
        "function updateX(int value) {\n"
        "    x = value + 5;\n"
        "    return x;\n"
        "}\n"
        "updateX(20);\n"
        "x;",
        25);
}

// Complex scope testing
TEST_F(RhoFunctionTests, ComplexScoping) {
    AssertDirectSimulation<int>(
        "int x = 10;\n"
        "function outerFunc() {\n"
        "    int y = 20;\n"
        "    function innerFunc() {\n"
        "        int z = 30;\n"
        "        x = x + 1;\n"
        "        y = y + 1;\n"
        "        return x + y + z;\n"
        "    }\n"
        "    return innerFunc();\n"
        "}\n"
        "outerFunc();",
        62);

    AssertDirectSimulation<int>(
        "int counter = 0;\n"
        "function makeCounter() {\n"
        "    int localCounter = 0;\n"
        "    function increment() {\n"
        "        localCounter = localCounter + 1;\n"
        "        counter = counter + 1;\n"
        "        return localCounter;\n"
        "    }\n"
        "    return increment();\n"
        "}\n"
        "makeCounter();\n"
        "makeCounter();\n"
        "counter;",
        2);
}