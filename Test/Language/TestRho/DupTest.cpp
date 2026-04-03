#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "KAI/Executor/Operation.h"

using namespace kai;
using namespace std;

// Test for the problematic "5 dup +" pattern
TEST(RhoDupTests, DupPlusPattern) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Get executor and clean stack
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Manually push 5 onto the stack
    Object value5 = reg.New<int>(5);
    exec->Push(value5);

    // Execute the 'dup' operation to duplicate the value
    exec->Perform(Operation::Dup);

    // Stack should now have [5, 5]
    ASSERT_EQ(stack->Size(), 2);
    ASSERT_TRUE(stack->At(0).IsType<int>());
    ASSERT_TRUE(stack->At(1).IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 5);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 5);

    // Execute the '+' operation
    exec->Perform(Operation::Plus);

    // Stack should now have [10]
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// Test for direct Dup operation with integers
TEST(RhoDupTests, DupIntegerValue) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Get executor and clean stack
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Push a test value
    Object value = reg.New<int>(42);
    exec->Push(value);

    // Execute 'dup'
    exec->Perform(Operation::Dup);

    // Verify top two values are identical
    ASSERT_EQ(stack->Size(), 2);
    ASSERT_TRUE(stack->At(0).IsType<int>());
    ASSERT_TRUE(stack->At(1).IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 42);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 42);
}

// Test for direct "5 5 +" without dup
TEST(RhoDupTests, DirectAdditionWithoutDup) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Get executor and clean stack
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Manually push two 5s onto the stack
    Object value5 = reg.New<int>(5);
    exec->Push(value5);
    exec->Push(
        value5.Clone());  // Use Clone to ensure we have two distinct objects

    // Verify the stack has two 5s
    ASSERT_EQ(stack->Size(), 2);
    ASSERT_TRUE(stack->At(0).IsType<int>());
    ASSERT_TRUE(stack->At(1).IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->At(0)), 5);
    ASSERT_EQ(ConstDeref<int>(stack->At(1)), 5);

    // Execute the '+' operation
    exec->Perform(Operation::Plus);

    // Stack should now have [10]
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// Test for 5 dup + via execution of Pi code
TEST(RhoDupTests, ExecutePiCode) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Execute the full Pi code string
    console.Execute("5 dup +");

    // Get stack to verify result
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();

    // Verify the stack has one value which is 10
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}

// Test for using the special handling feature in continuations
TEST(RhoDupTests, SpecialHandlingContinuation) {
    Console console;
    console.SetLanguage(Language::Pi);

    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));

    // Create a continuation with the [5, 5, +] pattern
    Pointer<Continuation> cont = reg.New<Continuation>();
    cont->Create();

    Pointer<Array> code = reg.New<Array>();
    code->Append(reg.New<int>(5));
    code->Append(reg.New<int>(5));
    code->Append(reg.New<Operation>(Operation::Plus));

    cont->SetCode(code);
    cont->SetSpecialHandling(true);  // Set special handling flag

    // Get executor and clean stack
    auto exec = console.GetExecutor();
    auto stack = exec->GetDataStack();
    stack->Clear();

    // Execute the continuation
    exec->Continue(cont);

    // Stack should now have [10]
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 10);
}