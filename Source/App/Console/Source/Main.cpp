#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "KAI/Console/Console.h"
#include "KAI/Core/Logger.h"
#include "KAI/Language/Common/TranslatorFactory.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "KAI/Language/Rho/RhoTranslator.h"
#include "rang.hpp"

using namespace std;
using namespace kai;

REGISTER_TRANSLATOR(Language::Pi, PiTranslator)
REGISTER_TRANSLATOR(Language::Rho, RhoTranslator)

struct ConsoleOptions {
    std::string filename;
    Language defaultLanguage = Language::Pi;
    int traceLevel = 0;
    bool showVersion = false;
    bool showHelp = false;
    bool verbose = false;
    bool interactive = true;
};

std::string KaiVersionString() {
    stringstream str;
    str << KAI_VERSION_MAJOR << '.' << KAI_VERSION_MINOR << '.' << KAI_VERSION_PATCH;
    return str.str();
}

void ShowHelp(const char* programName) {
    cout << rang::style::bold << "KAI Console v" << KaiVersionString()
         << rang::style::reset << "\n\n"
         << "Usage: " << programName << " [OPTIONS] [FILE]\n\n"
         << "OPTIONS:\n"
         << "  -h, --help              Show this help message\n"
         << "  -v, --version           Show version information\n"
         << "  -l, --language LANG     Set default language (pi, rho)\n"
         << "  -t, --trace LEVEL       Set trace level (0-5)\n"
         << "  -i, --interactive       Force interactive mode\n"
         << "  -n, --non-interactive   Non-interactive mode\n"
         << "  --verbose               Enable verbose output\n\n"
         << "FILE:\n"
         << "  Script file to execute (.pi or .rho)\n\n"
         << rang::fg::cyan << "Examples:\n" << rang::fg::reset
         << "  " << programName << "                    # Interactive Pi mode\n"
         << "  " << programName << " -l rho             # Interactive Rho mode\n"
         << "  " << programName << " script.pi          # Execute Pi script\n"
         << "  " << programName << " -t 2 script.rho    # Execute with trace level 2\n\n"
         << rang::fg::yellow << "Built-in Commands (in REPL):\n" << rang::fg::reset
         << "  help, clear, exit, quit, pi, rho, history, stack\n";
}

void ShowVersion() {
    cout << "KAI Console v" << KaiVersionString() << "\n"
         << "Built on " << __DATE__ << " at " << __TIME__ << "\n"
         << "Supported languages: Pi, Rho\n";
}

ConsoleOptions ParseArguments(int argc, char** argv) {
    ConsoleOptions options;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        auto next = [&]() -> std::string {
            if (i + 1 < argc) return argv[++i];
            cerr << "Error: " << arg << " requires an argument\n";
            exit(1);
        };

        if (arg == "-h" || arg == "--help") {
            options.showHelp = true;
        } else if (arg == "-v" || arg == "--version") {
            options.showVersion = true;
        } else if (arg == "-i" || arg == "--interactive") {
            options.interactive = true;
        } else if (arg == "-n" || arg == "--non-interactive") {
            options.interactive = false;
        } else if (arg == "--verbose") {
            options.verbose = true;
        } else if (arg == "-l" || arg == "--language") {
            std::string lang = next();
            if (lang == "pi")       options.defaultLanguage = Language::Pi;
            else if (lang == "rho") options.defaultLanguage = Language::Rho;
            else {
                cerr << rang::fg::red << "Error: " << rang::fg::reset
                     << "Unknown language: " << lang << "\n"
                     << "Supported: pi, rho\n";
                exit(1);
            }
        } else if (arg == "-t" || arg == "--trace") {
            std::string val = next();
            try {
                options.traceLevel = std::stoi(val);
                if (options.traceLevel < 0 || options.traceLevel > 5)
                    throw std::out_of_range("");
            } catch (...) {
                cerr << rang::fg::red << "Error: " << rang::fg::reset
                     << "Trace level must be 0-5\n";
                exit(1);
            }
        } else if (arg[0] != '-') {
            if (options.filename.empty()) options.filename = arg;
            else cerr << rang::fg::yellow << "Warning: " << rang::fg::reset
                      << "Multiple files specified, using: " << options.filename << "\n";
        } else {
            cerr << "Unknown option: " << arg << "\n"
                 << "Use --help for usage.\n";
            exit(1);
        }
    }

    return options;
}

std::shared_ptr<TranslatorCommon> CreateTranslatorForLanguage(Registry& reg, Language lang) {
    auto translator = TranslatorFactory::Instance().CreateTranslator(lang, reg);
    if (!translator)
        KAI_TRACE_ERROR() << "Unsupported language: " << static_cast<int>(lang);
    return translator;
}

int main(int argc, char** argv) {
#ifdef _WIN32
    // The Windows console defaults to a legacy codepage (not UTF-8), so
    // UTF-8 literals in source - like the Greek pi prompt symbol - render as
    // mojibake unless we explicitly switch both output and input to UTF-8.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    try {
        Logger::Init();
        Logger::Info("KAI Console starting");

        ConsoleOptions options = ParseArguments(argc, argv);

        if (options.showHelp)    { ShowHelp(argv[0]); return 0; }
        if (options.showVersion) { ShowVersion(); return 0; }

        Console console;
        Process::trace = options.verbose ? 1 : 0;

        auto executor = console.GetExecutor();
        if (executor.Exists()) {
            executor->SetTraceLevel(options.traceLevel);
        } else {
            cerr << rang::fg::red << "Error: " << rang::fg::reset
                 << "Console failed to initialize\n";
            return 1;
        }

        if (options.interactive || options.filename.empty()) {
            cout << rang::style::bold << "KAI Console v" << KaiVersionString()
                 << rang::style::reset << "\n"
                 << "Built on " << __DATE__ << " at " << __TIME__ << "\n";
            if (options.verbose)
                cout << rang::fg::gray << "Trace level: " << options.traceLevel
                     << rang::fg::reset << "\n";
            cout << rang::fg::cyan << "Type 'help' for available commands."
                 << rang::fg::reset << "\n\n";
        }

        if (!options.filename.empty()) {
            Language lang = options.defaultLanguage;
            if      (options.filename.ends_with(".pi"))  lang = Language::Pi;
            else if (options.filename.ends_with(".rho")) lang = Language::Rho;

            std::ifstream file(options.filename);
            if (!file.good()) {
                cerr << rang::fg::red << "Error: " << rang::fg::reset
                     << "File not found: " << options.filename << "\n";
                return 1;
            }
            file.close();

            console.SetLanguage(lang);
            auto translator = CreateTranslatorForLanguage(console.GetRegistry(), lang);
            if (!translator) {
                cerr << rang::fg::red << "Error: " << rang::fg::reset
                     << "Failed to create translator\n";
                return 1;
            }
            console.SetTranslator(translator);

            if (!console.ExecuteFile(options.filename.c_str())) {
                cerr << rang::fg::red << "Error: " << rang::fg::reset
                     << "Failed to execute: " << options.filename << "\n";
                return 1;
            }

            if (!options.interactive) return 0;

            cout << "\n" << rang::fg::green << "File execution completed."
                 << rang::fg::reset << " Entering interactive mode...\n\n";
        }

        console.SetLanguage(options.defaultLanguage);
        auto translator = CreateTranslatorForLanguage(console.GetRegistry(),
                                                      options.defaultLanguage);
        if (!translator) {
            cerr << rang::fg::red << "Error: " << rang::fg::reset
                 << "Failed to create translator\n";
            return 1;
        }
        console.SetTranslator(translator);
        return console.Run();

    } catch (const std::exception& e) {
        cerr << rang::fg::red << "Fatal error: " << rang::fg::reset << e.what() << "\n";
        return 1;
    } catch (...) {
        cerr << rang::fg::red << "Unknown fatal error\n" << rang::fg::reset;
        return 1;
    }
}
