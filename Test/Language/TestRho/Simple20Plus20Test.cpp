#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// A minimal test fixture for just testing "20 20 +"
class Simple20Plus20Test : public ::testing::Test {
protected:
    Console console;
    Registry* reg;
    Pointer<Stack> stack;
    
    void SetUp() override {
        reg = &console.GetRegistry();
        reg->AddClass<int>(Label("int"));
        stack = console.GetExecutor()->GetDataStack();
        console.SetLanguage(Language::Pi);
    }
};

// Test specifically for the "20 20 +" case
TEST_F(Simple20Plus20Test, DirectTest) {
    // Execute Pi expression
    stack->Clear();
    console.Execute("20 20 +");
    
    // Print debug info
    cout << "After execution, stack size: " << stack->Size() << endl;
    
    // Check the result
    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    
    Object result = stack->Top();
    
    if (result.IsType<Continuation>()) {
        cout << "Result is a continuation" << endl;
        
        // Unwrap manually
        Pointer<Continuation> cont = stack->Top();
        if (cont->GetCode()->Size() == 1) {
            cout << "Continuation has one element" << endl;
            
            // It's likely the ContinuationBegin-value-ContinuationEnd pattern 
            // wrapped in another continuation
            Object element = cont->GetCode()->At(0);
            
            if (element.IsType<Continuation>()) {
                Pointer<Continuation> innerCont = element;
                
                if (innerCont->GetCode()->Size() == 3) {
                    cout << "Inner continuation has 3 elements, checking pattern..." << endl;
                    
                    Object first = innerCont->GetCode()->At(0);
                    Object middle = innerCont->GetCode()->At(1);
                    Object last = innerCont->GetCode()->At(2);
                    
                    if (first.IsType<Operation>() && 
                        ConstDeref<Operation>(first).GetTypeNumber() == Operation::ContinuationBegin &&
                        last.IsType<Operation>() && 
                        ConstDeref<Operation>(last).GetTypeNumber() == Operation::ContinuationEnd) {
                        
                        cout << "Found begin-value-end pattern" << endl;
                        
                        if (middle.IsType<int>()) {
                            int result = ConstDeref<int>(middle);
                            cout << "Result value: " << result << endl;
                            
                            // Replace stack entry
                            stack->Pop();
                            stack->Push(reg->New<int>(result));
                        }
                    }
                }
            }
        }
    }
    
    // Now stack should have an integer
    ASSERT_TRUE(stack->Top().IsType<int>()) << "Result should be an integer, but got " 
                                          << stack->Top().GetClass()->GetName();
    ASSERT_EQ(ConstDeref<int>(stack->Top()), 40) << "Result should be 40";
}