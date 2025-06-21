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

// Basic function definition and call
TEST_F(RhoFunctionTests, BasicFunction) {
    AssertDirectSimulation<int>(
        "fun add(a, b)\n"
        "    return a + b\n"
        "add(2, 3)",
        5);

    AssertDirectSimulation<int>(
        "fun multiply(a, b)\n"
        "    return a * b\n"
        "multiply(4, 5)",
        20);
}

// Functions with multiple statements
TEST_F(RhoFunctionTests, MultiStatementFunction) {
    AssertDirectSimulation<int>(
        "fun computeSum(n)\n"
        "    sum = 0\n"
        "    for i = 1; i <= n; i = i + 1\n"
        "        sum = sum + i\n"
        "    return sum\n"
        "computeSum(5)",
        15);

    AssertDirectSimulation<int>(
        "fun factorial(n)\n"
        "    result = 1\n"
        "    for i = 2; i <= n; i = i + 1\n"
        "        result = result * i\n"
        "    return result\n"
        "factorial(5)",
        120);
}

// Functions with different return types
TEST_F(RhoFunctionTests, DifferentReturnTypes) {
    AssertDirectSimulation<int>(
        "fun returnInt()\n"
        "    return 42\n"
        "returnInt()",
        42);

    AssertDirectSimulation<float>(
        "fun returnFloat()\n"
        "    return 3.14159\n"
        "returnFloat()",
        3.14159f);

    AssertDirectSimulation<bool>(
        "fun returnBool()\n"
        "    return true\n"
        "returnBool()",
        true);

    AssertDirectSimulation<String>(
        "fun returnString()\n"
        "    return \"Hello World\"\n"
        "returnString()",
        String("Hello World"));
}

// Function parameter passing
TEST_F(RhoFunctionTests, ParameterPassing) {
    AssertDirectSimulation<int>(
        "fun addThree(a, b, c)\n"
        "    return a + b + c\n"
        "addThree(1, 2, 3)",
        6);

    AssertDirectSimulation<float>(
        "fun averageThree(a, b, c)\n"
        "    return (a + b + c) / 3.0\n"
        "averageThree(1.0, 2.0, 3.0)",
        2.0f);

    AssertDirectSimulation<String>(
        "fun joinStrings(a, b)\n"
        "    return a + b\n"
        "joinStrings(\"Hello \", \"World\")",
        String("Hello World"));
}

// Nested function calls
TEST_F(RhoFunctionTests, NestedFunctionCalls) {
    AssertDirectSimulation<int>(
        "fun square(n)\n"
        "    return n * n\n"
        "fun sumOfSquares(a, b)\n"
        "    return square(a) + square(b)\n"
        "sumOfSquares(3, 4)",
        25);

    AssertDirectSimulation<int>(
        "fun increment(n)\n"
        "    return n + 1\n"
        "fun double(n)\n"
        "    return n * 2\n"
        "double(increment(increment(double(2))))",
        12);  // double(2)=4, increment(4)=5, increment(5)=6, double(6)=12
}

// Recursion
TEST_F(RhoFunctionTests, Recursion) {
    AssertDirectSimulation<int>(
        "fun factorial(n)\n"
        "    if n <= 1\n"
        "        return 1\n"
        "    else\n"
        "        return n * factorial(n - 1)\n"
        "factorial(5)",
        120);

    AssertDirectSimulation<int>(
        "fun fibonacci(n)\n"
        "    if n <= 1\n"
        "        return n\n"
        "    else\n"
        "        return fibonacci(n - 1) + fibonacci(n - 2)\n"
        "fibonacci(7)",
        13);
}

// Mutual recursion
TEST_F(RhoFunctionTests, MutualRecursion) {
    AssertDirectSimulation<bool>(
        "fun isEven(n)\n"
        "    if n == 0\n"
        "        return true\n"
        "    else\n"
        "        return isOdd(n - 1)\n"
        "fun isOdd(n)\n"
        "    if n == 0\n"
        "        return false\n"
        "    else\n"
        "        return isEven(n - 1)\n"
        "isEven(4)",
        true);

    AssertDirectSimulation<bool>(
        "fun isEven(n)\n"
        "    if n == 0\n"
        "        return true\n"
        "    else\n"
        "        return isOdd(n - 1)\n"
        "fun isOdd(n)\n"
        "    if n == 0\n"
        "        return false\n"
        "    else\n"
        "        return isEven(n - 1)\n"
        "isOdd(5)",
        true);
}

// Scoping tests
TEST_F(RhoFunctionTests, BasicScoping) {
    AssertDirectSimulation<int>(
        "x = 10\n"
        "if true\n"
        "    x = 20\n"
        "    x = x + 5\n"
        "x",
        25);  // In Rho, inner assignment affects outer x

    AssertDirectSimulation<int>(
        "x = 10\n"
        "x = x + 5\n"
        "x",
        15);
}

// Function scoping
TEST_F(RhoFunctionTests, FunctionScoping) {
    AssertDirectSimulation<int>(
        "x = 10\n"
        "fun mutateX()\n"
        "    x = x + 5\n"
        "mutateX()\n"
        "x",
        15);

    AssertDirectSimulation<int>(
        "x = 10\n"
        "fun shadowX()\n"
        "    x = 20\n"
        "    x = x + 5\n"
        "    return x\n"
        "shadowX()\n"
        "x",
        25);
}  // In Rho, functions share scope with outer context

// Nested scoping
TEST_F(RhoFunctionTests, NestedScoping) {
    AssertDirectSimulation<int>(
        "x = 10\n"
        "y = 20\n"
        "z = 30\n"
        "x = x + y + z\n"
        "x",
        60);

    AssertDirectSimulation<int>(
        "x = 10\n"
        "x = 20\n"
        "x = 30\n"
        "x = x + 5\n"
        "x = x + 2\n"
        "x",
        37);
}  // In Rho, all assignments affect the same x

// Function scoping with parameters
TEST_F(RhoFunctionTests, FunctionScopingWithParams) {
    AssertDirectSimulation<int>(
        "x = 10\n"
        "fun updateX(x)\n"
        "    x = x + 5\n"
        "    return x\n"
        "updateX(x)\n"
        "x",
        10);  // Parameter x shadows global x

    AssertDirectSimulation<int>(
        "x = 10\n"
        "fun updateX(value)\n"
        "    x = value + 5\n"
        "    return x\n"
        "updateX(20)\n"
        "x",
        25);
}

// Complex scope testing
TEST_F(RhoFunctionTests, ComplexScoping) {
    AssertDirectSimulation<int>(
        "x = 10\n"
        "fun outerFunc()\n"
        "    y = 20\n"
        "    fun innerFunc()\n"
        "        z = 30\n"
        "        x = x + 1\n"
        "        y = y + 1\n"
        "        return x + y + z\n"
        "    return innerFunc()\n"
        "outerFunc()",
        62);

    AssertDirectSimulation<int>(
        "counter = 0\n"
        "fun makeCounter()\n"
        "    localCounter = 0\n"
        "    fun increment()\n"
        "        localCounter = localCounter + 1\n"
        "        counter = counter + 1\n"
        "        return localCounter\n"
        "    return increment()\n"
        "makeCounter()\n"
        "makeCounter()\n"
        "counter",
        2);
}

// Debug test for nested function calls - CURRENTLY FAILING
TEST_F(RhoFunctionTests, DebugSimpleNestedCalls) {
    // First, define two simple functions
    console_.Execute(
        "fun add(a, b)\n"
        "    return a + b\n");

    console_.Execute(
        "fun double(n)\n"
        "    return n * 2\n");

    // Test direct calls first
    data_->Clear();
    console_.Execute("add(2, 3)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(data_->Top()), 5);

    data_->Clear();
    console_.Execute("double(4)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(data_->Top()), 8);

    // Now test nested call - CURRENTLY FAILING
    data_->Clear();
    console_.Execute("double(add(2, 3))");

    // Check if we have a result
    ASSERT_EQ(data_->Size(), 1) << "Stack size: " << data_->Size();
    EXPECT_EQ(ConstDeref<int>(data_->Top()), 10)
        << "Expected double(add(2,3)) = double(5) = 10";
}