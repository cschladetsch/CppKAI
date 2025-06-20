#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "KAI/Console/Console.h"
#include "KAI/Language/Common/TranslatorFactory.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "rang.hpp"
using namespace std;
using namespace kai;

// Register the translators
REGISTER_TRANSLATOR(Language::Pi, PiTranslator)
REGISTER_TRANSLATOR(Language::Rho, RhoTranslator)

std::string KaiVersionString() {
    stringstream str;
    str << KAI_VERSION_MAJOR << '.' << KAI_VERSION_MINOR << '.'
        << KAI_VERSION_PATCH;
    return str.str();
}

std::shared_ptr<TranslatorCommon> CreateTranslatorForLanguage(Registry& reg,
                                                              Language lang) {
    auto translator = TranslatorFactory::Instance().CreateTranslator(lang, reg);
    if (!translator) {
        KAI_TRACE_ERROR() << "Unsupported language: " << static_cast<int>(lang);
    }
    return translator;
}

int main(int argc, char** argv) {
    // Create console here instead of as global
    Console console;

    // the higher the number, the greater the verbosity of debug output for
    // language systems
    Process::trace = 0;

    // the higher the number, the greater the verbosity of debug output for the
    // Executor
    auto executor = console.GetExecutor();
    if (executor.Exists()) {
        executor->SetTraceLevel(0);
    } else {
        std::cerr << "Error: Console failed to initialize properly\n";
        return 1;
    }

    cout << "v0.1" << endl;
    // Check if a file argument was provided
    if (argc > 1) {
        // Execute file as a program
        std::string filename = argv[1];

        // Determine language from file extension
        Language lang;
        if (filename.ends_with(".pi")) {
            lang = Language::Pi;
        } else if (filename.ends_with(".rho")) {
            lang = Language::Rho;
        } else {
            std::cerr << "Unknown file extension. Expected .pi or .rho\n";
            return 1;
        }

        // Set the language and create the appropriate translator
        console.SetLanguage(lang);
        auto translator =
            CreateTranslatorForLanguage(console.GetRegistry(), lang);
        if (!translator) {
            std::cerr << "Failed to create translator for language\n";
            return 1;
        }
        console.SetTranslator(translator);

        // Use ExecuteFile which now supports shell commands
        if (!console.ExecuteFile(filename.c_str())) {
            std::cerr << "Failed to execute file: " << filename << "\n";
            return 1;
        }
        return 0;
    }

    // No file argument - start REPL with Pi as default
    Language defaultLang = Language::Pi;
    console.SetLanguage(defaultLang);
    auto defaultTranslator =
        CreateTranslatorForLanguage(console.GetRegistry(), defaultLang);
    console.SetTranslator(defaultTranslator);

    // start the REPL
    return console.Run();
}
