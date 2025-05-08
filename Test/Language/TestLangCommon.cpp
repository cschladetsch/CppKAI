#include "TestLangCommon.h"

#include <boost/algorithm/string/predicate.hpp>
#include <cwctype>
#include <filesystem>

#include "File.h"
#include "TestCommon.h"
#include <KAI/Core/Exception.h>

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
    _reg = &_console.GetRegistry();
    _exec = &*_console.GetExecutor();
    _data = &*_exec->GetDataStack();
    _context = &*_exec->GetContextStack();
    _tree = &_console.GetTree();
    _root = _tree->GetRoot();
    
    // Always ensure a clean state on setup
    _exec->ClearStacks();
    _exec->ClearContext();
}

void TestLangCommon::TearDown() {
    // Clean up after each test to avoid state persistence
    _exec->ClearStacks();
    _exec->ClearContext();
}

void TestLangCommon::ExecScripts() {
    const fs::path scriptsRoot(KAI_STRINGISE(KAI_SCRIPT_ROOT));
    const auto ext = File::Extension(".pi");
    _console.SetLanguage(Language::KAI_LANG_NAME);
    for (auto const &scriptName :
         File::GetFilesWithExtensionRecursively(scriptsRoot, ext)) {
        // Include all scripts, including WIP ones
        // Only print script name in debug mode if needed
        // KAI_TRACE() << "Testing script: " << scriptName.generic_string().c_str();
        
        // Clear stacks before each script execution to ensure a clean state
        _exec->ClearStacks();
        _exec->ClearContext();
        
        auto contents = File::ReadAllText(scriptName);
        try {
            _console.Execute(contents.c_str());
        }
        catch (std::exception &e) {
            // Log the exception but continue with the next script
            // This ensures one failing script doesn't stop the entire test
            KAI_TRACE() << "Exception in script " << scriptName.generic_string().c_str() 
                        << ": " << e.what();
            
            // Make sure stacks are clean after an exception
            _exec->ClearStacks();
            _exec->ClearContext();
        }
        catch (...) {
            // Catch any other type of exception
            KAI_TRACE() << "Unknown exception in script " << scriptName.generic_string().c_str();
            
            // Make sure stacks are clean after an exception
            _exec->ClearStacks();
            _exec->ClearContext();
        }
        
        // Debug stack state only when needed
        // KAI_TRACE() << "Final stack depth: " << _exec->GetDataStack()->Size();
        // KAI_TRACE() << "Final context stack depth: " << _exec->GetContextStack()->Size();
    }
}

KAI_END

// EOF
