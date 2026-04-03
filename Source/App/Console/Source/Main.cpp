#include <getopt.h>

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
    str << KAI_VERSION_MAJOR << '.' << KAI_VERSION_MINOR << '.'
        << KAI_VERSION_PATCH;
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
         << rang::fg::cyan << "Examples:\n"
         << rang::fg::reset << "  " << programName
         << "                    # Interactive Pi mode\n"
         << "  " << programName
         << " -l rho             # Interactive Rho mode\n"
         << "  " << programName << " script.pi          # Execute Pi script\n"
         << "  " << programName
         << " -t 2 script.rho    # Execute with trace level 2\n\n"
         << rang::fg::yellow << "Built-in Commands (in REPL):\n"
         << rang::fg::reset << "  help                    Show help topics\n"
         << "  clear, cls              Clear screen\n"
         << "  exit, quit              Exit console\n"
         << "  pi, rho                 Switch language\n"
         << "  history                 Show command history\n"
         << "  stack                   Show current stack\n";
}

void ShowVersion() {
    cout << "KAI Console v" << KaiVersionString() << "\n"
         << "Built on " << __DATE__ << " at " << __TIME__ << "\n"
         << "\nKAI Language System\n"
         << "Supported languages: Pi, Rho\n";
}

ConsoleOptions ParseArguments(int argc, char** argv) {
    ConsoleOptions options;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"language", required_argument, 0, 'l'},
        {"trace", required_argument, 0, 't'},
        {"interactive", no_argument, 0, 'i'},
        {"non-interactive", no_argument, 0, 'n'},
        {"verbose", no_argument, 0, 0},
        {0, 0, 0, 0}};

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "hvl:t:in", long_options,
                            &option_index)) != -1) {
        switch (c) {
            case 'h':
                options.showHelp = true;
                break;
            case 'v':
                options.showVersion = true;
                break;
            case 'l':
                if (std::string(optarg) == "pi") {
                    options.defaultLanguage = Language::Pi;
                } else if (std::string(optarg) == "rho") {
                    options.defaultLanguage = Language::Rho;
                } else {
                    cerr << rang::fg::red << "Error: " << rang::fg::reset
                         << "Unknown language: " << optarg << "\n"
                         << "Supported languages: pi, rho\n";
                    exit(1);
                }
                break;
            case 't':
                try {
                    options.traceLevel = std::stoi(optarg);
                    if (options.traceLevel < 0 || options.traceLevel > 5) {
                        throw std::out_of_range("");
                    }
                } catch (...) {
                    cerr << rang::fg::red << "Error: " << rang::fg::reset
                         << "Trace level must be 0-5\n";
                    exit(1);
                }
                break;
            case 'i':
                options.interactive = true;
                break;
            case 'n':
                options.interactive = false;
                break;
            case 0:
                if (option_index == 6) options.verbose = true;  // --verbose
                break;
            default:
                cerr << "Use '" << argv[0]
                     << " --help' for usage information.\n";
                exit(1);
        }
    }

    // Handle positional argument (filename)
    if (optind < argc) {
        options.filename = argv[optind];
        if (optind + 1 < argc) {
            cerr << rang::fg::yellow << "Warning: " << rang::fg::reset
                 << "Multiple files specified, using only: " << options.filename
                 << "\n";
        }
    }

    return options;
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
    try {
        // Parse command line arguments
        ConsoleOptions options = ParseArguments(argc, argv);

        // Handle help and version first
        if (options.showHelp) {
            ShowHelp(argv[0]);
            return 0;
        }

        if (options.showVersion) {
            ShowVersion();
            return 0;
        }

        // Create console
        Console console;

        // Set trace levels from options
        Process::trace = options.verbose ? 1 : 0;

        auto executor = console.GetExecutor();
        if (executor.Exists()) {
            executor->SetTraceLevel(options.traceLevel);
        } else {
            cerr << rang::fg::red << "Error: " << rang::fg::reset
                 << "Console failed to initialize properly\n";
            return 1;
        }

        // Show startup banner unless in non-interactive mode
        if (options.interactive || options.filename.empty()) {
            cout << rang::style::bold << "KAI Console v" << KaiVersionString()
                 << rang::style::reset << "\n";
            cout << "Built on " << __DATE__ << " at " << __TIME__ << "\n";
            if (options.verbose) {
                cout << rang::fg::gray << "Trace level: " << options.traceLevel
                     << rang::fg::reset << "\n";
            }
            cout << rang::fg::cyan << "Type 'help' for available commands."
                 << rang::fg::reset << "\n\n";
        }

        // Handle file execution
        if (!options.filename.empty()) {
            // Determine language from file extension or use specified language
            Language lang = options.defaultLanguage;

            if (options.filename.ends_with(".pi")) {
                lang = Language::Pi;
            } else if (options.filename.ends_with(".rho")) {
                lang = Language::Rho;
            } else if (options.defaultLanguage == Language::Pi ||
                       options.defaultLanguage == Language::Rho) {
                // Use specified language for files without standard extensions
                if (options.verbose) {
                    cout << rang::fg::yellow << "Warning: " << rang::fg::reset
                         << "Unknown file extension, using "
                         << (lang == Language::Pi ? "Pi" : "Rho")
                         << " language\n";
                }
            } else {
                cerr << rang::fg::red << "Error: " << rang::fg::reset
                     << "Unknown file extension. Expected .pi or .rho, or "
                        "specify language with -l\n";
                return 1;
            }

            // Check if file exists
            std::ifstream file(options.filename);
            if (!file.good()) {
                cerr << rang::fg::red << "Error: " << rang::fg::reset
                     << "File not found: " << options.filename << "\n";
                return 1;
            }
            file.close();

            // Set up language and translator
            console.SetLanguage(lang);
            auto translator =
                CreateTranslatorForLanguage(console.GetRegistry(), lang);
            if (!translator) {
                cerr << rang::fg::red << "Error: " << rang::fg::reset
                     << "Failed to create translator for "
                     << (lang == Language::Pi ? "Pi" : "Rho") << " language\n";
                return 1;
            }
            console.SetTranslator(translator);

            if (options.verbose) {
                cout << "Executing " << options.filename << " as "
                     << (lang == Language::Pi ? "Pi" : "Rho") << " script...\n";
            }

            // Execute the file
            if (!console.ExecuteFile(options.filename.c_str())) {
                cerr << rang::fg::red << "Error: " << rang::fg::reset
                     << "Failed to execute file: " << options.filename << "\n";
                return 1;
            }

            // If interactive mode requested, continue to REPL after file
            // execution
            if (!options.interactive) {
                return 0;
            }

            cout << "\n"
                 << rang::fg::green << "File execution completed."
                 << rang::fg::reset << " Entering interactive mode...\n\n";
        }

        // Start REPL
        console.SetLanguage(options.defaultLanguage);
        auto translator = CreateTranslatorForLanguage(console.GetRegistry(),
                                                      options.defaultLanguage);
        if (!translator) {
            cerr << rang::fg::red << "Error: " << rang::fg::reset
                 << "Failed to create translator for default language\n";
            return 1;
        }
        console.SetTranslator(translator);

        if (options.verbose) {
            cout << "Starting REPL in "
                 << (options.defaultLanguage == Language::Pi ? "Pi" : "Rho")
                 << " mode...\n";
        }

        return console.Run();

    } catch (const std::exception& e) {
        cerr << rang::fg::red << "Fatal error: " << rang::fg::reset << e.what()
             << "\n";
        return 1;
    } catch (...) {
        cerr << rang::fg::red << "Unknown fatal error occurred"
             << rang::fg::reset << "\n";
        return 1;
    }
}
