#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "KAI/Core/Console.h"
#include "KAI/Core/File.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;
namespace fs = std::filesystem;

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

        // Load and run the simplest do-while test script.
        //
        // KAI_SCRIPT_ROOT is injected by Test/Language/CMakeLists.txt as the
        // absolute path to this language's Scripts/ directory (see
        // target_compile_definitions there), the same mechanism
        // TestLangCommon::ExecScriptFile already uses. The old code here
        // guessed a handful of paths relative to the *process's current
        // working directory* ("./Test/Language/...", a bare relative path,
        // or $KAI_ROOT/...), which only happens to work when ctest is
        // invoked from the repo root - anywhere else (a different working
        // directory, an IDE test runner, a build dir on another machine)
        // File::ReadAllText silently returned an empty string instead of
        // throwing, so `found` stayed true (the "Found script at:" line
        // still printed) while scriptContent.empty() was actually what
        // tripped the FAIL() below.
        const fs::path scriptPath =
            fs::path(KAI_STRINGISE(KAI_SCRIPT_ROOT)) / "SimplestDoWhile.rho";

        string scriptContent;
        bool found = false;

        if (fs::exists(scriptPath)) {
            scriptContent = String(File::ReadAllText(scriptPath)).c_str();
            found = !scriptContent.empty();
            cout << "Found script at: " << scriptPath.string() << endl;
        }

        if (!found) {
            FAIL() << "Could not read script file from " << scriptPath.string();
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
    } catch (const exception::Base& e) {
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
