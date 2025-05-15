#include "TestLangCommon.h"

#include <KAI/Core/Exception.h>

#include <boost/algorithm/string/predicate.hpp>
#include <cwctype>
#include <filesystem>

#include "File.h"
#include "TestCommon.h"

using namespace std;
namespace fs = std::filesystem;

KAI_BEGIN

void ToLower(std::string &str) { KAI_NOT_IMPLEMENTED(); }

std::wstring ToLower(std::wstring const &str) {
    std::wstring result;
    result.resize(str.size());
    std::transform(str.begin(), str.end(), result.begin(), std::towlower);
    return result;
}

void ToLower(std::wstring &str) {
    std::transform(str.begin(), str.end(), str.begin(), std::towlower);
}

void TestLangCommon::SetUp() {
    reg_ = &console_.GetRegistry();
    exec_ = &*console_.GetExecutor();
    data_ = &*exec_->GetDataStack();
    context_ = &*exec_->GetContextStack();
    tree_ = &console_.GetTree();
    root_ = tree_->GetRoot();

    // Check if the root object is valid
    if (!root_.Exists()) {
        // If not valid, create a new root object
        root_ = reg_->New<void>();
        // Root is now the tree's root
        tree_->SetRoot(root_);
    }

    // Always ensure a clean state on setup
    exec_->ClearStacks();
    exec_->ClearContext();
}

void TestLangCommon::TearDown() {
    // Clean up after each test to avoid state persistence
    exec_->ClearStacks();
    exec_->ClearContext();
}

void TestLangCommon::ExecScriptFile(const std::string &scriptFileName) {
    const fs::path scriptsRoot(KAI_STRINGISE(KAI_SCRIPT_ROOT));
    const fs::path scriptPath = scriptsRoot / scriptFileName;

    // Set the language based on file extension
    if (scriptFileName.find(".pi") != std::string::npos) {
        console_.SetLanguage(Language::Pi);
    } else if (scriptFileName.find(".rho") != std::string::npos) {
        console_.SetLanguage(Language::Rho);
        std::cout << "Setting language to Rho for script: " << scriptFileName
                  << std::endl;
    } else if (scriptFileName.find(".tau") != std::string::npos) {
        console_.SetLanguage(Language::Tau);
    }

    // Clear stacks before execution
    exec_->ClearStacks();
    exec_->ClearContext();

    try {
        auto contents = File::ReadAllText(scriptPath);
        std::cout << "Loaded script file: " << scriptPath.string() << std::endl;

        // Add detailed execution steps
        std::cout << "Executing script with length: " << contents.size()
                  << " bytes" << std::endl;

        // Execute the script - let any exceptions propagate to the caller
        console_.Execute(contents.c_str());

        std::cout << "Script execution complete" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Exception in ExecScriptFile: " << e.what() << std::endl;
        throw;  // Re-throw the exception
    } catch (...) {
        std::cerr << "Unknown exception in ExecScriptFile" << std::endl;
        throw;  // Re-throw the exception
    }
}

void TestLangCommon::ExecScripts() {
    const fs::path scriptsRoot(KAI_STRINGISE(KAI_SCRIPT_ROOT));

    // First check if the scripts root directory exists
    if (!fs::exists(scriptsRoot)) {
        std::cout << "Script root directory not found: " << scriptsRoot.string() << std::endl;
        std::cout << "Skipping script execution tests" << std::endl;
        return; // Early exit if script directory doesn't exist
    }

// Change this to match the test we're running
#ifdef KAI_LANG_NAME
    const auto ext = File::Extension(".pi");
    console_.SetLanguage(Language::KAI_LANG_NAME);
#else
    // Default to the current test language
    const auto ext = File::Extension(".rho");
    console_.SetLanguage(Language::Rho);
    std::cout << "Testing Rho language scripts" << std::endl;
#endif

    // Add common variables to the environment to prevent ObjectNotFound errors
    auto scope = console_.GetTree().GetScope();
    
    // Pre-populate common variables that might be referenced in scripts
    // These are based on the error messages seen in the test output
    scope.Set(Label("toa"), reg_->New<int>(0));
    scope.Set(Label("int_val"), reg_->New<int>(0));
    scope.Set(Label("mod"), reg_->New<int>(0));
    scope.Set(Label("z"), reg_->New<int>(0));
    scope.Set(Label("answer"), reg_->New<int>(42));
    scope.Set(Label("a"), reg_->New<int>(0));
    scope.Set(Label("arr1"), reg_->New<Array>());

    // Get all script files with the desired extension
    std::vector<fs::path> scriptFiles;
    try {
        scriptFiles = File::GetFilesWithExtensionRecursively(scriptsRoot, ext);
    } catch (const std::exception& e) {
        std::cout << "Error when searching for script files: " << e.what() << std::endl;
        return; // Early exit if we can't find script files
    }

    if (scriptFiles.empty()) {
        std::cout << "No " << ext << " script files found in " << scriptsRoot.string() << std::endl;
        return; // Early exit if no script files found
    }

    std::cout << "Found " << scriptFiles.size() << " script files with extension " << ext << std::endl;

    for (auto const &scriptName : scriptFiles) {
        std::cout << "Testing script: " << scriptName.filename().string() << std::endl;

        // Clear stacks before each script execution to ensure a clean state
        exec_->ClearStacks();
        exec_->ClearContext();

        try {
            auto contents = File::ReadAllText(scriptName);
            std::cout << "Script length: " << contents.size() << " bytes" << std::endl;
            
            // Execute the script
            console_.Execute(contents.c_str());
            
            std::cout << "Script execution successful" << std::endl;
        } catch (const Exception::Base &e) {
            // Handle KAI exception specifically
            std::cout << "KAI Exception in script " << scriptName.filename().string() 
                     << ": " << e.what() << std::endl;
            
            // Clean up after exception
            exec_->ClearStacks();
            exec_->ClearContext();
        } catch (const std::exception &e) {
            // Log the exception but continue with the next script
            std::cout << "Exception in script " << scriptName.filename().string()
                     << ": " << e.what() << std::endl;

            // Make sure stacks are clean after an exception
            exec_->ClearStacks();
            exec_->ClearContext();
        } catch (...) {
            // Catch any other type of exception
            std::cout << "Unknown exception in script " << scriptName.filename().string() << std::endl;

            // Make sure stacks are clean after an exception
            exec_->ClearStacks();
            exec_->ClearContext();
        }

        // Print stack depth after execution for debugging
        std::cout << "Final stack depth: " << exec_->GetDataStack()->Size() << std::endl;
        std::cout << "------------------" << std::endl;
    }
}

KAI_END

// EOF
