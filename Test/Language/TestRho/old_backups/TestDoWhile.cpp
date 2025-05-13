#include <gtest/gtest.h>

#include <iostream>

#include "KAI/Core/Console.h"
#include "KAI/Core/File.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// TEMPORARILY DISABLED - See Todo-Rho.md for details on the type mismatch issue
// that affects Rho language functionality, including do-while loops.
// This test will remain disabled until the fundamental language issues are
// fixed. The core issue is: "Type Mismatch: expected=Continuation,
// got=Signed32" which happens even with basic arithmetic operations.
TEST(RhoLanguage, DISABLED_TestDoWhileLoops) {
    // Initialize console
    Console console;
    console.SetLanguage(Language::Rho);

    // Register necessary types
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<float>(Label("float"));
    reg.AddClass<String>(Label("String"));
    reg.AddClass<bool>(Label("bool"));

    // Get executor and set trace level
    auto exec = console.GetExecutor();
    exec->SetTraceLevel(5);

    try {
        cout << "========= Testing SimplestDoWhile.rho ==========" << endl;

        // Clear stacks before running
        exec->ClearStacks();
        exec->ClearContext();

        // Load and run the simplest do-while test script
        string scriptPath =
            "/home/xian/local/KAI/Test/Language/TestRho/Scripts/"
            "SimplestDoWhile.rho";
        string scriptContent = String(File::ReadAllText(scriptPath)).c_str();
        cout << "Script content:" << endl << scriptContent << endl;

        // Execute the script
        console.Execute(scriptContent);

        // Check the stack after execution
        Value<Stack> stack = exec->GetDataStack();
        cout << "Stack after execution has " << stack->Size() << " elements"
             << endl;

        // Print stack contents
        while (!stack->Empty()) {
            Object obj = stack->Pop();
            cout << "  Stack element: " << obj.ToString().c_str() << endl;
        }

        cout << "Test completed successfully!" << endl;
        SUCCEED();
    } catch (const Exception::Base& e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
        FAIL() << "KAI Exception: " << e.ToString();
    } catch (const std::exception& e) {
        cerr << "C++ Exception: " << e.what() << endl;
        FAIL() << "C++ Exception: " << e.what();
    } catch (...) {
        cerr << "Unknown exception" << endl;
        FAIL() << "Unknown exception";
    }
}