#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "TestLangCommon.h"

using namespace kai;

struct MinimalControlFlowTest : TestLangCommon {
    void TestScript(const char *script, bool expectValue = true) {
        Console console;
        console.SetLanguage(Language::Rho);

        std::cout << "\n=== Testing script: ===\n"
                  << script << "\n===================\n";

        try {
            console.Execute(script);

            auto executor = console.GetExecutor();
            auto dataStack = executor->GetDataStack();

            std::cout << "Execution completed. Stack size: "
                      << dataStack->Size() << std::endl;

            if (expectValue) {
                if (!dataStack->Empty()) {
                    auto val = dataStack->Top();
                    std::cout << "Top of stack: " << val.ToString()
                              << std::endl;
                    if (val.GetClass()) {
                        std::cout << "Type: " << val.GetClass()->GetName()
                                  << std::endl;
                    }
                } else {
                    std::cout << "ERROR: Stack is empty!" << std::endl;
                }
            }
        } catch (const Exception::Base &e) {
            std::cout << "Exception: " << e.ToString() << std::endl;
        } catch (...) {
            std::cout << "Unknown exception" << std::endl;
        }
    }
};

TEST_F(MinimalControlFlowTest, SimpleExpression) { TestScript("42"); }

TEST_F(MinimalControlFlowTest, SimpleVariable) { TestScript("x = 42\nx"); }

TEST_F(MinimalControlFlowTest, SimpleAssignment) {
    TestScript("x = 10\ny = 20\nx + y");
}

TEST_F(MinimalControlFlowTest, SimpleIf) {
    TestScript("result = 0\nif true\n    result = 42\nresult");
}