#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"

using namespace kai;
using namespace std;

// Helper method to directly evaluate operations at test time
Object EvaluateOperation(Registry& reg, Object first, Object second,
                         Operation::Type op) {
    // Create an executor to evaluate the operations
    Pointer<Executor> executor = reg.New<Executor>();
    executor->Create();

    // For binary operations, use PerformBinaryOp
    return executor->PerformBinaryOp(first, second, op);
}

/* Extended set of tests for Rho binary operations */

// Direct integer addition
TEST(RhoExtendedOps, IntegerAddition) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    Object a = reg.New<int>(10);
    Object b = reg.New<int>(25);

    Object result = EvaluateOperation(reg, a, b, Operation::Plus);

    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 10+25 but got "
                                      << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 35)
        << "Expected value 35 for 10+25 but got " << result.ToString();
}

// Direct integer subtraction
TEST(RhoExtendedOps, IntegerSubtraction) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    Object a = reg.New<int>(100);
    Object b = reg.New<int>(42);

    Object result = EvaluateOperation(reg, a, b, Operation::Minus);

    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 100-42 but got "
                                      << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 58)
        << "Expected value 58 for 100-42 but got " << result.ToString();
}

// Direct integer multiplication
TEST(RhoExtendedOps, IntegerMultiplication) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    Object a = reg.New<int>(12);
    Object b = reg.New<int>(5);

    Object result = EvaluateOperation(reg, a, b, Operation::Multiply);

    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 12*5 but got "
                                      << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 60)
        << "Expected value 60 for 12*5 but got " << result.ToString();
}

// Integer division
TEST(RhoExtendedOps, IntegerDivision) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    Object a = reg.New<int>(100);
    Object b = reg.New<int>(20);

    Object result = EvaluateOperation(reg, a, b, Operation::Divide);

    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 100/20 but got "
                                      << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 5)
        << "Expected value 5 for 100/20 but got " << result.ToString();
}

// Modulo operation
TEST(RhoExtendedOps, IntegerModulo) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    Object a = reg.New<int>(17);
    Object b = reg.New<int>(5);

    Object result = EvaluateOperation(reg, a, b, Operation::Modulo);

    ASSERT_TRUE(result.IsType<int>()) << "Expected int type for 17%5 but got "
                                      << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(result), 2)
        << "Expected value 2 for 17%5 but got " << result.ToString();
}

// Boolean AND operation
TEST(RhoExtendedOps, BooleanAnd) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    Object a = reg.New<bool>(true);
    Object b = reg.New<bool>(false);

    Object result = EvaluateOperation(reg, a, b, Operation::LogicalAnd);

    ASSERT_TRUE(result.IsType<bool>())
        << "Expected bool type for true&&false but got "
        << result.GetClass()->GetName();
    ASSERT_FALSE(ConstDeref<bool>(result))
        << "Expected false for true&&false but got " << result.ToString();
}

// Boolean OR operation
TEST(RhoExtendedOps, BooleanOr) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<bool>(Label("bool"));

    Object a = reg.New<bool>(true);
    Object b = reg.New<bool>(false);

    Object result = EvaluateOperation(reg, a, b, Operation::LogicalOr);

    ASSERT_TRUE(result.IsType<bool>())
        << "Expected bool type for true||false but got "
        << result.GetClass()->GetName();
    ASSERT_TRUE(ConstDeref<bool>(result))
        << "Expected true for true||false but got " << result.ToString();
}

// Equality comparison
TEST(RhoExtendedOps, EqualityComparison) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    Object a = reg.New<int>(42);
    Object b = reg.New<int>(42);
    Object c = reg.New<int>(100);

    // Test equal values
    Object resultEqual = EvaluateOperation(reg, a, b, Operation::Equiv);
    ASSERT_TRUE(resultEqual.IsType<bool>())
        << "Expected bool type for comparison but got "
        << resultEqual.GetClass()->GetName();
    ASSERT_TRUE(ConstDeref<bool>(resultEqual))
        << "Expected true for 42==42 but got " << resultEqual.ToString();

    // Test unequal values
    Object resultNotEqual = EvaluateOperation(reg, a, c, Operation::Equiv);
    ASSERT_TRUE(resultNotEqual.IsType<bool>())
        << "Expected bool type for comparison but got "
        << resultNotEqual.GetClass()->GetName();
    ASSERT_FALSE(ConstDeref<bool>(resultNotEqual))
        << "Expected false for 42==100 but got " << resultNotEqual.ToString();
}

// Inequality comparison
TEST(RhoExtendedOps, InequalityComparison) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    Object a = reg.New<int>(42);
    Object b = reg.New<int>(42);
    Object c = reg.New<int>(100);

    // Test equal values
    Object resultEqual = EvaluateOperation(reg, a, b, Operation::NotEquiv);
    ASSERT_TRUE(resultEqual.IsType<bool>())
        << "Expected bool type for comparison but got "
        << resultEqual.GetClass()->GetName();
    ASSERT_FALSE(ConstDeref<bool>(resultEqual))
        << "Expected false for 42!=42 but got " << resultEqual.ToString();

    // Test unequal values
    Object resultNotEqual = EvaluateOperation(reg, a, c, Operation::NotEquiv);
    ASSERT_TRUE(resultNotEqual.IsType<bool>())
        << "Expected bool type for comparison but got "
        << resultNotEqual.GetClass()->GetName();
    ASSERT_TRUE(ConstDeref<bool>(resultNotEqual))
        << "Expected true for 42!=100 but got " << resultNotEqual.ToString();
}

// String concatenation
TEST(RhoExtendedOps, StringConcatenation) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<String>(Label("String"));

    Object a = reg.New<String>("Hello, ");
    Object b = reg.New<String>("World!");

    Object result = EvaluateOperation(reg, a, b, Operation::Plus);

    ASSERT_TRUE(result.IsType<String>())
        << "Expected String type for concatenation but got "
        << result.GetClass()->GetName();
    ASSERT_EQ(ConstDeref<String>(result), "Hello, World!")
        << "Expected 'Hello, World!' but got " << result.ToString();
}

// Less than comparison
TEST(RhoExtendedOps, LessThanComparison) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    Object smaller = reg.New<int>(10);
    Object larger = reg.New<int>(20);

    // Test less than
    Object resultLess =
        EvaluateOperation(reg, smaller, larger, Operation::Less);
    ASSERT_TRUE(resultLess.IsType<bool>())
        << "Expected bool type for comparison but got "
        << resultLess.GetClass()->GetName();
    ASSERT_TRUE(ConstDeref<bool>(resultLess))
        << "Expected true for 10<20 but got " << resultLess.ToString();

    // Test not less than
    Object resultNotLess =
        EvaluateOperation(reg, larger, smaller, Operation::Less);
    ASSERT_TRUE(resultNotLess.IsType<bool>())
        << "Expected bool type for comparison but got "
        << resultNotLess.GetClass()->GetName();
    ASSERT_FALSE(ConstDeref<bool>(resultNotLess))
        << "Expected false for 20<10 but got " << resultNotLess.ToString();
}

// Greater than comparison
TEST(RhoExtendedOps, GreaterThanComparison) {
    Console console;
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<bool>(Label("bool"));

    Object smaller = reg.New<int>(10);
    Object larger = reg.New<int>(20);

    // Test greater than
    Object resultGreater =
        EvaluateOperation(reg, larger, smaller, Operation::Greater);
    ASSERT_TRUE(resultGreater.IsType<bool>())
        << "Expected bool type for comparison but got "
        << resultGreater.GetClass()->GetName();
    ASSERT_TRUE(ConstDeref<bool>(resultGreater))
        << "Expected true for 20>10 but got " << resultGreater.ToString();

    // Test not greater than
    Object resultNotGreater =
        EvaluateOperation(reg, smaller, larger, Operation::Greater);
    ASSERT_TRUE(resultNotGreater.IsType<bool>())
        << "Expected bool type for comparison but got "
        << resultNotGreater.GetClass()->GetName();
    ASSERT_FALSE(ConstDeref<bool>(resultNotGreater))
        << "Expected false for 10>20 but got " << resultNotGreater.ToString();
}