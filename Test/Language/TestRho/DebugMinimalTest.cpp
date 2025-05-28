#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "TestLangCommon.h"

using namespace kai;

struct DebugMinimalTest : TestLangCommon {
    void TestScript(const char *script) {
        Console console;
        console.SetLanguage(Language::Rho);

        try {
            console.Execute(script);

            auto executor = console.GetExecutor();
            auto dataStack = executor->GetDataStack();

            std::cout << "Stack size after execution: " << dataStack->Size()
                      << std::endl;
            if (!dataStack->Empty()) {
                auto val = dataStack->Top();
                std::cout << "Top of stack type: " << val.GetClass()->GetName()
                          << std::endl;
                if (val.IsType<int>()) {
                    std::cout << "Value: " << ConstDeref<int>(val) << std::endl;
                }
            }
        } catch (const Exception::Base &e) {
            std::cout << "Exception: " << e.ToString() << std::endl;
        }
    }
};

TEST_F(DebugMinimalTest, SimpleVariable) { TestScript("x = 42\nx"); }

TEST_F(DebugMinimalTest, SimpleIf) {
    TestScript("result = 0\nif true\n    result = 42\nresult");
}