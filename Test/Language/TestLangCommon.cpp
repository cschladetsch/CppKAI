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
    for (auto const &scriptName :
         File::GetFilesWithExtensionRecursively(scriptsRoot, ext)) {
        // Include all scripts, including WIP ones
        // Only print script name in debug mode if needed
        // KAI_TRACE() << "Testing script: " <<
        // scriptName.generic_string().c_str();

        // Clear stacks before each script execution to ensure a clean state
        exec_->ClearStacks();
        exec_->ClearContext();

        auto contents = File::ReadAllText(scriptName);
        try {
            console_.Execute(contents.c_str());
        } catch (std::exception &e) {
            // Log the exception but continue with the next script
            // This ensures one failing script doesn't stop the entire test
            KAI_TRACE() << "Exception in script "
                        << scriptName.generic_string().c_str() << ": "
                        << e.what();

            // Make sure stacks are clean after an exception
            exec_->ClearStacks();
            exec_->ClearContext();
        } catch (...) {
            // Catch any other type of exception
            KAI_TRACE() << "Unknown exception in script "
                        << scriptName.generic_string().c_str();

            // Make sure stacks are clean after an exception
            exec_->ClearStacks();
            exec_->ClearContext();
        }

        // Debug stack state only when needed
        // KAI_TRACE() << "Final stack depth: " <<
        // _exec->GetDataStack()->Size(); KAI_TRACE() << "Final context stack
        // depth: " << _exec->GetContextStack()->Size();
    }
}

KAI_END

// EOF
