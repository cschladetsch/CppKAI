#include <gtest/gtest.h>

#include <iostream>

#include "KAI/Core/Console.h"
#include "KAI/Core/File.h"
#include "KAI/Language/Rho/RhoTranslator.h"
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
    
    // Set up the translator for Rho language
    Registry& reg = console.GetRegistry();
    auto translator = std::make_shared<RhoTranslator>(reg);
    console.SetTranslator(translator);

    // Register necessary types
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
        vector<string> possiblePaths = {
            "./Test/Language/TestRho/Scripts/SimplestDoWhile.rho",
            "Test/Language/TestRho/Scripts/SimplestDoWhile.rho",
            "/home/xian/local/KAI/Test/Language/TestRho/Scripts/SimplestDoWhile.rho"
        };

        string scriptContent;
        string scriptPath;
        bool found = false;
        
        for (const auto& path : possiblePaths) {
            try {
                scriptContent = String(File::ReadAllText(path)).c_str();
                scriptPath = path;
                found = true;
                cout << "Found script at: " << scriptPath << endl;
                break;
            } catch (const std::exception&) {
                // Try next path
                continue;
            }
        }

        if (!found || scriptContent.empty()) {
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