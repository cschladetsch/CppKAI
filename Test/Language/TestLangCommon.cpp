#include "TestLangCommon.h"

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
        if (boost::algorithm::contains(scriptName.c_str(), "WIP")) {
            KAI_TRACE() << "Skipping script: "
                        << scriptName.generic_string().c_str();
            continue;
        }
        KAI_TRACE() << "Testing script: "
                    << scriptName.generic_string().c_str();
        
        // Clear stacks before each script execution to ensure a clean state
        _exec->ClearStacks();
        _exec->ClearContext();
        
        // Debug: check initial stack state
        KAI_TRACE() << "Initial stack depth: " << _exec->GetDataStack()->Size();
        KAI_TRACE() << "Initial context stack depth: " << _exec->GetContextStack()->Size();
        
        auto contents = File::ReadAllText(scriptName);
        _console.Execute(contents.c_str());
        
        // Debug: check final stack state
        KAI_TRACE() << "Final stack depth: " << _exec->GetDataStack()->Size();
        KAI_TRACE() << "Final context stack depth: " << _exec->GetContextStack()->Size();
    }
}

KAI_END

// EOF
