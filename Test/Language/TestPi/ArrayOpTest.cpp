#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/BuiltinTypes/Array.h>
#include <KAI/Core/BuiltinTypes/Map.h>

#include "TestLangCommon.h"

using namespace kai;
using namespace std;

struct ArrayOpTest : TestLangCommon {};

// Test array operations directly using Executor
TEST_F(ArrayOpTest, DirectArrayTest) {
    // Direct test of array operations without depending on Pi language parsing

    // Test ToArray operation directly
    data_->Clear();

    // First put a count of 0 on the stack, simulating "[]"
    data_->Push(reg_->New<int>(0));

    // Push ToArray operation
    auto toArrayOp = reg_->New<Operation>(Operation::ToArray);
    console_.GetExecutor()->Eval(toArrayOp);

    // Verify we have an array on the stack
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<Array>());
    ASSERT_EQ(Deref<Array>(data_->Top()).Size(), 0);

    // Test Size operation on empty array
    data_->Clear();

    // Create an empty array
    auto emptyArray = reg_->New<Array>();
    data_->Push(emptyArray);

    // Push Size operation
    auto sizeOp = reg_->New<Operation>(Operation::Size);
    console_.GetExecutor()->Eval(sizeOp);

    // Verify the size is 0
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(Deref<int>(data_->Top()), 0);

    // Test size of non-empty array
    data_->Clear();

    // Create array with elements
    auto array = reg_->New<Array>();
    auto& arr = Deref<Array>(array);
    arr.Append(reg_->New<int>(1));
    arr.Append(reg_->New<int>(2));
    arr.Append(reg_->New<int>(3));
    data_->Push(array);

    // Push Size operation
    console_.GetExecutor()->Eval(sizeOp);

    // Verify size is 3
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(Deref<int>(data_->Top()), 3);

    // Test Plus operation with integers
    data_->Clear();

    // Push two integers
    data_->Push(reg_->New<int>(3));
    data_->Push(reg_->New<int>(4));

    // Push Plus operation
    auto plusOp = reg_->New<Operation>(Operation::Plus);
    console_.GetExecutor()->Eval(plusOp);

    // Verify result is 7
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(Deref<int>(data_->Top()), 7);

    // Test Plus operation with strings
    data_->Clear();

    // Push two strings
    data_->Push(reg_->New<String>("Hello"));
    data_->Push(reg_->New<String>(" World"));

    // Push Plus operation
    console_.GetExecutor()->Eval(plusOp);

    // Verify result is "Hello World"
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<String>());
    ASSERT_EQ(Deref<String>(data_->Top()), "Hello World");

    // Test Minus operation
    data_->Clear();

    // Push two integers
    data_->Push(reg_->New<int>(10));
    data_->Push(reg_->New<int>(3));

    // Push Minus operation
    auto minusOp = reg_->New<Operation>(Operation::Minus);
    console_.GetExecutor()->Eval(minusOp);

    // Verify result is 7
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(Deref<int>(data_->Top()), 7);

    // Test Multiply operation
    data_->Clear();

    // Push two integers
    data_->Push(reg_->New<int>(3));
    data_->Push(reg_->New<int>(4));

    // Push Multiply operation
    auto multiplyOp = reg_->New<Operation>(Operation::Multiply);
    console_.GetExecutor()->Eval(multiplyOp);

    // Verify result is 12
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(Deref<int>(data_->Top()), 12);

    // Test Divide operation
    data_->Clear();

    // Push two integers
    data_->Push(reg_->New<int>(12));
    data_->Push(reg_->New<int>(3));

    // Push Divide operation
    auto divideOp = reg_->New<Operation>(Operation::Divide);
    console_.GetExecutor()->Eval(divideOp);

    // Verify result is 4
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(Deref<int>(data_->Top()), 4);

    // Test LogicalAnd operation
    data_->Clear();

    // Push two booleans
    data_->Push(reg_->New<bool>(true));
    data_->Push(reg_->New<bool>(false));

    // Push LogicalAnd operation
    auto andOp = reg_->New<Operation>(Operation::LogicalAnd);
    console_.GetExecutor()->Eval(andOp);

    // Verify result is false
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_FALSE(Deref<bool>(data_->Top()));

    // Test LogicalOr operation
    data_->Clear();

    // Push two booleans
    data_->Push(reg_->New<bool>(true));
    data_->Push(reg_->New<bool>(false));

    // Push LogicalOr operation
    auto orOp = reg_->New<Operation>(Operation::LogicalOr);
    console_.GetExecutor()->Eval(orOp);

    // Verify result is true
    ASSERT_EQ(data_->Size(), 1);
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(Deref<bool>(data_->Top()));

    std::cout << "All direct operation tests passed successfully!" << std::endl;
}