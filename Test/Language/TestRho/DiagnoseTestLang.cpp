#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// Helper function to dump stack info
void DumpStack(Stack* stack) {
    std::cout << "Stack size: " << stack->Size() << std::endl;

    for (int i = 0; i < stack->Size(); i++) {
        Object obj = stack->At(i);
        std::cout << "Item " << i << ": ";
        if (obj.Exists() && obj.GetClass()) {
            std::cout << "Type=" << obj.GetClass()->GetName().ToString();

            // Handle specific types
            if (obj.IsType<int>()) {
                std::cout << ", Value=" << ConstDeref<int>(obj);
            } else if (obj.IsType<bool>()) {
                std::cout << ", Value="
                          << (ConstDeref<bool>(obj) ? "true" : "false");
            } else if (obj.IsType<String>()) {
                std::cout << ", Value=\"" << ConstDeref<String>(obj) << "\"";
            } else if (obj.IsType<Continuation>()) {
                Continuation& cont = Deref<Continuation>(obj);
                int codeSize = cont.GetCode()->Size();
                std::cout << ", Code size=" << codeSize;

                // Print the code elements if available
                if (codeSize > 0) {
                    std::cout << ", Contents=[";
                    for (int j = 0; j < codeSize && j < 5; j++) {
                        if (j > 0) std::cout << ", ";

                        Object codeObj = cont.GetCode()->At(j);
                        if (codeObj.Exists() && codeObj.GetClass()) {
                            std::cout
                                << codeObj.GetClass()->GetName().ToString();

                            // For operations, show which one
                            if (codeObj.IsType<Operation>()) {
                                Operation::Type op =
                                    ConstDeref<Operation>(codeObj)
                                        .GetTypeNumber();
                                std::cout << "(" << Operation::ToString(op)
                                          << ")";
                            }
                        } else {
                            std::cout << "null";
                        }
                    }
                    if (codeSize > 5) std::cout << ", ...";
                    std::cout << "]";
                }
            }
        } else {
            std::cout << "null object";
        }
        std::cout << std::endl;
    }
}

// Test fixture for diagnosing Pi operations and continuation unwrapping
class DiagnoseTestLang : public TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<String>(Label("String"));
    }
};

// Test to diagnose the structure of TestLangCommon and how Pi operations work
TEST_F(DiagnoseTestLang, PiOperation) {
    // Addition
    data_->Clear();
    std::cout << "\n=== Test Addition ===" << std::endl;
    console_.Execute("2 3 +");

    std::cout << "Stack after Pi execution:" << std::endl;
    DumpStack(data_);

    // Use UnwrapStackValues to extract primitive values from continuations
    UnwrapStackValues();

    std::cout << "Stack after UnwrapStackValues:" << std::endl;
    DumpStack(data_);

    // Comparison
    data_->Clear();
    std::cout << "\n=== Test Comparison ===" << std::endl;
    console_.Execute("10 5 >");

    std::cout << "Stack after Pi execution:" << std::endl;
    DumpStack(data_);

    // Use UnwrapStackValues to extract primitive values from continuations
    UnwrapStackValues();

    std::cout << "Stack after UnwrapStackValues:" << std::endl;
    DumpStack(data_);

    // String
    data_->Clear();
    std::cout << "\n=== Test String ===" << std::endl;
    console_.Execute("\"Hello World\"");

    std::cout << "Stack after Pi execution:" << std::endl;
    DumpStack(data_);

    // Use UnwrapStackValues to extract primitive values from continuations
    UnwrapStackValues();

    std::cout << "Stack after UnwrapStackValues:" << std::endl;
    DumpStack(data_);

    // We expect all tests to use UnwrapStackValues to extract primitive types
    // from continuations, and our implementation should handle all the patterns
    // But for now, we're just diagnosing
    ASSERT_TRUE(true);
}