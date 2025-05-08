#include <iostream>

#include "KAI/Core/Console.h"
#include "KAI/Core/File.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace KAI;
using namespace std;

// Simple standalone test for do-while loops
int main() {
    // Initialize console
    Console console;
    console.SetLanguage(Language::Rho);

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
        string scriptContent = File::ReadAllText(scriptPath);
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
        return 0;
    } catch (const Exception::Base& e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
    } catch (const std::exception& e) {
        cerr << "C++ Exception: " << e.what() << endl;
    } catch (...) {
        cerr << "Unknown exception" << endl;
    }

    return 1;
}