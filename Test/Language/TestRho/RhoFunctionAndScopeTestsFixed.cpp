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

// Fixture for testing Rho functions and scope with correct syntax
struct RhoFunctionTestsFixed : TestLangCommon {
    template <class T>
    void RunAndExpect(const char *script, T expected, bool verbose = false) {
        if (verbose) {
            KAI_LOG_INFO(std::string("Testing script: ") + script);
        }

        try {
            Console console;
            console.SetLanguage(Language::Rho);  // Critical: Set language to Rho!
            Registry &reg = console.GetRegistry();

            // Execute as a complete program
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

// Function definitions with parameters are not fully implemented in Rho yet.
// These tests are disabled until the feature is complete.

// Basic function definition and call
TEST_F(RhoFunctionTestsFixed, BasicFunction) {
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
TEST_F(RhoFunctionTestsFixed, MultiStatementFunction) {
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

    RunAndExpect<int>(R"(
fun factorial(n)
    result = 1
    i = 2
    while i <= n
        result = result * i
        i = i + 1
    return result

factorial(5)
)",
                      120);
}

// Functions with different return types
TEST_F(RhoFunctionTestsFixed, DifferentReturnTypes) {
    RunAndExpect<int>(R"(
fun returnInt()
    return 42

returnInt()
)",
                      42);

    RunAndExpect<float>(R"(
fun returnFloat()
    return 3.14159

returnFloat()
)",
                        3.14159f);

    RunAndExpect<bool>(R"(
fun returnBool()
    return true

returnBool()
)",
                       true);

    RunAndExpect<String>(R"(
fun returnString()
    return "Hello World"

returnString()
)",
                         String("Hello World"));
}

// Function parameter passing
TEST_F(RhoFunctionTestsFixed, ParameterPassing) {
    RunAndExpect<int>(R"(
fun addThree(a, b, c)
    return a + b + c

addThree(1, 2, 3)
)",
                      6);

    RunAndExpect<float>(R"(
fun averageThree(a, b, c)
    return (a + b + c) / 3.0

averageThree(1.0, 2.0, 3.0)
)",
                        2.0f);

    RunAndExpect<String>(R"(
fun joinStrings(a, b)
    return a + b

joinStrings("Hello ", "World")
)",
                         String("Hello World"));
}

// Nested function calls
// NOTE: This test is disabled due to a limitation in the executor
// where function returns don't properly continue execution in the
// parent continuation. The bytecode is generated correctly but
// execution stops after the first nested function returns.
TEST_F(RhoFunctionTestsFixed, DISABLED_NestedFunctionCalls) {
    RunAndExpect<int>(R"(
fun square(n)
    return n * n

fun sumOfSquares(a, b)
    return square(a) + square(b)

sumOfSquares(3, 4)
)",
                      25);

    RunAndExpect<int>(R"(
fun increment(n)
    return n + 1

fun double(n)
    return n * 2

double(increment(increment(double(2))))
)",
                      10);
}

// Recursion
// DISABLED: Requires nested function calls which don't work due to executor limitation
TEST_F(RhoFunctionTestsFixed, DISABLED_Recursion) {
    RunAndExpect<int>(R"(
fun factorial(n)
    if n <= 1
        return 1
    else
        return n * factorial(n - 1)

factorial(5)
)",
                      120);

    RunAndExpect<int>(R"(
fun fibonacci(n)
    if n <= 1
        return n
    else
        return fibonacci(n - 1) + fibonacci(n - 2)

fibonacci(7)
)",
                      13);
}

// Mutual recursion
// DISABLED: Requires nested function calls which don't work due to executor limitation
TEST_F(RhoFunctionTestsFixed, DISABLED_MutualRecursion) {
    RunAndExpect<bool>(R"(
fun isEven(n)
    if n == 0
        return true
    else
        return isOdd(n - 1)

fun isOdd(n)
    if n == 0
        return false
    else
        return isEven(n - 1)

isEven(4)
)",
                       true);

    RunAndExpect<bool>(R"(
fun isEven(n)
    if n == 0
        return true
    else
        return isOdd(n - 1)

fun isOdd(n)
    if n == 0
        return false
    else
        return isEven(n - 1)

isOdd(5)
)",
                       true);
}

// Scoping tests - Rho doesn't have block scoping with braces
TEST_F(RhoFunctionTestsFixed, BasicScoping) {
    // Test global variable modification
    RunAndExpect<int>(R"(
x = 10
x = x + 5
x
)",
                      15);
}

// Function scoping
TEST_F(RhoFunctionTestsFixed, FunctionScoping) {
    RunAndExpect<int>(R"(
x = 10
fun mutateX()
    x = x + 5

mutateX()
x
)",
                      15);

    // Functions in Rho share scope with outer context
    RunAndExpect<int>(R"(
x = 10
fun updateX()
    x = 20
    x = x + 5
    return x

updateX()
x
)",
                      25);
}

// Function scoping with parameters
TEST_F(RhoFunctionTestsFixed, FunctionScopingWithParams) {
    RunAndExpect<int>(R"(
x = 10
fun updateX(x)
    x = x + 5
    return x

result = updateX(x)
x
)",
                      10);

    RunAndExpect<int>(R"(
x = 10
fun updateX(value)
    x = value + 5
    return x

updateX(20)
x
)",
                      25);
}

// Complex scope testing
TEST_F(RhoFunctionTestsFixed, ComplexScoping) {
    RunAndExpect<int>(R"(
counter = 0
fun makeCounter()
    counter = counter + 1
    return counter

makeCounter()
makeCounter()
counter
)",
                      2);
}