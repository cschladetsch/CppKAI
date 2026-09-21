// kaicalc - a command-line calculator that doesn't insult you.
//
// Windows' `calc` opens a GUI window and nothing else: no expression
// argument, nothing to pipe into it, no way to script it, no functions, no
// variables. This is the same problem KAI already solved for arbitrary
// Pi (RPN)/Rho (infix) programs, so rather than write a calculator from
// scratch, this is a thin CLI front-end onto KAI's real Console/Executor -
// the same machinery Console (Source/App/Console) uses for full programs,
// pointed at three specific ways of driving it:
//
//   kaicalc 2 + 3 * 4        one-shot: evaluate argv, print the answer, exit
//   echo "sqrt(2)" | kaicalc pipe mode: one expression per line of stdin
//   kaicalc                  interactive REPL (KAI Console, full featured)
//
// Rho (infix, "2 + 3 * 4") is the default so it behaves the way anyone
// typing `calc 2+2` would expect; pass -p/--pi for RPN if, like the author,
// you've used an HP-48 since 1993 and infix feels like a design mistake.

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define KAICALC_ISATTY _isatty
#define KAICALC_FILENO _fileno
#else
#include <unistd.h>
#define KAICALC_ISATTY isatty
#define KAICALC_FILENO fileno
#endif

#include "KAI/Console/Console.h"
#include "KAI/Core/Logger.h"
#include "KAI/Language/Common/TranslatorFactory.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "KAI/Language/Rho/RhoTranslator.h"

using namespace std;
using namespace kai;

REGISTER_TRANSLATOR(Language::Pi, PiTranslator)
REGISTER_TRANSLATOR(Language::Rho, RhoTranslator)

namespace {

std::string KaiVersionString() {
    stringstream s;
    s << KAI_VERSION_MAJOR << '.' << KAI_VERSION_MINOR << '.' << KAI_VERSION_PATCH;
    return s.str();
}

void ShowHelp(const char *programName) {
    cout
        << "kaicalc " << KaiVersionString() << " - a calculator for the command line\n\n"
        << "Windows' `calc` is a GUI window with no expression argument, nothing to\n"
        << "pipe into it, and no scripting. kaicalc is a proper CLI calculator built\n"
        << "on KAI's real Pi/Rho expression evaluator.\n\n"
        << "Usage:\n"
        << "  " << programName << " EXPRESSION...    Evaluate EXPRESSION, print the result, exit\n"
        << "  " << programName << "                   Interactive session\n"
        << "  ... | " << programName << "             One expression per line of stdin\n\n"
        << "Options:\n"
        << "  -p, --pi        Use Pi (RPN/postfix) instead of the default Rho (infix)\n"
        << "  -h, --help      Show this help\n"
        << "  -v, --version   Show version\n\n"
        << "Examples:\n"
        << "  " << programName << " 2 + 3 * 4          -> 14\n"
        << "  " << programName << " -p 2 3 4 * +       -> 14   (RPN: push 2 3 4, multiply, add)\n"
        << "  echo \"2 ^ 10\" | " << programName << "    -> 1024\n\n"
        << "The interactive session is the full KAI Console: expression history,\n"
        << "help, variables, and everything else Console supports - see `help` once\n"
        << "you're in it.\n";
}

bool StdinIsPiped() {
    return !static_cast<bool>(KAICALC_ISATTY(KAICALC_FILENO(stdin)));
}

std::shared_ptr<TranslatorCommon> MakeTranslator(Registry &reg, Language lang) {
    auto translator = TranslatorFactory::Instance().CreateTranslator(lang, reg);
    if (!translator)
        KAI_TRACE_ERROR() << "Unsupported language: " << static_cast<int>(lang);
    return translator;
}

// Evaluate one expression against an already-configured Console and print
// its result - the new top of the data stack - to stdout. Clears the data
// stack afterwards so a sequence of independent expressions (stdin pipe
// mode, or several one-shot args) doesn't accumulate leftovers from
// previous lines: each line of input is its own calculation, not a
// continuation of the last one's stack state.
bool EvaluateAndPrint(Console &console, const std::string &expr) {
    const String error = console.Process(String(expr.c_str()));
    if (!error.StdString().empty()) {
        cerr << error.StdString();
        return false;
    }

    auto executor = console.GetExecutor();
    auto stack = executor->GetDataStack();
    if (!stack.Exists() || stack->Empty()) {
        // Nothing pushed a result (e.g. a bare statement/assignment) - not
        // an error, just nothing to print.
        return true;
    }

    cout << stack->Top().ToString().c_str() << "\n";
    stack->Clear();
    return true;
}

}  // namespace

int main(int argc, char **argv) {
#ifdef _WIN32
    // Match Console's handling: the Windows console defaults to a legacy
    // codepage, so anything past ASCII (error messages, etc.) can come out
    // as mojibake without this.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    Language language = Language::Rho;
    std::vector<std::string> exprArgs;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            ShowHelp(argv[0]);
            return 0;
        }
        if (arg == "-v" || arg == "--version") {
            cout << "kaicalc " << KaiVersionString() << " (built on KAI)\n";
            return 0;
        }
        if (arg == "-p" || arg == "--pi") {
            language = Language::Pi;
            continue;
        }
        exprArgs.push_back(arg);
    }

    try {
        Logger::Init();

        Console console;
        console.SetLanguage(language);

        auto translator = MakeTranslator(console.GetRegistry(), language);
        if (!translator) {
            cerr << "Error: failed to initialize the "
                 << (language == Language::Pi ? "Pi" : "Rho")
                 << " translator\n";
            return 1;
        }
        console.SetTranslator(translator);

        auto executor = console.GetExecutor();
        if (!executor.Exists()) {
            cerr << "Error: console failed to initialize\n";
            return 1;
        }

        if (!exprArgs.empty()) {
            // One-shot mode: `kaicalc 2 + 3 * 4`. Join argv back into a
            // single expression - the shell has already split it on
            // whitespace, and Rho/Pi both parse fine either way.
            std::string expr;
            for (size_t i = 0; i < exprArgs.size(); ++i) {
                if (i) expr += " ";
                expr += exprArgs[i];
            }
            return EvaluateAndPrint(console, expr) ? 0 : 1;
        }

        if (StdinIsPiped()) {
            // Pipe mode: one expression per line of stdin, one result per
            // line of stdout. Blank lines are skipped rather than treated
            // as errors, so trailing newlines in piped input don't produce
            // spurious failures.
            std::string line;
            bool ok = true;
            while (std::getline(cin, line)) {
                if (line.find_first_not_of(" \t\r\n") == std::string::npos)
                    continue;
                ok = EvaluateAndPrint(console, line) && ok;
            }
            return ok ? 0 : 1;
        }

        // Interactive mode: the full KAI Console REPL. History, help,
        // multi-line input handling and everything else Console already
        // does comes along for free - this is not a separate calculator
        // REPL, it's the same one KAI's Console app uses for whole
        // programs, just started with a calculator-friendly banner.
        cout << "kaicalc " << KaiVersionString()
             << " - a calculator that isn't garbage from the CLI\n"
             << "Language: " << (language == Language::Pi ? "Pi (RPN)" : "Rho (infix)")
             << "  (-p for Pi, -h for help)\n\n";
        return console.Run();

    } catch (const std::exception &e) {
        cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        cerr << "Unknown fatal error\n";
        return 1;
    }
}