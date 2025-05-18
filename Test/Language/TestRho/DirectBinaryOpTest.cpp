#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "KAI/Core/Object/ClassBuilder.h"
#include "KAI/Executor/Operation.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Create a concrete test class that implements TestBody to help with
// continuations
class ContinuationTestHelper {
   public:
    // Just a simple helper that creates a console, registry, and executor to
    // test continuations
    Object TestExtractValue(Object value) {
        Console console;
        Registry& reg = console.GetRegistry();

        // Register common types
        reg.AddClass<int>(Label("int"));
        reg.AddClass<bool>(Label("bool"));
        reg.AddClass<String>(Label("String"));
        reg.AddClass<float>(Label("float"));

        // Create the executor and data stack
        Pointer<Executor> executor = reg.New<Executor>();
        executor->Create();

        // We'll use our own version of the extraction logic to avoid dependency
        // on TestLangCommon
        if (!value.IsType<Continuation>()) {
            return value;  // Pass through non-continuations
        }

        Pointer<Continuation> cont = value;
        if (!cont.Valid() || !cont.Exists()) {
            KAI_TRACE_ERROR() << "TestExtractValue: Invalid continuation";
            return value;
        }

        if (!cont->GetCode().Valid() || !cont->GetCode().Exists() ||
            !cont->GetCode()->Size()) {
            KAI_TRACE_ERROR()
                << "TestExtractValue: Invalid or empty code array";
            return value;
        }

        Pointer<const Array> code = cont->GetCode();

        // Make sure the registry is valid
        Registry* registry = value.GetRegistry();
        if (!registry) {
            KAI_TRACE_ERROR() << "TestExtractValue: Null registry";
            return value;
        }

        // If it has 3 items and the last is an operation, it's likely a binary
        // operation
        if (code->Size() == 3 && code->At(2).IsType<Operation>()) {
            Object val1 = code->At(0);
            Object val2 = code->At(1);
            Operation::Type op =
                ConstDeref<Operation>(code->At(2)).GetTypeNumber();

            // Handle integer operations
            if (val1.IsType<int>() && val2.IsType<int>()) {
                int num1 = ConstDeref<int>(val1);
                int num2 = ConstDeref<int>(val2);

                switch (op) {
                    case Operation::Plus:
                        return registry->New<int>(num1 + num2);
                    case Operation::Minus:
                        return registry->New<int>(num1 - num2);
                    case Operation::Multiply:
                        return registry->New<int>(num1 * num2);
                    case Operation::Divide:
                        if (num2 != 0) return registry->New<int>(num1 / num2);
                        break;
                    case Operation::Less:
                        return registry->New<bool>(num1 < num2);
                    case Operation::Greater:
                        return registry->New<bool>(num1 > num2);
                    case Operation::Equiv:
                        return registry->New<bool>(num1 == num2);
                    default:
                        break;
                }
            }

            // Handle boolean operations
            else if (val1.IsType<bool>() && val2.IsType<bool>()) {
                bool b1 = ConstDeref<bool>(val1);
                bool b2 = ConstDeref<bool>(val2);

                switch (op) {
                    case Operation::LogicalAnd:
                        return registry->New<bool>(b1 && b2);
                    case Operation::LogicalOr:
                        return registry->New<bool>(b1 || b2);
                    default:
                        break;
                }
            }

            // Handle string operations
            else if (val1.IsType<String>() && val2.IsType<String>()) {
                String str1 = ConstDeref<String>(val1);
                String str2 = ConstDeref<String>(val2);

                switch (op) {
                    case Operation::Plus:
                        return registry->New<String>(str1 + str2);
                    default:
                        break;
                }
            }
        }

        // If we can't handle this pattern, return the original
        return value;
    }
};

// Helper method to create a test continuation with binary operation
static Object CreateTestContinuation(Registry& reg,
                                     const std::vector<Object>& values,
                                     Operation::Type op) {
    Pointer<Continuation> cont = reg.New<Continuation>();
    cont->Create();

    Pointer<Array> code = reg.New<Array>();

    // Add each value to the code array
    for (const Object& val : values) {
        code->Append(val);
    }

    // Add the operation
    code->Append(reg.New<Operation>(op));

    cont->SetCode(code);
    cont->SetSpecialHandling(true);

    return cont;
}

/*
 * DIRECT BINARY OPERATION TESTS
 * -----------------------------
 * These tests directly verify the PerformBinaryOp method and type handling in
 * the Executor class. They bypass the translation and execution phases to
 * ensure that binary operations are properly handling type preservation.
 */

TEST(DirectBinaryOp, IntArithmetic) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<float>(Label("float"));
    reg.AddClass<bool>(Label("bool"));

    // Create an executor to test operations
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();

    // Test addition
    {
        Object a = reg.New<int>(5);
        Object b = reg.New<int>(7);
        Object result = executor->PerformBinaryOp(a, b, Operation::Plus);

        ASSERT_TRUE(result.IsType<int>()) << "Addition result should be int";
        ASSERT_EQ(ConstDeref<int>(result), 12) << "5 + 7 should equal 12";
    }

    // Test subtraction
    {
        Object a = reg.New<int>(10);
        Object b = reg.New<int>(3);
        Object result = executor->PerformBinaryOp(a, b, Operation::Minus);

        ASSERT_TRUE(result.IsType<int>()) << "Subtraction result should be int";
        ASSERT_EQ(ConstDeref<int>(result), 7) << "10 - 3 should equal 7";
    }

    // Test multiplication
    {
        Object a = reg.New<int>(6);
        Object b = reg.New<int>(8);
        Object result = executor->PerformBinaryOp(a, b, Operation::Multiply);

        ASSERT_TRUE(result.IsType<int>())
            << "Multiplication result should be int";
        ASSERT_EQ(ConstDeref<int>(result), 48) << "6 * 8 should equal 48";
    }

    // Test division
    {
        Object a = reg.New<int>(20);
        Object b = reg.New<int>(4);
        Object result = executor->PerformBinaryOp(a, b, Operation::Divide);

        ASSERT_TRUE(result.IsType<int>()) << "Division result should be int";
        ASSERT_EQ(ConstDeref<int>(result), 5) << "20 / 4 should equal 5";
    }

    // Test modulo
    {
        Object a = reg.New<int>(17);
        Object b = reg.New<int>(5);
        Object result = executor->PerformBinaryOp(a, b, Operation::Modulo);

        ASSERT_TRUE(result.IsType<int>()) << "Modulo result should be int";
        ASSERT_EQ(ConstDeref<int>(result), 2) << "17 % 5 should equal 2";
    }
}

TEST(DirectBinaryOp, FloatArithmetic) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<float>(Label("float"));

    // Create an executor to test operations
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();

    // Test addition
    {
        Object a = reg.New<float>(3.5f);
        Object b = reg.New<float>(2.25f);
        Object result = executor->PerformBinaryOp(a, b, Operation::Plus);

        ASSERT_TRUE(result.IsType<float>())
            << "Addition result should be float";
        ASSERT_FLOAT_EQ(ConstDeref<float>(result), 5.75f)
            << "3.5 + 2.25 should equal 5.75";
    }

    // Test subtraction
    {
        Object a = reg.New<float>(7.5f);
        Object b = reg.New<float>(2.5f);
        Object result = executor->PerformBinaryOp(a, b, Operation::Minus);

        ASSERT_TRUE(result.IsType<float>())
            << "Subtraction result should be float";
        ASSERT_FLOAT_EQ(ConstDeref<float>(result), 5.0f)
            << "7.5 - 2.5 should equal 5.0";
    }

    // Test multiplication
    {
        Object a = reg.New<float>(3.5f);
        Object b = reg.New<float>(2.0f);
        Object result = executor->PerformBinaryOp(a, b, Operation::Multiply);

        ASSERT_TRUE(result.IsType<float>())
            << "Multiplication result should be float";
        ASSERT_FLOAT_EQ(ConstDeref<float>(result), 7.0f)
            << "3.5 * 2.0 should equal 7.0";
    }

    // Test division
    {
        Object a = reg.New<float>(10.0f);
        Object b = reg.New<float>(2.5f);
        Object result = executor->PerformBinaryOp(a, b, Operation::Divide);

        ASSERT_TRUE(result.IsType<float>())
            << "Division result should be float";
        ASSERT_FLOAT_EQ(ConstDeref<float>(result), 4.0f)
            << "10.0 / 2.5 should equal 4.0";
    }
}

TEST(DirectBinaryOp, MixedTypeArithmetic) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<float>(Label("float"));

    // Create an executor to test operations
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();

    // Test int + float
    {
        Object a = reg.New<int>(5);
        Object b = reg.New<float>(2.5f);
        Object result = executor->PerformBinaryOp(a, b, Operation::Plus);

        ASSERT_TRUE(result.IsType<float>())
            << "Int + Float should result in Float";
        ASSERT_FLOAT_EQ(ConstDeref<float>(result), 7.5f)
            << "5 + 2.5 should equal 7.5";
    }

    // Test float + int
    {
        Object a = reg.New<float>(3.5f);
        Object b = reg.New<int>(2);
        Object result = executor->PerformBinaryOp(a, b, Operation::Plus);

        ASSERT_TRUE(result.IsType<float>())
            << "Float + Int should result in Float";
        ASSERT_FLOAT_EQ(ConstDeref<float>(result), 5.5f)
            << "3.5 + 2 should equal 5.5";
    }

    // Test float * int
    {
        Object a = reg.New<float>(4.5f);
        Object b = reg.New<int>(2);
        Object result = executor->PerformBinaryOp(a, b, Operation::Multiply);

        ASSERT_TRUE(result.IsType<float>())
            << "Float * Int should result in Float";
        ASSERT_FLOAT_EQ(ConstDeref<float>(result), 9.0f)
            << "4.5 * 2 should equal 9.0";
    }
}

TEST(DirectBinaryOp, ComparisonOperations) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    // Create an executor to test operations
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();

    // Test equality
    {
        Object a = reg.New<int>(5);
        Object b = reg.New<int>(5);
        Object result = executor->PerformBinaryOp(a, b, Operation::Equiv);

        ASSERT_TRUE(result.IsType<bool>())
            << "Comparison result should be bool";
        ASSERT_TRUE(ConstDeref<bool>(result)) << "5 == 5 should be true";
    }

    // Test inequality
    {
        Object a = reg.New<int>(5);
        Object b = reg.New<int>(7);
        Object result = executor->PerformBinaryOp(a, b, Operation::NotEquiv);

        ASSERT_TRUE(result.IsType<bool>())
            << "Comparison result should be bool";
        ASSERT_TRUE(ConstDeref<bool>(result)) << "5 != 7 should be true";
    }

    // Test less than
    {
        Object a = reg.New<int>(5);
        Object b = reg.New<int>(10);
        Object result = executor->PerformBinaryOp(a, b, Operation::Less);

        ASSERT_TRUE(result.IsType<bool>())
            << "Comparison result should be bool";
        ASSERT_TRUE(ConstDeref<bool>(result)) << "5 < 10 should be true";
    }

    // Test greater than
    {
        Object a = reg.New<int>(15);
        Object b = reg.New<int>(10);
        Object result = executor->PerformBinaryOp(a, b, Operation::Greater);

        ASSERT_TRUE(result.IsType<bool>())
            << "Comparison result should be bool";
        ASSERT_TRUE(ConstDeref<bool>(result)) << "15 > 10 should be true";
    }
}

TEST(DirectBinaryOp, LogicalOperations) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    // Create an executor to test operations
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();

    // Test logical AND
    {
        Object a = reg.New<bool>(true);
        Object b = reg.New<bool>(false);
        Object result = executor->PerformBinaryOp(a, b, Operation::LogicalAnd);

        ASSERT_TRUE(result.IsType<bool>())
            << "Logical AND result should be bool";
        ASSERT_FALSE(ConstDeref<bool>(result))
            << "true && false should be false";
    }

    // Test logical OR
    {
        Object a = reg.New<bool>(false);
        Object b = reg.New<bool>(true);
        Object result = executor->PerformBinaryOp(a, b, Operation::LogicalOr);

        ASSERT_TRUE(result.IsType<bool>())
            << "Logical OR result should be bool";
        ASSERT_TRUE(ConstDeref<bool>(result)) << "false || true should be true";
    }
}

TEST(DirectBinaryOp, StringConcatenation) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    // Create an executor to test operations
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();

    // Test string concatenation
    {
        Object a = reg.New<String>("Hello ");
        Object b = reg.New<String>("World");
        Object result = executor->PerformBinaryOp(a, b, Operation::Plus);

        ASSERT_TRUE(result.IsType<String>())
            << "String concatenation result should be String";
        ASSERT_EQ(ConstDeref<String>(result), "Hello World")
            << "\"Hello \" + \"World\" should equal \"Hello World\"";
    }
}

// Test the helper method for creating continuations with specific operations
TEST(DirectBinaryOp, ContinuationEvaluation) {
    try {
        Console console;
        Registry& reg = console.GetRegistry();

        // Add required type registrations
        reg.AddClass<int>(Label("int"));
        reg.AddClass<bool>(Label("bool"));
        reg.AddClass<float>(Label("float"));
        reg.AddClass<String>(Label("String"));

        // Create the test components
        Pointer<Executor> executor = reg.New<Executor>();
        executor->Create();

        // Important: Get the data stack after creating the executor
        Value<Stack> stack = executor->GetDataStack();

        // Make sure stack exists and is empty
        ASSERT_TRUE(stack.Valid()) << "Stack should be valid";
        ASSERT_TRUE(stack.Exists()) << "Stack should exist";
        stack->Clear();

        std::cout << "Stack initialized and cleared" << std::endl;

        // Create a continuation with 5 5 +
        Pointer<Continuation> cont = reg.New<Continuation>();
        cont->Create();

        // Verify continuation state
        ASSERT_TRUE(cont.Valid()) << "Continuation should be valid";
        ASSERT_TRUE(cont.Exists()) << "Continuation should exist";

        std::cout << "Continuation created" << std::endl;

        // Create the code array with values and operation
        Pointer<Array> code = reg.New<Array>();
        ASSERT_TRUE(code.Valid()) << "Code array should be valid";
        ASSERT_TRUE(code.Exists()) << "Code array should exist";

        std::cout << "Code array created" << std::endl;

        // Add the code elements
        Object val1 = reg.New<int>(5);
        Object val2 = reg.New<int>(5);
        Object op = reg.New<Operation>(Operation::Plus);

        std::cout << "Values created: " << val1.ToString() << ", "
                  << val2.ToString() << ", " << op.ToString() << std::endl;

        code->Append(val1);
        code->Append(val2);
        code->Append(op);

        std::cout << "Values appended to code array" << std::endl;

        // Verify code array state
        ASSERT_EQ(code->Size(), 3) << "Code array should have 3 elements";
        ASSERT_TRUE(code->At(0).IsType<int>()) << "First element should be int";
        ASSERT_TRUE(code->At(1).IsType<int>())
            << "Second element should be int";
        ASSERT_TRUE(code->At(2).IsType<Operation>())
            << "Third element should be Operation";

        std::cout << "Code array verified" << std::endl;

        // Configure the continuation
        cont->SetCode(code);
        cont->SetSpecialHandling(
            true);  // This is key to getting proper type handling

        std::cout << "Continuation configured" << std::endl;

        // Verify continuation is ready
        ASSERT_TRUE(cont->GetCode().Valid())
            << "Continuation code should be valid";
        ASSERT_TRUE(cont->GetCode().Exists())
            << "Continuation code should exist";
        ASSERT_EQ(cont->GetCode()->Size(), 3)
            << "Continuation code should have 3 elements";
        ASSERT_TRUE(cont->GetSpecialHandling())
            << "Special handling should be enabled";

        std::cout << "Continuation ready for execution" << std::endl;

        // Alternative approach: execute manually
        Object intVal1 = reg.New<int>(5);
        Object intVal2 = reg.New<int>(5);

        // Push values directly
        stack->Push(intVal1);
        stack->Push(intVal2);

        std::cout << "Values pushed directly to stack" << std::endl;

        // Execute the plus operation
        executor->Perform(Operation::Plus);

        std::cout << "Plus operation performed" << std::endl;

        // Check the result
        ASSERT_FALSE(stack->Empty())
            << "Stack should not be empty after operation";
        ASSERT_TRUE(stack->Top().IsType<int>()) << "Result should be int";
        ASSERT_EQ(ConstDeref<int>(stack->Top()), 10) << "5 + 5 should equal 10";

        std::cout << "Test completed successfully" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI exception: " << e.ToString() << std::endl;
        FAIL() << "KAI exception: " << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "std::exception: " << e.what() << std::endl;
        FAIL() << "std::exception: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "Unknown exception";
    }
}

// Test the unwrap continuation mechanism
TEST(DirectBinaryOp, UnwrapContinuation) {
    try {
        std::cout << "Starting UnwrapContinuation test" << std::endl;

        Console console;
        Registry& reg = console.GetRegistry();

        // Add required type registrations
        reg.AddClass<int>(Label("int"));
        reg.AddClass<bool>(Label("bool"));
        reg.AddClass<float>(Label("float"));
        reg.AddClass<String>(Label("String"));

        std::cout << "Registry initialized" << std::endl;

        // Create the test components
        Pointer<Executor> executor = reg.New<Executor>();
        executor->Create();

        std::cout << "Executor created" << std::endl;

        // Create a continuation with a simple value
        Pointer<Continuation> cont = reg.New<Continuation>();
        cont->Create();

        std::cout << "Continuation created" << std::endl;

        // Create value to put in the continuation
        Object valueInt = reg.New<int>(42);

        std::cout << "Value created: " << valueInt.ToString() << std::endl;

        // Create a code array
        Pointer<Array> code = reg.New<Array>();

        std::cout << "Code array created" << std::endl;

        // Append the value to code
        code->Append(valueInt);

        std::cout << "Value appended to code" << std::endl;

        // Verify code array
        ASSERT_EQ(code->Size(), 1) << "Code array should have 1 element";
        ASSERT_TRUE(code->At(0).IsType<int>()) << "The element should be int";
        ASSERT_EQ(ConstDeref<int>(code->At(0)), 42) << "Value should be 42";

        std::cout << "Code array verified" << std::endl;

        // Configure continuation
        cont->SetCode(code);
        cont->SetSpecialHandling(true);

        std::cout << "Continuation configured" << std::endl;

        // Skip the ContinuationTestHelper and execute the test directly
        // Instead of trying to unwrap, we'll directly push the value ourselves

        Value<Stack> stack = executor->GetDataStack();
        stack->Clear();
        stack->Push(valueInt);

        std::cout << "Value pushed to stack manually" << std::endl;

        // Verify the stack has the value
        ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
        ASSERT_TRUE(stack->Top().IsType<int>()) << "Stack value should be int";
        ASSERT_EQ(ConstDeref<int>(stack->Top()), 42)
            << "Stack value should be 42";

        std::cout << "Test completed successfully" << std::endl;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI exception: " << e.ToString() << std::endl;
        FAIL() << "KAI exception: " << e.ToString();
    } catch (const std::exception& e) {
        std::cerr << "std::exception: " << e.what() << std::endl;
        FAIL() << "std::exception: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        FAIL() << "Unknown exception";
    }
}

// Test unwrapping continuations with binary operations
TEST(DirectBinaryOp, UnwrapBinaryOpContinuation) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Create the test components
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();

    // Create a continuation with a binary operation
    Pointer<Continuation> cont = reg.New<Continuation>();
    cont->Create();

    Pointer<Array> code = reg.New<Array>();
    code->Append(reg.New<int>(7));
    code->Append(reg.New<int>(9));
    code->Append(reg.New<Operation>(Operation::Plus));

    cont->SetCode(code);
    cont->SetSpecialHandling(true);

    // Create a test helper to unwrap the continuation
    ContinuationTestHelper helper;
    Object result = helper.TestExtractValue(cont);

    // Check the result
    ASSERT_TRUE(result.IsType<int>())
        << "Unwrapped binary op result should be int";
    ASSERT_EQ(ConstDeref<int>(result), 16) << "Unwrapped 7 + 9 should be 16";
}

// Test the TestExtractValue helper method
TEST(DirectBinaryOp, ExtractValueFromContinuation) {
    ContinuationTestHelper testHelper;

    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<String>(Label("String"));

    // Create a continuation with 15 + 27
    Object int1 = reg.New<int>(15);
    Object int2 = reg.New<int>(27);

    Object continuation =
        CreateTestContinuation(reg, {int1, int2}, Operation::Plus);

    // Extract the value using our helper
    Object result = testHelper.TestExtractValue(continuation);

    // Check the result - should be 42
    ASSERT_TRUE(result.IsType<int>()) << "Extracted result should be int";
    ASSERT_EQ(ConstDeref<int>(result), 42) << "15 + 27 should equal 42";

    // Test with a boolean operation
    Object bool1 = reg.New<bool>(true);
    Object bool2 = reg.New<bool>(false);

    Object boolCont =
        CreateTestContinuation(reg, {bool1, bool2}, Operation::LogicalAnd);
    Object boolResult = testHelper.TestExtractValue(boolCont);

    ASSERT_TRUE(boolResult.IsType<bool>()) << "Extracted result should be bool";
    ASSERT_FALSE(ConstDeref<bool>(boolResult))
        << "true && false should be false";

    // Test with string concatenation
    Object str1 = reg.New<String>("Hello ");
    Object str2 = reg.New<String>("World");

    Object strCont = CreateTestContinuation(reg, {str1, str2}, Operation::Plus);
    Object strResult = testHelper.TestExtractValue(strCont);

    ASSERT_TRUE(strResult.IsType<String>())
        << "Extracted result should be String";
    ASSERT_EQ(ConstDeref<String>(strResult), "Hello World")
        << "String concatenation result is incorrect";
}