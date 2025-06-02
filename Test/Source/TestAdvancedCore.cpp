#include <iostream>
#include <string>
#include <memory>
#include "KAI/Test/Base.h"
#include "KAI/Core/Object.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Operation.h"
#include "KAI/Executor/Executor.h"
#include "KAI/Executor/Continuation.h"

using namespace kai;
using namespace std;

struct TestAdvancedCore : TestBase {};

// Test the ForLoop operation directly at the executor level
TEST_F(TestAdvancedCore, ForLoopRangeBased) {
    // Create an executor
    auto exec = reg->New<Executor>();
    auto stack = exec->GetDataStack();
    
    // Test range-based for loop: 0 1 5 { + } for
    // This should sum numbers from 1 to 5
    
    // Push accumulator
    stack->Push(reg->New<int>(0));
    
    // Push start
    stack->Push(reg->New<int>(1));
    
    // Push end
    stack->Push(reg->New<int>(5));
    
    // Create body continuation for addition
    auto bodyCont = reg->New<Continuation>();
    auto bodyCode = reg->New<Array>();
    bodyCode->Append(reg->New<Operation>(Operation::Plus));
    bodyCont->SetCode(bodyCode);
    stack->Push(bodyCont);
    
    // Execute ForLoop operation
    exec->Perform(Operation::ForLoop);
    
    // Check result
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 15);  // 1+2+3+4+5 = 15
}

// Test traditional 4-continuation for loop at executor level
TEST_F(TestAdvancedCore, ForLoopTraditional) {
    // Create an executor
    auto exec = reg->New<Executor>();
    auto stack = exec->GetDataStack();
    
    // Traditional for loop: for(i=0; i<3; i++) { ... }
    
    // Create init continuation: { 0 }
    auto initCont = reg->New<Continuation>();
    auto initCode = reg->New<Array>();
    initCode->Append(reg->New<int>(0));
    initCont->SetCode(initCode);
    stack->Push(initCont);
    
    // Create condition continuation: { dup 3 < }
    auto condCont = reg->New<Continuation>();
    auto condCode = reg->New<Array>();
    condCode->Append(reg->New<Operation>(Operation::Dup));
    condCode->Append(reg->New<int>(3));
    condCode->Append(reg->New<Operation>(Operation::Less));
    condCont->SetCode(condCode);
    stack->Push(condCont);
    
    // Create increment continuation: { 1 + }
    auto incrCont = reg->New<Continuation>();
    auto incrCode = reg->New<Array>();
    incrCode->Append(reg->New<int>(1));
    incrCode->Append(reg->New<Operation>(Operation::Plus));
    incrCont->SetCode(incrCode);
    stack->Push(incrCont);
    
    // Create empty body continuation: { }
    auto bodyCont = reg->New<Continuation>();
    auto bodyCode = reg->New<Array>();
    bodyCont->SetCode(bodyCode);
    stack->Push(bodyCont);
    
    // Execute ForLoop operation
    exec->Perform(Operation::ForLoop);
    
    // Check result - should have final counter value
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 3);
}

// Test edge cases for range-based for loop
TEST_F(TestAdvancedCore, ForLoopEdgeCases) {
    // Create an executor
    auto exec = reg->New<Executor>();
    auto stack = exec->GetDataStack();
    
    // Test 1: Single iteration (start == end)
    stack->Push(reg->New<int>(10));  // accumulator
    stack->Push(reg->New<int>(5));   // start
    stack->Push(reg->New<int>(5));   // end
    
    auto bodyCont = reg->New<Continuation>();
    auto bodyCode = reg->New<Array>();
    bodyCode->Append(reg->New<Operation>(Operation::Plus));
    bodyCont->SetCode(bodyCode);
    stack->Push(bodyCont);
    
    exec->Perform(Operation::ForLoop);
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 15);  // 10 + 5 = 15
    
    stack->Clear();
    
    // Test 2: No iterations (start > end)
    stack->Push(reg->New<int>(100));  // accumulator
    stack->Push(reg->New<int>(10));   // start
    stack->Push(reg->New<int>(5));    // end
    
    bodyCont = reg->New<Continuation>();
    bodyCode = reg->New<Array>();
    bodyCode->Append(reg->New<Operation>(Operation::Plus));
    bodyCont->SetCode(bodyCode);
    stack->Push(bodyCont);
    
    exec->Perform(Operation::ForLoop);
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 100);  // No iterations
}

// Test complex operations in for loop body
TEST_F(TestAdvancedCore, ForLoopComplexBody) {
    // Create an executor
    auto exec = reg->New<Executor>();
    auto stack = exec->GetDataStack();
    
    // Factorial calculation: 1 1 5 { * } for
    stack->Push(reg->New<int>(1));  // accumulator
    stack->Push(reg->New<int>(1));  // start
    stack->Push(reg->New<int>(5));  // end
    
    auto bodyCont = reg->New<Continuation>();
    auto bodyCode = reg->New<Array>();
    bodyCode->Append(reg->New<Operation>(Operation::Multiply));
    bodyCont->SetCode(bodyCode);
    stack->Push(bodyCont);
    
    exec->Perform(Operation::ForLoop);
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 120);  // 5! = 120
}

// Test WhileLoop operation
TEST_F(TestAdvancedCore, WhileLoopOperation) {
    // Create an executor
    auto exec = reg->New<Executor>();
    auto stack = exec->GetDataStack();
    
    // While loop that counts from 0 to 3
    // Initial value
    stack->Push(reg->New<int>(0));
    
    // Create condition continuation: { dup 3 < }
    auto condCont = reg->New<Continuation>();
    auto condCode = reg->New<Array>();
    condCode->Append(reg->New<Operation>(Operation::Dup));
    condCode->Append(reg->New<int>(3));
    condCode->Append(reg->New<Operation>(Operation::Less));
    condCont->SetCode(condCode);
    stack->Push(condCont);
    
    // Create body continuation: { 1 + }
    auto bodyCont = reg->New<Continuation>();
    auto bodyCode = reg->New<Array>();
    bodyCode->Append(reg->New<int>(1));
    bodyCode->Append(reg->New<Operation>(Operation::Plus));
    bodyCont->SetCode(bodyCode);
    stack->Push(bodyCont);
    
    // Execute WhileLoop operation
    exec->Perform(Operation::WhileLoop);
    
    // Check result
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 3);
}

// Test DoLoop operation (do-while)
TEST_F(TestAdvancedCore, DoLoopOperation) {
    // Create an executor
    auto exec = reg->New<Executor>();
    auto stack = exec->GetDataStack();
    
    // Do-while loop that counts from 0 to 3
    // Initial value
    stack->Push(reg->New<int>(0));
    
    // Create body continuation: { 1 + }
    auto bodyCont = reg->New<Continuation>();
    auto bodyCode = reg->New<Array>();
    bodyCode->Append(reg->New<int>(1));
    bodyCode->Append(reg->New<Operation>(Operation::Plus));
    bodyCont->SetCode(bodyCode);
    stack->Push(bodyCont);
    
    // Create condition continuation: { dup 3 < }
    auto condCont = reg->New<Continuation>();
    auto condCode = reg->New<Array>();
    condCode->Append(reg->New<Operation>(Operation::Dup));
    condCode->Append(reg->New<int>(3));
    condCode->Append(reg->New<Operation>(Operation::Less));
    condCont->SetCode(condCode);
    stack->Push(condCont);
    
    // Execute DoLoop operation
    exec->Perform(Operation::DoLoop);
    
    // Check result
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 3);
}