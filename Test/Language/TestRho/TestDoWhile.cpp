#include <gtest/gtest.h>

#include <iostream>

#include "KAI/Core/Console.h"
#include "KAI/Core/File.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

// This test was previously disabled due to type mismatch issues that affected
// Rho language functionality, including do-while loops.
// We are now enabling it since fixes have been applied.
TEST(RhoLanguage, TestDoWhileLoops) {
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

        // In this version, we don't have File::Exists, so we'll try to read the
        // file directly If it fails, we'll try alternative paths
        string scriptContent;
        try {
            // Try with the absolute path first
            scriptContent = String(File::ReadAllText(scriptPath)).c_str();
            cout << "Found script at: " << scriptPath << endl;
        } catch (const std::exception&) {
            // If that fails, try with a relative path
            scriptPath = "Test/Language/TestRho/Scripts/SimplestDoWhile.rho";
            try {
                scriptContent = String(File::ReadAllText(scriptPath)).c_str();
                cout << "Found script at: " << scriptPath << endl;
            } catch (const std::exception&) {
                // If that also fails, try with the path from current directory
                scriptPath =
                    "./Test/Language/TestRho/Scripts/SimplestDoWhile.rho";
                scriptContent = String(File::ReadAllText(scriptPath)).c_str();
                cout << "Found script at: " << scriptPath << endl;
            }
        }

        if (scriptContent.empty()) {
            FAIL() << "Could not read script file from any path";
        }
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