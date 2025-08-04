#include "KAI/Console/Console.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <thread>

#ifdef __linux__
#include <termios.h>
#include <unistd.h>
#endif

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Memory/StandardAllocator.h"
#include "KAI/Core/Object.h"
#include "KAI/Executor/BinBase.h"
#include "rang.hpp"

using namespace std;

KAI_BEGIN

Console::Console() {
    alloc = make_shared<Memory::StandardAllocator>();
    peer_ = nullptr;
    networkingEnabled_ = false;
    networkRunning_ = false;
    listenPort_ = 14600;
    consoleId_ = GenerateConsoleId();
    Create();
    LoadHistory();
}

Console::Console(shared_ptr<Memory::IAllocator> alloc) {
    this->alloc = alloc;
    peer_ = nullptr;
    networkingEnabled_ = false;
    networkRunning_ = false;
    listenPort_ = 14600;
    consoleId_ = GenerateConsoleId();
    Create();
    LoadHistory();
}

Console::~Console() {
    StopNetworking();
    SaveHistory();
    alloc->DeAllocate(reg_);
}

void Console::Create() {
    try {
        auto result = alloc->Allocate<Registry>(alloc);
        if (!result.has_value()) {
            KAI_TRACE_ERROR() << "Could not allocate Registry";
            return;
        }
        reg_ = result.value();

        RegisterTypes();

        executor = reg_->New<Executor>();
        compiler = reg_->New<Compiler>();

        executor.SetManaged(false);
        compiler.SetManaged(false);

        // Set the compiler reference in the executor
        executor->SetCompiler(compiler);

        CreateTree();

        // Set default language to Pi
        SetLanguage(Language::Pi);
    }
    KAI_CATCH(exception, e) {
        KAI_TRACE_1(e.what());
        std::cerr << "Console::Create::Exception '" << e.what() << "'" << ends;
    }
}

void Console::ExposeTypesToTree(Object types) {
    for (int N = 0; N < Type::Number::Last; ++N) {
        const ClassBase *K = reg_->GetClass(N);
        if (K == 0) continue;
        types.Set(K->GetName(), reg_->New(K));
    }
}

void Console::SetLanguage(Language lang) {
    language = lang;

    // Update the compiler's language
    if (compiler.Exists()) {
        compiler->SetLanguage(static_cast<int>(lang));
    }
}

void Console::SetLanguage(int lang) {
    SetLanguage(static_cast<Language>(lang));
}

void Console::SetTranslator(std::shared_ptr<TranslatorCommon> trans) {
    translator = trans;

    // Set up the compiler's translation function to use our translator
    if (compiler.Exists() && translator) {
        compiler->SetTranslateFunction(
            [this](const String &text, Structure st) -> Pointer<Continuation> {
                return translator->Translate(text.c_str(), st);
            });
    }
}

void Console::ControlC() { executor->ClearContext(); }

void Console::ClearScreen() const {
    // Use ANSI escape sequences to clear screen and move cursor to top
    // \033[2J clears the screen
    // \033[H moves cursor to home position (top-left)
    cout << "\033[2J\033[H" << flush;
}

String Console::ReadLineWithDynamicColor() {
#ifdef __linux__
    // Save current terminal settings
    struct termios old_settings, new_settings;
    tcgetattr(STDIN_FILENO, &old_settings);
    new_settings = old_settings;

    // Set terminal to raw mode
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

    string line;
    bool isShellCommand = false;
    char ch;

    while (true) {
        if (read(STDIN_FILENO, &ch, 1) == 1) {
            if (ch == '\n' || ch == '\r') {
                cout << endl;
                break;
            } else if (ch == 127 || ch == 8) {  // Backspace
                if (!line.empty()) {
                    line.pop_back();
                    cout << "\b \b" << flush;

                    // Check if we need to update color after backspace
                    if (line.empty()) {
                        isShellCommand = false;
                        // Move cursor back to start of line and rewrite
                        cout << "\r";
                        WritePrompt(cout);
                        cout << rang::fg::gray;
                    } else if (line.size() == 1 && line[0] != '$' &&
                               isShellCommand) {
                        isShellCommand = false;
                        // Rewrite entire line with normal color
                        cout << "\r";
                        WritePrompt(cout);
                        cout << rang::fg::gray << line << flush;
                    }
                }
            } else if (ch == 3) {  // Ctrl-C
                // Restore terminal settings
                tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
                cout << rang::fg::reset << endl;
                throw std::runtime_error("Interrupted");
            } else if (ch == 12) {  // Ctrl-L
                ClearScreen();
                WritePrompt(cout);
                if (isShellCommand) {
                    cout << rang::fg::green;
                } else {
                    cout << rang::fg::gray;
                }
                cout << line << flush;
            } else if (isprint(ch)) {
                line += ch;

                // Check if this is the first character and it's '$'
                if (line.size() == 1 && ch == '$') {
                    isShellCommand = true;
                    cout << rang::fg::green << ch << flush;
                } else if (isShellCommand) {
                    cout << ch << flush;
                } else {
                    cout << ch << flush;
                }
            }
        }
    }

    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
    cout << rang::fg::reset;

    return String(line);
#else
    // Fallback to regular getline on non-Linux systems
    string line;
    cout << rang::fg::gray;
    getline(cin, line);
    cout << rang::fg::reset;
    return String(line);
#endif
}

void Console::ExecuteShellCommandWithColor(const std::string &command) {
    // Prepare the command to force color output
    std::string colorCommand = command;

    // Check if this is an ls command and add color flag if not already present
    if (command.find("ls") == 0 &&
        command.find("--color") == std::string::npos) {
        // Check if there are any flags
        size_t spacePos = command.find(' ');
        if (spacePos != std::string::npos) {
            // Insert --color=always after 'ls'
            colorCommand = "ls --color=always" + command.substr(spacePos);
        } else {
            // Just 'ls' with no arguments
            colorCommand = "ls --color=always";
        }
    }

    // For other commands that support color, we might need to add specific
    // flags grep -> --color=always diff -> --color=always gcc/g++ ->
    // -fdiagnostics-color=always

    if (command.find("grep") == 0 &&
        command.find("--color") == std::string::npos) {
        size_t spacePos = command.find(' ');
        if (spacePos != std::string::npos) {
            colorCommand = "grep --color=always" + command.substr(spacePos);
        }
    }

    // Set TERM environment variable and ensure dircolors are loaded
    // This will respect the user's ~/.dircolors if it exists
    std::string fullCommand = "TERM=xterm-256color ";

    // For ls commands, ensure dircolors are properly loaded
    if (command.find("ls") == 0) {
        fullCommand +=
            "eval \"$(dircolors -b ~/.dircolors 2>/dev/null || dircolors -b)\" "
            "&& ";
    }

    fullCommand += colorCommand;

    // Execute the command
    FILE *pipe = popen(fullCommand.c_str(), "r");
    if (pipe) {
        // Use larger buffer to handle ANSI escape sequences properly
        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            cout << buffer;
        }
        int exitCode = pclose(pipe);
        if (exitCode != 0) {
            cout << rang::fg::red << "Command exited with code: " << exitCode
                 << rang::fg::reset << endl;
        }
    } else {
        cout << rang::fg::red << "Failed to execute: " << command
             << rang::fg::reset << endl;
    }
}

Language Console::GetLanguage() const { return language; }

void Console::CreateTree() {
    Object root = reg_->New<void>();
    Object types = reg_->New<void>();
    Object sys = reg_->New<void>();
    Object bin = reg_->New<void>();
    Object home = reg_->New<void>();

    types.SetSwitch(IObject::Managed, false);
    sys.SetSwitch(IObject::Managed, false);
    root.SetSwitch(IObject::Managed, false);
    bin.SetSwitch(IObject::Managed, false);

    home.SetManaged(false);

    tree.SetRoot(root);
    root.Set("Types", types);
    root.Set("Sys", sys);
    root.Set("Bin", bin);
    root.Set("Home", home);

    Set(root, Pathname("/Compiler"), compiler);
    Set(root, Pathname("/Executor"), executor);

    Bin::AddFunctions(bin);
    tree.AddSearchPath(Pathname("/Bin"));
    tree.AddSearchPath(Pathname("/Sys"));
    tree.AddSearchPath(Pathname("/Types"));

    executor->SetTree(&tree);
    reg_->SetTree(tree);

    root.Set("Home", home);
    tree.SetScope(Pathname("/Home"));

    ExposeTypesToTree(types);
}

void Console::Execute(Pointer<Continuation> cont) {
    KAI_TRY {
        // Extra defensive check for necessary objects
        if (!executor.Exists()) {
            KAI_TRACE_ERROR() << "Execute: Null executor - skipping execution";
            return;
        }

        if (!executor->GetDataStack().Exists()) {
            KAI_TRACE_ERROR()
                << "Execute: Null data stack - skipping execution";
            return;
        }

        // Check for null continuation
        if (!cont.Exists()) {
            KAI_TRACE() << "Execute: Null continuation - skipping execution";
            return;
        }

        // Check if the continuation has valid code
        if (!cont->GetCode().Exists()) {
            KAI_TRACE()
                << "Execute: Continuation has no code - skipping execution";
            return;
        }

        // Debug the continuation code to help with diagnosing any issues
        // Removed noisy trace for cleaner Console output

        // For null or empty continuations, nothing to do
        if (cont->GetCode()->Size() == 0) {
            KAI_TRACE() << "Execute: Continuation has empty code array - "
                           "skipping execution";
            return;
        }

        // Set the scope for the continuation if possible
        if (executor->GetTree() != nullptr) {
            cont->SetScope(executor->GetTree()->GetScope());
        }

        // Option 1: Execute the continuation using the standard executor
        if (!cont.Exists()) {
            KAI_TRACE_ERROR()
                << "Execute: Continuation is invalid - skipping execution";
            return;
        }

        // Let exceptions propagate so that Process can catch them
        // Use ContinueOnly to execute this continuation without
        // saving/restoring state
        executor->ContinueOnly(cont);

        // After execution, process the stack to ensure proper type extraction
        Value<Stack> dataStack = executor->GetDataStack();

        // Check if we have a valid stack before processing
        if (!dataStack.Valid() || !dataStack.Exists()) {
            KAI_TRACE_WARN() << "Execute: Invalid data stack after execution";
            return;
        }

        // Process each stack item to extract primitive values from
        // continuations
        int stackSize = dataStack->Size();
        for (int i = 0; i < stackSize; i++) {
            // Get the object at the current position (from the bottom)
            // We want to preserve the original stack order
            // Removed unused variable to avoid null object access
            // We no longer automatically unwrap continuations here
            // Continuations are preserved by design for blocks and Pi {}
            // constructs Test code should use UnwrapStackValues() from
            // TestLangCommon if needed
        }

        // The continuation might have finished, which is normal
        // Don't access continuation properties after execution completes
    }
    KAI_CATCH(Exception::Base, E) {
        KAI_TRACE_ERROR_1(E);
        // Only re-throw assertion failures and similar errors that should be
        // visible to Process
        if (E.ToString().find("Assertion failed") != std::string::npos) {
            throw;
        }
        // For debugging: log stack state when exception occurs
        KAI_TRACE() << "Exception occurred. Stack state:";
        if (executor.Exists() && executor->GetDataStack().Exists()) {
            KAI_TRACE() << "  Stack size: " << executor->GetDataStack()->Size();
        }
    }
    KAI_CATCH(exception, E) {
        KAI_TRACE_ERROR_2("StdException: ", E.what());
        // Don't re-throw standard exceptions unless they're assertion-related
    }
    KAI_CATCH_ALL() {
        KAI_TRACE_ERROR_1("UnknownException");
        // Don't re-throw unknown exceptions
    }
}

void Console::Execute(String const &text, Structure st) {
    // Use the translator if available, otherwise use compiler
    Pointer<Continuation> cont;

    if (translator) {
        cont = translator->Translate(text.c_str(), st);
    } else if (compiler.Exists()) {
        cont = compiler->Translate(text.c_str(), st);
    } else {
        KAI_TRACE_ERROR() << "No translator or compiler available";
        return;
    }

    if (!cont.Exists()) {
        KAI_TRACE_WARN() << "Translation of '" << text
                         << "' yielded invalid continuation";
        return;
    }

    // Log what we're about to execute for debugging purposes
    // Removed noisy trace for cleaner Console output

    // Set the scope on the continuation (important for Store operations)
    cont->SetScope(tree.GetScope());

    // Execute the continuation - let exceptions propagate to Process
    Execute(cont);
}

String Console::ProcessShellCommand(const String &text) {
    // Extract command after the leading backtick
    std::string fullText = text.StdString();

    // Find the closing backtick (optional)
    size_t closingPos = fullText.find('`', 1);

    // Extract command - either between backticks or from first backtick to end
    std::string commandStd;
    if (closingPos != std::string::npos) {
        // Closing backtick found - extract between backticks
        commandStd = fullText.substr(1, closingPos - 1);
    } else {
        // No closing backtick - treat rest of line as command
        commandStd = fullText.substr(1);
    }

    // Trim whitespace from the command
    size_t start = commandStd.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return String("Error: Empty shell command\n");
    }
    size_t end = commandStd.find_last_not_of(" \t\n\r");
    commandStd = commandStd.substr(start, end - start + 1);

    // Use popen to execute the command and capture output
    FILE *pipe = popen(commandStd.c_str(), "r");
    if (!pipe) {
        return String("Error: Failed to execute shell command\n");
    }

    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    int returnCode = pclose(pipe);
    if (returnCode != 0) {
        result +=
            "\nCommand exited with code: " + std::to_string(returnCode) + "\n";
    }

    return String(result);
}

String Console::ExpandShellCommands(const String &text) {
    std::string result = text.StdString();
    std::regex backtick_regex("`([^`]+)`");
    std::smatch match;

    // Keep replacing until no more backtick expressions are found
    while (std::regex_search(result, match, backtick_regex)) {
        std::string command = match[1].str();

        // Execute the command
        FILE *pipe = popen(command.c_str(), "r");
        std::string output;
        if (pipe) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);

            // Remove trailing newline if present
            if (!output.empty() && output.back() == '\n') {
                output.pop_back();
            }
        } else {
            output = "[shell command failed]";
        }

        // Replace the backtick expression with the command output
        result = match.prefix().str() + output + match.suffix().str();
    }

    return String(result);
}

std::vector<std::string> Console::SplitIntoWords(const std::string &text) {
    std::vector<std::string> words;
    std::string current;
    bool inQuotes = false;
    char quoteChar = '\0';

    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];

        if (!inQuotes && (c == '"' || c == '\'')) {
            inQuotes = true;
            quoteChar = c;
            current += c;
        } else if (inQuotes && c == quoteChar) {
            inQuotes = false;
            current += c;
        } else if (!inQuotes && std::isspace(c)) {
            if (!current.empty()) {
                words.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        words.push_back(current);
    }

    return words;
}

String Console::ApplyWordDesignators(const std::string &command,
                                     const std::string &designators) {
    auto words = SplitIntoWords(command);
    if (words.empty()) return String("");

    std::vector<std::string> result;

    // Parse word designators
    if (designators.empty()) {
        return String(command);
    }

    // Handle special designators
    if (designators == "^") {
        // First argument (word 1)
        if (words.size() > 1) result.push_back(words[1]);
    } else if (designators == "$") {
        // Last argument
        if (!words.empty()) result.push_back(words.back());
    } else if (designators == "*") {
        // All arguments (words 1 to end)
        for (size_t i = 1; i < words.size(); ++i) {
            result.push_back(words[i]);
        }
    } else if (designators.find('*') != std::string::npos) {
        // Handle n* (from word n to end)
        std::string startStr = designators.substr(0, designators.find('*'));
        size_t start = std::stoul(startStr);
        for (size_t i = start; i < words.size(); ++i) {
            result.push_back(words[i]);
        }
    } else if (designators.find('-') != std::string::npos) {
        // Range like n-m
        size_t dashPos = designators.find('-');
        std::string startStr = designators.substr(0, dashPos);
        std::string endStr = designators.substr(dashPos + 1);

        size_t start = startStr.empty() ? 0 : std::stoul(startStr);
        size_t end = endStr == "$"   ? words.size() - 1
                     : endStr == "*" ? words.size() - 1
                                     : std::stoul(endStr);

        for (size_t i = start; i <= end && i < words.size(); ++i) {
            result.push_back(words[i]);
        }
    } else if (std::isdigit(designators[0])) {
        // Single word number
        size_t n = std::stoul(designators);
        if (n < words.size()) {
            result.push_back(words[n]);
        }
    }

    // Join results with spaces
    std::string joined;
    for (size_t i = 0; i < result.size(); ++i) {
        if (i > 0) joined += " ";
        joined += result[i];
    }

    return String(joined);
}

String Console::ApplyModifiers(const String &text,
                               const std::string &modifiers) {
    String result = text;
    std::string currentText = result.StdString();

    // Process each modifier in sequence
    for (size_t i = 0; i < modifiers.length(); ++i) {
        char mod = modifiers[i];

        // Handle substitution modifiers
        if (mod == 's' && i + 1 < modifiers.length() &&
            modifiers[i + 1] == '/') {
            // Find the substitution pattern s/old/new/
            size_t start = i + 2;
            size_t mid = modifiers.find('/', start);
            size_t end = modifiers.find('/', mid + 1);

            if (mid != std::string::npos && end != std::string::npos) {
                std::string oldStr = modifiers.substr(start, mid - start);
                std::string newStr = modifiers.substr(mid + 1, end - mid - 1);

                // Perform substitution
                size_t pos = currentText.find(oldStr);
                if (pos != std::string::npos) {
                    currentText.replace(pos, oldStr.length(), newStr);
                }

                i = end;  // Skip past the substitution
                continue;
            }
        }

        // Handle global substitution gs/old/new/
        if (mod == 'g' && i + 2 < modifiers.length() &&
            modifiers[i + 1] == 's' && modifiers[i + 2] == '/') {
            size_t start = i + 3;
            size_t mid = modifiers.find('/', start);
            size_t end = modifiers.find('/', mid + 1);

            if (mid != std::string::npos && end != std::string::npos) {
                std::string oldStr = modifiers.substr(start, mid - start);
                std::string newStr = modifiers.substr(mid + 1, end - mid - 1);

                // Global substitution
                size_t pos = 0;
                while ((pos = currentText.find(oldStr, pos)) !=
                       std::string::npos) {
                    currentText.replace(pos, oldStr.length(), newStr);
                    pos += newStr.length();
                }

                i = end;  // Skip past the substitution
                continue;
            }
        }

        // Single character modifiers
        currentText =
            ProcessHistoryModifier(String(currentText), mod).StdString();
    }

    return String(currentText);
}

String Console::ProcessHistoryModifier(const String &text, char modifier) {
    std::string str = text.StdString();

    switch (modifier) {
        case 'h': {  // Head - remove last pathname component
            size_t lastSlash = str.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                return String(str.substr(0, lastSlash));
            }
            return String(".");
        }

        case 't': {  // Tail - remove all but last pathname component
            size_t lastSlash = str.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                return String(str.substr(lastSlash + 1));
            }
            return text;
        }

        case 'r': {  // Root - remove extension
            size_t lastDot = str.find_last_of('.');
            size_t lastSlash = str.find_last_of("/\\");
            if (lastDot != std::string::npos &&
                (lastSlash == std::string::npos || lastDot > lastSlash)) {
                return String(str.substr(0, lastDot));
            }
            return text;
        }

        case 'e': {  // Extension - remove all but extension
            size_t lastDot = str.find_last_of('.');
            size_t lastSlash = str.find_last_of("/\\");
            if (lastDot != std::string::npos &&
                (lastSlash == std::string::npos || lastDot > lastSlash)) {
                return String(str.substr(lastDot + 1));
            }
            return String("");
        }

        case 'p': {  // Print - just print but don't execute
            std::cout << str << std::endl;
            return String("");
        }

        case 'q': {  // Quote - quote the substituted words
            return String("\"" + str + "\"");
        }

        case 'x': {  // Quote each word separately
            auto words = SplitIntoWords(str);
            std::string result;
            for (size_t i = 0; i < words.size(); ++i) {
                if (i > 0) result += " ";
                result += "\"" + words[i] + "\"";
            }
            return String(result);
        }

        case 'u': {  // Uppercase
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
            return String(str);
        }

        case 'l': {  // Lowercase
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            return String(str);
        }

        default:
            return text;
    }
}

String Console::ParseHistoryExpansion(const String &text) {
    std::string expansion = text.StdString();
    if (expansion.empty() || expansion[0] != '!') {
        return text;
    }

    // Find the event designator and word designators
    size_t colonPos = expansion.find(':');
    std::string eventPart = colonPos != std::string::npos
                                ? expansion.substr(0, colonPos)
                                : expansion;
    std::string wordPart =
        colonPos != std::string::npos ? expansion.substr(colonPos + 1) : "";

    // Get the command from history using the event part
    String historicalCommand = ProcessZshCommand(String(eventPart));
    if (historicalCommand.size() == 0) {
        return String("");
    }

    // Apply word designators if present
    if (!wordPart.empty()) {
        // Check for modifiers (they come after another colon)
        size_t modifierPos = wordPart.find(':');
        std::string designators = modifierPos != std::string::npos
                                      ? wordPart.substr(0, modifierPos)
                                      : wordPart;
        std::string modifiers = modifierPos != std::string::npos
                                    ? wordPart.substr(modifierPos + 1)
                                    : "";

        String result =
            ApplyWordDesignators(historicalCommand.StdString(), designators);
        if (!modifiers.empty()) {
            result = ApplyModifiers(result, modifiers);
        }
        return result;
    }

    return historicalCommand;
}

String Console::ProcessQuickSubstitution(const String &text) {
    std::string cmd = text.StdString();

    // Check for ^old^new^ or ^old^new pattern
    if (cmd.length() > 2 && cmd[0] == '^') {
        size_t secondCaret = cmd.find('^', 1);
        if (secondCaret != std::string::npos) {
            std::string oldStr = cmd.substr(1, secondCaret - 1);

            // Find the end of the new string
            size_t thirdCaret = cmd.find('^', secondCaret + 1);
            std::string newStr;
            if (thirdCaret != std::string::npos) {
                newStr =
                    cmd.substr(secondCaret + 1, thirdCaret - secondCaret - 1);
            } else {
                newStr = cmd.substr(secondCaret + 1);
            }

            // Apply to last command
            if (!commandHistory.empty()) {
                std::string lastCmd = commandHistory.back();
                size_t pos = lastCmd.find(oldStr);
                if (pos != std::string::npos) {
                    lastCmd.replace(pos, oldStr.length(), newStr);
                    return String(lastCmd);
                }
            }
        }
    }

    return String("");
}

String Console::SearchHistoryAnywhere(const String &pattern) {
    std::string searchStr = pattern.StdString();

    // Search backwards through history for pattern anywhere in command
    for (auto it = commandHistory.rbegin(); it != commandHistory.rend(); ++it) {
        if (it->find(searchStr) != std::string::npos) {
            return String(*it);
        }
    }

    return String("");
}

String Console::ProcessZshCommand(const String &text) {
    // Check if this is a zsh-like command starting with !
    std::string cmd = text.StdString();
    if (cmd.empty() || cmd[0] != '!') {
        return text;
    }

    // Handle !$ - last argument of previous command
    if (cmd == "!$") {
        if (!commandHistory.empty()) {
            auto words = SplitIntoWords(commandHistory.back());
            if (!words.empty()) {
                return String(words.back());
            }
        }
        return String("");
    }

    // Handle !^ - first argument of previous command
    if (cmd == "!^") {
        if (!commandHistory.empty()) {
            auto words = SplitIntoWords(commandHistory.back());
            if (words.size() > 1) {
                return String(words[1]);
            }
        }
        return String("");
    }

    // Handle !?string? - search anywhere in command
    if (cmd.length() > 2 && cmd[1] == '?') {
        size_t endPos = cmd.find('?', 2);
        if (endPos != std::string::npos) {
            std::string searchStr = cmd.substr(2, endPos - 2);
            return SearchHistoryAnywhere(String(searchStr));
        }
    }

    // Handle !# - reference to current command line (before the !#)
    if (cmd == "!#") {
        return String(currentCommand);
    }

    // Check if it contains word designators
    if (cmd.find(':') != std::string::npos) {
        return ParseHistoryExpansion(text);
    }

    // Handle !! - repeat last command
    if (cmd == "!!" || cmd.substr(0, 2) == "!!") {
        if (commandHistory.empty()) {
            return String("");
        }
        return String(commandHistory.back());
    }

    // Handle !n - execute nth command from history
    if (cmd.size() > 1 && std::isdigit(cmd[1])) {
        size_t endPos = 1;
        while (endPos < cmd.size() && std::isdigit(cmd[endPos])) {
            endPos++;
        }
        size_t n = std::stoul(cmd.substr(1, endPos - 1));
        if (n > 0 && n <= commandHistory.size()) {
            return String(commandHistory[n - 1]);
        }
        return String("");
    }

    // Handle !-n - execute nth command from end
    if (cmd.size() > 2 && cmd[1] == '-' && std::isdigit(cmd[2])) {
        size_t endPos = 2;
        while (endPos < cmd.size() && std::isdigit(cmd[endPos])) {
            endPos++;
        }
        size_t n = std::stoul(cmd.substr(2, endPos - 2));
        if (n > 0 && n <= commandHistory.size()) {
            return String(commandHistory[commandHistory.size() - n]);
        }
        return String("");
    }

    // Handle !string - execute last command starting with string
    if (cmd.size() > 1) {
        std::string searchStr = cmd.substr(1);
        // Search backwards through history
        for (auto it = commandHistory.rbegin(); it != commandHistory.rend();
             ++it) {
            if (it->substr(0, searchStr.size()) == searchStr) {
                return String(*it);
            }
        }
        return String("");
    }

    return text;
}

String Console::ExpandHistoryReferences(const String &text) {
    std::string result = text.StdString();

    // First handle !# references (current command line)
    std::regex currentCmd_regex("!#(:[^\\s]+)?");
    std::smatch currentMatch;
    while (std::regex_search(result, currentMatch, currentCmd_regex)) {
        std::string histRef = currentMatch[0].str();
        String expanded = ProcessZshCommand(String(histRef));

        if (expanded.size() > 0) {
            result = currentMatch.prefix().str() + expanded.StdString() +
                     currentMatch.suffix().str();
        } else {
            break;
        }
    }

    // Handle !$ and !^ shortcuts
    std::regex shortcuts_regex("![$^]");
    std::smatch shortcutMatch;
    while (std::regex_search(result, shortcutMatch, shortcuts_regex)) {
        std::string histRef = shortcutMatch[0].str();
        String expanded = ProcessZshCommand(String(histRef));

        if (expanded.size() > 0) {
            result = shortcutMatch.prefix().str() + expanded.StdString() +
                     shortcutMatch.suffix().str();
        } else {
            break;
        }
    }

    // Handle !?string? patterns
    std::regex search_regex("!\\?([^?]+)\\?");
    std::smatch searchMatch;
    while (std::regex_search(result, searchMatch, search_regex)) {
        std::string searchStr = searchMatch[1].str();
        String expanded = SearchHistoryAnywhere(String(searchStr));

        if (expanded.size() > 0) {
            result = searchMatch.prefix().str() + expanded.StdString() +
                     searchMatch.suffix().str();
        } else {
            break;
        }
    }

    // More comprehensive regex to match other history expansions
    // This matches patterns like !!, !n, !-n, !string, and also !n:word,
    // !-n:word, etc.
    std::regex history_regex("!(-?\\d+|!|[^\\s:?$^]+)(:[^\\s]+)?");
    std::smatch match;

    while (std::regex_search(result, match, history_regex)) {
        std::string histRef = match[0].str();
        String expanded = ProcessZshCommand(String(histRef));

        if (expanded.size() > 0) {
            result = match.prefix().str() + expanded.StdString() +
                     match.suffix().str();
        } else {
            // No match found, break to avoid infinite loop
            break;
        }
    }

    return String(result);
}

String Console::Process(const String &text) {
    StringStream result;
    std::string textStr = text.StdString();

    // Trim whitespace
    size_t start = textStr.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return String("");  // Empty or whitespace-only input
    }
    std::string trimmed = textStr.substr(start);
    size_t end = trimmed.find_last_not_of(" \t\n\r");
    if (end != std::string::npos) {
        trimmed = trimmed.substr(0, end + 1);
    }

    // Check for built-in commands first
    if (ProcessBuiltinCommand(trimmed)) {
        return String("");  // Built-in command handled
    }

    // Check if this is a shell command (starts with '$')
    if (!trimmed.empty() && trimmed[0] == '$') {
        // Execute as shell command (strip the $ and any leading space)
        std::string shellCmd = trimmed.substr(1);
        // Trim leading whitespace
        size_t firstNonSpace = shellCmd.find_first_not_of(" \t");
        if (firstNonSpace != std::string::npos) {
            shellCmd = shellCmd.substr(firstNonSpace);
        }

        // Execute the shell command
        FILE *pipe = popen(shellCmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result << buffer;
            }
            int exitCode = pclose(pipe);
            if (exitCode != 0) {
                result << "Command exited with code: " << exitCode << "\n";
            }
        } else {
            result << "Failed to execute: " << shellCmd << "\n";
        }

        return result.ToString();
    }

    // Not a shell command, process as language code
    KAI_TRY {
        // First expand any backtick shell commands
        String expandedText = ExpandShellCommands(text);

        // Determine the appropriate structure based on the content
        Structure structure = Structure::Expression;

        // For Rho, check if this looks like a statement (contains control
        // structures)
        if (language == Language::Rho) {
            std::string str = expandedText.StdString();
            if (str.find("for") != std::string::npos ||
                str.find("while") != std::string::npos ||
                str.find("do") != std::string::npos ||
                str.find("if") != std::string::npos) {
                structure = Structure::Statement;
            }
        }

        // Translate the text into a continuation
        Pointer<Continuation> cont;
        if (translator) {
            cont = translator->Translate(expandedText.c_str(), structure);
        } else if (compiler.Exists()) {
            cont = compiler->Translate(expandedText.c_str(), structure);
        }

        if (cont.Exists()) {
            // Set the scope
            cont->SetScope(tree.GetScope());

            // Execute the continuation using our improved Execute method
            Execute(cont);
        }

        return "";
    }
    KAI_CATCH(Exception::Base, E) {
        result << "Exception: " << E.ToString() << "\n";
    }
    KAI_CATCH(exception, E) { result << "StdException: " << E.what() << "\n"; }
    KAI_CATCH_ALL() { result << "UnknownException: " << "\n"; }
    return result.ToString();
}

void Console::WritePrompt(ostream &out) const {
    // Use colorful prompt with lambda symbol
    if (shellMode) {
        // Shell mode prompt
        out << rang::style::bold << rang::fg::green << "Bash"
            << rang::fg::yellow << " λ " << rang::fg::reset
            << rang::style::bold;
    } else {
        // Normal Pi/Rho prompt
        out << rang::style::bold << rang::fg::cyan
            << ToString(static_cast<Language>(compiler->GetLanguage()))
            << rang::fg::yellow << " λ " << rang::fg::reset
            << rang::style::bold;
    }
    out.flush();  // Ensure prompt is displayed immediately
}

String Console::GetPrompt() const {
    StringStream prompt;
    prompt << ConsoleColor::LanguageName
           << ToString(static_cast<Language>(compiler->GetLanguage()))
           << ConsoleColor::Pathname
           << GetFullname(GetTree().GetScope()).ToString().c_str()
           << ConsoleColor::Input << "> ";

    return prompt.ToString();
}

void Console::ShowColoredStack() const {
    const Value<const Stack> data = executor->GetDataStack();
    if (!data.Exists() || data->Size() == 0) {
        return;  // Don't show anything for empty stack
    }

    auto A = data->Begin(), B = data->End();
    int N = 0;
    for (N = data->Size() - 1; A != B; ++A, --N) {
        // Colored output: [N] in bright yellow/orange, content in white/colored
        // by type
        cout << rang::fgB::yellow << "[" << N << "]: " << rang::fg::reset;

        const bool is_string = A->GetTypeNumber() == Type::Number::String;
        const bool is_int = A->GetTypeNumber() == Type::Number::Signed32;
        const bool is_float = A->GetTypeNumber() == Type::Number::Single;

        String objStr = A->ToString();

        if (is_string) {
            cout << rang::fg::green << "\"" << objStr.c_str() << "\""
                 << rang::fg::reset;
        } else if (is_int) {
            cout << rang::fg::yellow << objStr.c_str() << rang::fg::reset;
        } else if (is_float) {
            cout << rang::fg::magenta << objStr.c_str() << rang::fg::reset;
        } else {
            cout << rang::fg::gray << objStr.c_str() << rang::fg::reset;
        }

        cout << endl;
    }
}

String Console::WriteStack() const {
    const Value<const Stack> data = executor->GetDataStack();
    auto A = data->Begin(), B = data->End();
    StringStream result;
    for (int N = 0; A != B; ++A) {
        result << "[" << N << "] ";
        const bool is_string = A->GetTypeNumber() == Type::Number::String;
        if (is_string) result << "\"";

        result << *A;
        if (is_string) result << "\"";

        result << "\n";
    }

    return result.ToString();
}

int Console::Run() {
    // Enable bold formatting at the start and maintain it
    cout << rang::style::bold;
    cout.flush();

    for (;;) {
        KAI_TRY {
            for (;;) {
                // Always show prompt before input
                WritePrompt(cout);

                // Get input with dynamic color
                string text;
                try {
                    text = ReadLineWithDynamicColor().StdString();
                } catch (const std::runtime_error &e) {
                    // Handle Ctrl-C
                    cout << rang::fg::reset << endl;
                    continue;
                }

                // Store current command for !# support
                currentCommand = text;

                // Check if we need to accumulate multi-line input
                String accumulatedInput(text);
                while (IsStructureIncomplete(accumulatedInput)) {
                    // Show continuation prompt
                    cout << rang::style::bold;
                    cout << ToString(language) << " ... ";

                    string continuationLine;
                    try {
                        continuationLine =
                            ReadLineWithDynamicColor().StdString();
                    } catch (const std::runtime_error &e) {
                        // Handle Ctrl-C during multi-line input
                        cout << rang::fg::reset << endl;
                        break;
                    }

                    // Append the continuation line
                    accumulatedInput =
                        accumulatedInput + "\n" + continuationLine;
                }

                // Update text with the full accumulated input
                text = accumulatedInput.StdString();

                // Commands starting with $ are shell commands
                if (!text.empty() && text[0] == '$') {
                    // Execute as shell command (strip the $ and any leading
                    // space)
                    std::string shellCmd = text.substr(1);
                    // Trim leading whitespace
                    size_t firstNonSpace = shellCmd.find_first_not_of(" \t");
                    if (firstNonSpace != std::string::npos) {
                        shellCmd = shellCmd.substr(firstNonSpace);
                    }

                    // Execute the shell command with color support
                    ExecuteShellCommandWithColor(shellCmd);

                    // Add to history
                    AddToHistory(text);
                } else if (!text.empty() && text[0] == '^') {
                    // Handle quick substitution ^old^new^
                    String substituted = ProcessQuickSubstitution(String(text));
                    if (substituted.size() > 0) {
                        cout << rang::fg::cyan << "=> "
                             << substituted.StdString() << rang::fg::reset
                             << endl;

                        // Process the substituted command
                        String expandedText = ExpandShellCommands(substituted);
                        String output = Process(expandedText);
                        cout << output.c_str();

                        // Add the substituted command to history
                        AddToHistory(substituted.StdString());
                    } else {
                        cout << rang::fg::red
                             << "Substitution failed: no match found"
                             << rang::fg::reset << endl;
                    }
                } else if (!text.empty()) {
                    // Check for clear screen command
                    if (text == "clear" || text == "cls") {
                        ClearScreen();
                        continue;
                    }

                    // Check for shell mode toggle
                    if (text == "sh" || text == "bash" || text == "zsh") {
                        shellMode = !shellMode;
                        if (shellMode) {
                            cout << rang::fg::yellow
                                 << "Entering shell mode. Type 'exit' to "
                                    "return to "
                                 << ToString(static_cast<Language>(
                                        compiler->GetLanguage()))
                                 << " mode." << rang::fg::reset << endl;
                        }
                        continue;
                    }

                    // In shell mode, execute everything as shell commands
                    if (shellMode) {
                        if (text == "exit") {
                            shellMode = false;
                            cout << rang::fg::yellow
                                 << "Exited shell mode. Back to "
                                 << ToString(static_cast<Language>(
                                        compiler->GetLanguage()))
                                 << " mode." << rang::fg::reset << endl;
                            continue;
                        }

                        // Apply history expansion even in shell mode
                        std::string expandedCmd = text;

                        // Check for quick substitution
                        if (text[0] == '^') {
                            String substituted =
                                ProcessQuickSubstitution(String(text));
                            if (substituted.size() > 0) {
                                expandedCmd = substituted.StdString();
                                cout << rang::fg::cyan << "=> " << expandedCmd
                                     << rang::fg::reset << endl;
                            }
                        } else {
                            // Expand history references
                            expandedCmd = ExpandHistoryReferences(String(text))
                                              .StdString();
                            if (expandedCmd != text) {
                                cout << rang::fg::cyan << "=> " << expandedCmd
                                     << rang::fg::reset << endl;
                            }
                        }

                        // Execute as shell command with color support
                        ExecuteShellCommandWithColor(expandedCmd);

                        // Add to history
                        AddToHistory(text);
                        continue;
                    }

                    // Check for exit command in normal mode
                    if (text == "exit" || text == "quit") {
                        end_ = true;
                        endCode_ = 0;
                        continue;
                    }

                    // Check for language switch commands
                    if (text == "pi") {
                        SetLanguage(Language::Pi);
                        // Note: The main application should handle translator
                        // switching For now, just set the language
                        continue;
                    } else if (text == "rho") {
                        SetLanguage(Language::Rho);
                        // Note: The main application should handle translator
                        // switching For now, just set the language
                        continue;
                    }

                    // Check for network commands
                    if (!text.empty() && text[0] == '/') {
                        String result = ProcessNetworkCommand(String(text));
                        if (!result.Empty()) {
                            cout << result.c_str() << endl;
                        }
                        continue;
                    }

                    // Check for zsh-like history commands first
                    std::string processedText = text;

                    // If it's a pure history command (just !!, !n, etc), expand
                    // it
                    if (text[0] == '!' && text.find(' ') == std::string::npos) {
                        String expanded = ProcessZshCommand(String(text));
                        if (expanded.size() > 0) {
                            processedText = expanded.StdString();
                            // Show what command is being executed
                            cout << rang::fg::cyan << "=> " << processedText
                                 << rang::fg::reset << endl;
                        } else {
                            cout << rang::fg::red
                                 << "No matching command in history"
                                 << rang::fg::reset << endl;
                            continue;
                        }
                    } else {
                        // Expand any history references within the command
                        processedText =
                            ExpandHistoryReferences(String(text)).StdString();
                    }

                    // Add original command to history before processing
                    AddToHistory(text);

                    // Check for shell commands
                    if (!processedText.empty() && processedText[0] == '`') {
                        String output =
                            ProcessShellCommand(String(processedText));
                        cout << output.c_str();
                    } else {
                        // Expand any embedded shell commands first
                        String expandedText =
                            ExpandShellCommands(String(processedText));
                        String output = Process(expandedText);
                        cout << output.c_str();
                    }
                }

                // Always show the stack after processing (unless it's empty)
                if (executor.Exists() && executor->GetDataStack().Exists()) {
                    ShowColoredStack();
                }

                if (end_) return endCode_;
            }
        }
        KAI_CATCH(Exception::Base, E) {
            // Use rang for formatting, keeping bold
            cout << rang::style::bold << rang::fg::red;
            KAI_TRACE_ERROR_1(E);
            // Reset color but maintain bold
            cout << rang::style::bold << rang::fg::reset;
        }
        KAI_CATCH(exception, E) {
            cout << rang::style::bold << rang::fg::red;
            KAI_TRACE_ERROR_1(E.what());
            cout << rang::style::bold << rang::fg::reset;
        }
        KAI_CATCH_ALL() {
            cout << rang::style::bold << rang::fg::red;
            KAI_TRACE_ERROR() << " something went wrong";
            cout << rang::style::bold << rang::fg::reset;
        }
    }
}

void Console::RegisterTypes() {
    // built-ins
    reg_->AddClass<const ClassBase *>(Label("Class"));  // TODO: add methods_
    reg_->AddClass<void>(Label("Void"));
    reg_->AddClass<bool>(Label("Bool"));
    reg_->AddClass<int>(Label("Int"));
    reg_->AddClass<float>(Label("Float"));
    reg_->AddClass<Vector3>(Label("Vector3"));
    reg_->AddClass<Vector4>(Label("Vector4"));

    // system types
    // ObjectSet::Register(*registry);
    String::Register(*reg_);
    Object::Register(*reg_);
    Handle::Register(*reg_);
    Stack::Register(*reg_);
    Continuation::Register(*reg_);
    Label::Register(*reg_);
    Operation::Register(*reg_);
    Compiler::Register(*reg_);
    Executor::Register(*reg_);
    Pathname::Register(*reg_);
    BasePointerBase::Register(*reg_);
    Pair::Register(*reg_);
    FunctionBase::Register(*reg_);
    BasePointer<MethodBase>::Register(*reg_);
    BasePointer<PropertyBase>::Register(*reg_);
    BinaryStream::Register(*reg_);
    StringStream::Register(*reg_);
    Array::Register(*reg_);
    List::Register(*reg_);
    Map::Register(*reg_, "Map");

    // TODO: remove less-than comparable trait for hash maps:
    // HashMap::Register(*registry, "HashMap");

#ifdef KAI_UNIT_TESTS
    registry->AddClass<Test::IOutput *>("TestOutputBase");
    Test::Summary::Register(*registry);
    Test::Module::Register(*registry, "TestModule");
    Test::BasicOutput::Register(*registry);
    Test::XmlOutput::Register(*registry);
#endif
}

Pointer<Continuation> Console::Compile(const char *text, Structure st) {
    if (translator) {
        return translator->Translate(text, st);
    } else if (compiler.Exists()) {
        return compiler->Translate(text, st);
    }
    return Object();
}

void Console::Register(Registry &) {}

bool Console::IsStructureIncomplete(const String &text) const {
    // For Rho language, check if we have unmatched braces
    if (language == Language::Rho) {
        int braceCount = 0;
        bool inString = false;
        char stringChar = '\0';

        for (char c : text.StdString()) {
            // Handle string literals
            if ((c == '"' || c == '\'') && !inString) {
                inString = true;
                stringChar = c;
            } else if (inString && c == stringChar) {
                inString = false;
            } else if (!inString) {
                // Count braces outside of strings
                if (c == '{') {
                    braceCount++;
                } else if (c == '}') {
                    braceCount--;
                }
            }
        }

        // Structure is incomplete if we have unmatched opening braces
        return braceCount > 0;
    }

    // For other languages, we don't have multi-line structures
    return false;
}

bool Console::ExecuteFile(const char *fileName) {
    // Validate inputs first
    if (fileName == nullptr || strlen(fileName) == 0) {
        KAI_TRACE_ERROR() << "ExecuteFile: Null or empty filename";
        return false;
    }

    if (!compiler.Exists()) {
        KAI_TRACE_ERROR() << "ExecuteFile: Null compiler";
        return false;
    }

    // Open the file
    std::ifstream file(fileName);
    if (!file.is_open()) {
        KAI_TRACE_ERROR() << "ExecuteFile: Failed to open " << fileName;
        return false;
    }

    // Process the file line by line to support shell commands
    std::string line;
    std::string accumulated;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        // Skip empty lines and comments
        if (line.empty() || (line.size() > 0 && line[0] == '#')) {
            continue;
        }

        // Check if this is a shell command
        if (!line.empty() && line[0] == '$') {
            // If we have accumulated code, execute it first
            if (!accumulated.empty()) {
                String result = Process(String(accumulated));
                if (!result.Empty()) {
                    std::cout << result.c_str();
                }
                accumulated.clear();
            }

            // Execute the shell command
            String result = Process(String(line));
            if (!result.Empty()) {
                std::cout << result.c_str();
            }
        } else {
            // Accumulate language code
            accumulated += line + "\n";
        }
    }

    // Execute any remaining accumulated code
    if (!accumulated.empty()) {
        String result = Process(String(accumulated));
        if (!result.Empty()) {
            std::cout << result.c_str();
        }
    }

    file.close();
    return true;
}

String Console::ProcessSubstitutionModifier(const String &text,
                                            const std::string &pattern) {
    // This is handled in ApplyModifiers for s/ and gs/ patterns
    return text;
}

std::string Console::ExtractFilePath(const std::string &text) {
    // Simple extraction - finds first path-like string
    std::regex pathRegex("([/\\\\]?[\\w.-]+(?:[/\\\\][\\w.-]+)*)");
    std::smatch match;
    if (std::regex_search(text, match, pathRegex)) {
        return match[1].str();
    }
    return "";
}

// Help System Implementation
bool Console::ProcessBuiltinCommand(const std::string &command) {
    // Convert to lowercase for case-insensitive matching
    std::string cmd = command;
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    if (cmd == "help" || cmd == "?") {
        ShowHelp();
        return true;
    }

    // Check for help with topic
    if (cmd.substr(0, 5) == "help ") {
        std::string topic = command.substr(5);
        ShowHelp(topic);
        return true;
    }

    if (cmd == "clear" || cmd == "cls") {
        ClearScreen();
        return true;
    }

    if (cmd == "exit" || cmd == "quit") {
        end_ = true;
        return true;
    }

    if (cmd == "history") {
        cout << rang::style::bold << "Command History:" << rang::style::reset
             << "\n";
        for (size_t i = 0; i < commandHistory.size(); ++i) {
            cout << rang::fg::cyan << "  " << (i + 1) << ": " << rang::fg::reset
                 << commandHistory[i] << "\n";
        }
        return true;
    }

    if (cmd == "stack") {
        ShowColoredStack();
        return true;
    }

    if (cmd == "pi") {
        SetLanguage(Language::Pi);
        cout << rang::fg::green << "Switched to Pi language mode"
             << rang::fg::reset << "\n";
        return true;
    }

    if (cmd == "rho") {
        SetLanguage(Language::Rho);
        cout << rang::fg::green << "Switched to Rho language mode"
             << rang::fg::reset << "\n";
        return true;
    }

    return false;  // Not a built-in command
}

void Console::ShowHelp(const std::string &topic) const {
    if (topic.empty()) {
        cout << rang::style::bold << "KAI Console Help" << rang::style::reset
             << "\n\n"
             << "Available help topics:\n"
             << rang::fg::cyan << "  help basics     " << rang::fg::reset
             << "- Basic usage and commands\n"
             << rang::fg::cyan << "  help history    " << rang::fg::reset
             << "- History and command expansion\n"
             << rang::fg::cyan << "  help shell      " << rang::fg::reset
             << "- Shell integration\n"
             << rang::fg::cyan << "  help languages  " << rang::fg::reset
             << "- Pi and Rho language features\n"
             << rang::fg::cyan << "  help network    " << rang::fg::reset
             << "- Network console commands\n\n"
             << "Language-specific help:\n"
             << rang::fg::cyan << "  help pi         " << rang::fg::reset
             << "- Pi language reference\n"
             << rang::fg::cyan << "  help rho        " << rang::fg::reset
             << "- Rho language reference\n\n";
        ShowBuiltinCommands();
    } else if (topic == "basics") {
        ShowBasicHelp();
    } else if (topic == "history") {
        ShowHistoryHelp();
    } else if (topic == "shell") {
        cout << rang::style::bold << "Shell Integration Help"
             << rang::style::reset << "\n\n"
             << "Execute shell commands with $ prefix:\n"
             << rang::fg::green << "  $ ls -la        " << rang::fg::reset
             << "# List files\n"
             << rang::fg::green << "  $ pwd           " << rang::fg::reset
             << "# Show current directory\n"
             << rang::fg::green << "  $ echo hello    " << rang::fg::reset
             << "# Echo text\n\n"
             << "Backtick expansion in code:\n"
             << rang::fg::yellow << "  `echo 42` 10 +  " << rang::fg::reset
             << "# Uses output of echo as number\n"
             << rang::fg::yellow << "  \"Hello \" `whoami` + "
             << rang::fg::reset
             << "# String concatenation with command output\n";
    } else if (topic == "languages") {
        cout << rang::style::bold << "Language Overview" << rang::style::reset
             << "\n\n"
             << rang::fg::cyan << "Pi Language:" << rang::fg::reset << "\n"
             << "  Stack-based, postfix notation\n"
             << "  Example: 2 3 + (adds 2 and 3)\n\n"
             << rang::fg::cyan << "Rho Language:" << rang::fg::reset << "\n"
             << "  C-like syntax with advanced features\n"
             << "  Example: x = 2 + 3;\n\n"
             << "Switch languages with: pi or rho\n";
    } else if (topic == "network") {
        ShowNetworkHelp();
    } else if (topic == "pi") {
        ShowLanguageHelp("pi");
    } else if (topic == "rho") {
        ShowLanguageHelp("rho");
    } else {
        cout << rang::fg::red << "Unknown help topic: " << rang::fg::reset
             << topic << "\n"
             << "Type 'help' for available topics.\n";
    }
}

void Console::ShowBasicHelp() const {
    cout << rang::style::bold << "Basic Usage" << rang::style::reset << "\n\n"
         << "KAI Console is a REPL (Read-Eval-Print Loop) for the KAI language "
            "system.\n\n"
         << rang::fg::cyan << "Getting Started:" << rang::fg::reset << "\n"
         << "  • Type expressions and press Enter to evaluate\n"
         << "  • Use Pi (stack-based) or Rho (C-like) syntax\n"
         << "  • Multi-line input is supported for complex structures\n\n"
         << rang::fg::cyan << "Examples:" << rang::fg::reset << "\n"
         << rang::fg::green << "  2 3 +           " << rang::fg::reset
         << "# Pi: adds 2 and 3\n"
         << rang::fg::green << "  x = 5;          " << rang::fg::reset
         << "# Rho: assigns 5 to x\n"
         << rang::fg::green << "  stack           " << rang::fg::reset
         << "# Show current stack\n";
}

void Console::ShowHistoryHelp() const {
    cout << rang::style::bold << "History and Command Expansion"
         << rang::style::reset << "\n\n"
         << "Command history is automatically saved and can be accessed:\n\n"
         << rang::fg::cyan << "Basic History:" << rang::fg::reset << "\n"
         << "  history         - Show all commands\n"
         << "  !!              - Repeat last command\n"
         << "  !n              - Repeat command number n\n\n"
         << rang::fg::cyan << "History Search:" << rang::fg::reset << "\n"
         << "  !string         - Find last command starting with 'string'\n"
         << "  !?string        - Find last command containing 'string'\n\n"
         << rang::fg::cyan << "Word Designators:" << rang::fg::reset << "\n"
         << "  !^              - First argument of last command\n"
         << "  !$              - Last argument of last command\n"
         << "  !*              - All arguments of last command\n";
}

void Console::ShowLanguageHelp(const std::string &lang) const {
    if (lang == "pi") {
        cout << rang::style::bold << "Pi Language Reference"
             << rang::style::reset << "\n\n"
             << "Pi is a stack-based language with postfix notation.\n\n"
             << rang::fg::cyan << "Basic Operations:" << rang::fg::reset << "\n"
             << rang::fg::green << "  2 3 +           " << rang::fg::reset
             << "# Push 2, push 3, add (result: 5)\n"
             << rang::fg::green << "  10 3 -          " << rang::fg::reset
             << "# Subtract (result: 7)\n"
             << rang::fg::green << "  4 5 *           " << rang::fg::reset
             << "# Multiply (result: 20)\n"
             << rang::fg::green << "  15 3 /          " << rang::fg::reset
             << "# Divide (result: 5)\n\n"
             << rang::fg::cyan << "Stack Operations:" << rang::fg::reset << "\n"
             << rang::fg::green << "  dup             " << rang::fg::reset
             << "# Duplicate top of stack\n"
             << rang::fg::green << "  swap            " << rang::fg::reset
             << "# Swap top two elements\n"
             << rang::fg::green << "  drop            " << rang::fg::reset
             << "# Remove top element\n"
             << rang::fg::green << "  over            " << rang::fg::reset
             << "# Copy second element to top\n\n"
             << rang::fg::cyan << "Control Flow:" << rang::fg::reset << "\n"
             << rang::fg::green << "  if then else    " << rang::fg::reset
             << "# Conditional execution\n"
             << rang::fg::green << "  while do        " << rang::fg::reset
             << "# Loop while condition is true\n";
    } else if (lang == "rho") {
        cout << rang::style::bold << "Rho Language Reference"
             << rang::style::reset << "\n\n"
             << "Rho is a C-like language with modern features.\n\n"
             << rang::fg::cyan << "Variables and Assignment:" << rang::fg::reset
             << "\n"
             << rang::fg::green << "  x = 42;         " << rang::fg::reset
             << "# Assign value to variable\n"
             << rang::fg::green << "  y = x * 2;      " << rang::fg::reset
             << "# Use variables in expressions\n\n"
             << rang::fg::cyan << "Control Structures:" << rang::fg::reset
             << "\n"
             << rang::fg::green << "  if (condition) { ... } "
             << rang::fg::reset << "# Conditional\n"
             << rang::fg::green << "  while (condition) { ... } "
             << rang::fg::reset << "# Loop\n"
             << rang::fg::green << "  for (i = 0; i < 10; i++) { ... } "
             << rang::fg::reset << "# For loop\n\n"
             << rang::fg::cyan << "Functions:" << rang::fg::reset << "\n"
             << rang::fg::green << "  fun name(args) { ... } "
             << rang::fg::reset << "# Define function\n"
             << rang::fg::green << "  name(arguments); " << rang::fg::reset
             << "# Call function\n";
    }
}

void Console::ShowBuiltinCommands() const {
    cout << rang::style::bold << "Built-in Commands:" << rang::style::reset
         << "\n"
         << rang::fg::cyan << "  help [topic]    " << rang::fg::reset
         << "- Show help (optionally for specific topic)\n"
         << rang::fg::cyan << "  clear, cls      " << rang::fg::reset
         << "- Clear screen\n"
         << rang::fg::cyan << "  exit, quit      " << rang::fg::reset
         << "- Exit console\n"
         << rang::fg::cyan << "  pi, rho         " << rang::fg::reset
         << "- Switch language mode\n"
         << rang::fg::cyan << "  history         " << rang::fg::reset
         << "- Show command history\n"
         << rang::fg::cyan << "  stack           " << rang::fg::reset
         << "- Show current stack\n"
         << rang::fg::cyan << "  $ <command>     " << rang::fg::reset
         << "- Execute shell command\n";
}

// History Management Implementation
void Console::LoadHistory() {
    // Set history file path
    const char *home = std::getenv("HOME");
    if (home) {
        historyFile = std::string(home) + "/.kai_history";
    } else {
        historyFile = ".kai_history";
    }

    std::ifstream file(historyFile);
    if (!file.is_open()) {
        return;  // File doesn't exist yet, that's fine
    }

    std::string line;
    while (std::getline(file, line) && commandHistory.size() < maxHistorySize) {
        if (!line.empty()) {
            commandHistory.push_back(line);
        }
    }
    file.close();
}

void Console::SaveHistory() const {
    if (historyFile.empty()) {
        return;
    }

    std::ofstream file(historyFile);
    if (!file.is_open()) {
        return;  // Can't save, but don't error
    }

    // Save only the last maxHistorySize entries
    size_t start = commandHistory.size() > maxHistorySize
                       ? commandHistory.size() - maxHistorySize
                       : 0;

    for (size_t i = start; i < commandHistory.size(); ++i) {
        file << commandHistory[i] << "\n";
    }
    file.close();
}

void Console::AddToHistory(const std::string &command) {
    // Don't add empty commands or duplicates of the last command
    if (command.empty() ||
        (!commandHistory.empty() && commandHistory.back() == command)) {
        return;
    }

    commandHistory.push_back(command);

    // Keep history size under control
    if (commandHistory.size() > maxHistorySize) {
        commandHistory.erase(
            commandHistory.begin(),
            commandHistory.begin() + (commandHistory.size() - maxHistorySize));
    }
}

// Network functionality implementation
bool Console::StartNetworking(int listenPort) {
    if (networkingEnabled_) {
        cout << rang::fg::yellow << "Networking already enabled" << rang::fg::reset << endl;
        return true;
    }
    
    listenPort_ = listenPort;
    peer_ = RakNet::RakPeerInterface::GetInstance();
    if (!peer_) {
        cerr << rang::fg::red << "Failed to create RakNet peer interface" << rang::fg::reset << endl;
        return false;
    }
    
    cout << rang::fg::cyan << "Starting network console on port " << listenPort_ 
         << rang::fg::reset << endl;
    
    RakNet::SocketDescriptor sd(listenPort_, nullptr);
    RakNet::StartupResult result = peer_->Startup(32, &sd, 1);
    
    if (result != RakNet::RAKNET_STARTED) {
        cerr << rang::fg::red << "Failed to start network listener, error: " 
             << result << rang::fg::reset << endl;
        RakNet::RakPeerInterface::DestroyInstance(peer_);
        peer_ = nullptr;
        return false;
    }
    
    peer_->SetMaximumIncomingConnections(32);
    networkingEnabled_ = true;
    networkRunning_ = true;
    
    messageThread_ = thread(&Console::ProcessNetworkMessages, this);
    
    cout << rang::fg::green << "Network console listening on port " << listenPort_ 
         << " (ID: " << consoleId_ << ")" << rang::fg::reset << endl;
    return true;
}

bool Console::ConnectToPeer(const std::string& host, int port) {
    if (!networkingEnabled_ || !peer_) {
        cout << rang::fg::red << "Networking not enabled. Use '/network start' first." 
             << rang::fg::reset << endl;
        return false;
    }
    
    cout << rang::fg::yellow << "Connecting to peer at " << host << ":" << port 
         << rang::fg::reset << endl;
    
    RakNet::ConnectionAttemptResult result = peer_->Connect(host.c_str(), port, nullptr, 0);
    
    if (result != RakNet::CONNECTION_ATTEMPT_STARTED) {
        cerr << rang::fg::red << "Failed to connect to " << host << ":" << port 
             << ", error: " << result << rang::fg::reset << endl;
        return false;
    }
    
    return true;
}

void Console::StopNetworking() {
    if (!networkingEnabled_) return;
    
    networkRunning_ = false;
    
    if (messageThread_.joinable()) {
        messageThread_.join();
    }
    
    if (peer_) {
        peer_->Shutdown(300);
        RakNet::RakPeerInterface::DestroyInstance(peer_);
        peer_ = nullptr;
    }
    
    networkingEnabled_ = false;
    cout << rang::fg::yellow << "Network console stopped" << rang::fg::reset << endl;
}

bool Console::SendCommandToPeer(const std::string& peerAddr, const std::string& command) {
    if (!networkingEnabled_) return false;
    
    RakNet::SystemAddress targetPeer = FindPeerByAddress(peerAddr);
    
    if (targetPeer == RakNet::UNASSIGNED_SYSTEM_ADDRESS) {
        cout << rang::fg::red << "Peer not found: " << peerAddr << rang::fg::reset << endl;
        return false;
    }
    
    RakNet::BitStream bs;
    bs.Write(static_cast<RakNet::MessageID>(NetworkMessageType::CONSOLE_COMMAND));
    bs.Write(consoleId_);
    bs.Write(command);
    bs.Write(static_cast<int>(GetLanguage()));
    
    peer_->Send(&bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, targetPeer, false);
    
    cout << rang::fg::cyan << "-> [" << AddressToString(targetPeer) << "] " 
         << command << rang::fg::reset << endl;
    
    return true;
}

void Console::BroadcastCommand(const std::string& command) {
    if (!networkingEnabled_) return;
    
    lock_guard<mutex> lock(peersMutex_);
    
    if (connectedPeers_.empty()) {
        cout << rang::fg::yellow << "No peers connected for broadcast" << rang::fg::reset << endl;
        return;
    }
    
    RakNet::BitStream bs;
    bs.Write(static_cast<RakNet::MessageID>(NetworkMessageType::CONSOLE_BROADCAST));
    bs.Write(consoleId_);
    bs.Write(command);
    bs.Write(static_cast<int>(GetLanguage()));
    
    for (const auto& peerAddr : connectedPeers_) {
        peer_->Send(&bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, peerAddr, false);
    }
    
    cout << rang::fg::magenta << ">> [BROADCAST] " << command << rang::fg::reset << endl;
}

std::vector<std::string> Console::GetConnectedPeers() const {
    lock_guard<mutex> lock(const_cast<mutex&>(peersMutex_));
    vector<string> peers;
    
    for (const auto& peer : connectedPeers_) {
        peers.push_back(AddressToString(peer));
    }
    
    return peers;
}

std::vector<NetworkConsoleMessage> Console::GetNetworkHistory() const {
    return messageHistory_;
}

void Console::SetNetworkMessageCallback(std::function<void(const NetworkConsoleMessage&)> callback) {
    messageCallback_ = callback;
}

String Console::ProcessNetworkCommand(const String& command) {
    string cmd = command.c_str();
    stringstream ss(cmd);
    string verb;
    ss >> verb;
    
    if (verb == "/network") {
        string subCmd;
        ss >> subCmd;
        
        if (subCmd == "start") {
            int port = 14600;
            ss >> port;  // Optional port override
            bool success = StartNetworking(port);
            return success ? String("Network started") : String("Failed to start network");
        }
        
        if (subCmd == "stop") {
            StopNetworking();
            return String("Network stopped");
        }
        
        if (subCmd == "status") {
            if (networkingEnabled_) {
                auto peers = GetConnectedPeers();
                return String("Network enabled, port ") + std::to_string(listenPort_) + 
                       String(", peers: ") + std::to_string(static_cast<int>(peers.size()));
            }
            return String("Network disabled");
        }
        
        return String("Usage: /network {start|stop|status} [port]");
    }
    
    if (!networkingEnabled_) {
        return String("Network not enabled. Use '/network start' first.");
    }
    
    if (verb == "/connect") {
        string host;
        int port;
        if (ss >> host >> port) {
            bool success = ConnectToPeer(host, port);
            return success ? String("Connecting...") : String("Connection failed");
        } else {
            return String("Usage: /connect <host> <port>");
        }
    }
    
    if (verb == "/peers") {
        auto peers = GetConnectedPeers();
        if (peers.empty()) {
            return String("No peers connected");
        }
        
        stringstream result;
        result << "Connected peers (" << peers.size() << "):";
        for (size_t i = 0; i < peers.size(); ++i) {
            result << "\n  " << i << ": " << peers[i];
        }
        return String(result.str().c_str());
    }
    
    if (verb == "/broadcast") {
        string broadcastCmd;
        getline(ss, broadcastCmd);
        if (!broadcastCmd.empty() && broadcastCmd[0] == ' ') {
            broadcastCmd = broadcastCmd.substr(1);
        }
        
        if (broadcastCmd.empty()) {
            return String("Usage: /broadcast <command>");
        }
        
        BroadcastCommand(broadcastCmd);
        return String("");
    }
    
    if (verb.find("/@") == 0) {
        string peerAddr = verb.substr(2);
        string remoteCmd;
        getline(ss, remoteCmd);
        if (!remoteCmd.empty() && remoteCmd[0] == ' ') {
            remoteCmd = remoteCmd.substr(1);
        }
        
        if (remoteCmd.empty()) {
            return String("Usage: /@<peer> <command>");
        }
        
        bool success = SendCommandToPeer(peerAddr, remoteCmd);
        return success ? String("") : String("Failed to send command");
    }
    
    if (verb == "/nethistory") {
        auto history = GetNetworkHistory();
        if (history.empty()) {
            return String("No network message history");
        }
        
        stringstream result;
        result << "Network History (" << history.size() << " messages):";
        for (const auto& msg : history) {
            result << "\n[" << msg.senderId << "] " << msg.command;
            if (!msg.result.empty()) {
                result << " -> " << msg.result;
            }
        }
        return String(result.str().c_str());
    }
    
    return String("Unknown network command: ") + command;
}

void Console::ShowNetworkHelp() const {
    cout << rang::style::bold << "Network Console Commands:" << rang::style::reset << endl;
    cout << "  /network start [port]   - Start networking (default port 14600)" << endl;
    cout << "  /network stop           - Stop networking" << endl; 
    cout << "  /network status         - Show network status" << endl;
    cout << "  /connect <host> <port>  - Connect to a peer console" << endl;
    cout << "  /peers                  - List connected peers" << endl;
    cout << "  /broadcast <command>    - Broadcast command to all peers" << endl;
    cout << "  /@<peer> <command>      - Send command to specific peer" << endl;
    cout << "  /nethistory             - Show network message history" << endl;
}

void Console::ProcessNetworkMessages() {
    while (networkRunning_) {
        if (!peer_) break;
        
        RakNet::Packet* packet;
        while ((packet = peer_->Receive()) != nullptr) {
            HandleNetworkPacket(packet);
            peer_->DeallocatePacket(packet);
        }
        
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void Console::HandleNetworkPacket(RakNet::Packet* packet) {
    if (!packet || packet->length < 1) return;
    
    NetworkMessageType msgType = static_cast<NetworkMessageType>(packet->data[0]);
    
    switch (msgType) {
        case NetworkMessageType::CONSOLE_COMMAND:
            HandleConsoleCommand(packet);
            break;
        case NetworkMessageType::CONSOLE_RESULT:
            HandleConsoleResult(packet);
            break;
        case NetworkMessageType::CONSOLE_BROADCAST:
            HandleConsoleBroadcast(packet);
            break;
        case NetworkMessageType::CONSOLE_LANGUAGE_SWITCH:
            HandleLanguageSwitch(packet);
            break;
        default:
            if (packet->data[0] == RakNet::ID_NEW_INCOMING_CONNECTION) {
                cout << rang::fg::green << "<- Peer connected: " 
                     << AddressToString(packet->systemAddress) << rang::fg::reset << endl;
                AddPeer(packet->systemAddress);
            } else if (packet->data[0] == RakNet::ID_CONNECTION_REQUEST_ACCEPTED) {
                cout << rang::fg::green << "<- Connected to peer: " 
                     << AddressToString(packet->systemAddress) << rang::fg::reset << endl;
                AddPeer(packet->systemAddress);
            } else if (packet->data[0] == RakNet::ID_DISCONNECTION_NOTIFICATION ||
                      packet->data[0] == RakNet::ID_CONNECTION_LOST) {
                cout << rang::fg::yellow << "<- Peer disconnected: " 
                     << AddressToString(packet->systemAddress) << rang::fg::reset << endl;
                RemovePeer(packet->systemAddress);
            }
            break;
    }
}

void Console::HandleConsoleCommand(RakNet::Packet* packet) {
    RakNet::BitStream bs(packet->data, packet->length, false);
    bs.IgnoreBytes(sizeof(RakNet::MessageID));
    
    string senderId, command;
    int languageInt;
    bs.Read(senderId);
    bs.Read(command);
    bs.Read(languageInt);
    
    Language originalLang = GetLanguage();
    Language remoteLang = static_cast<Language>(languageInt);
    
    cout << rang::fg::cyan << "<- [" << senderId << "] " << command << rang::fg::reset << endl;
    
    try {
        if (remoteLang != originalLang) {
            SetLanguage(remoteLang);
        }
        
        Execute(String(command.c_str()));
        string result = WriteStack().c_str();
        
        if (remoteLang != originalLang) {
            SetLanguage(originalLang);
        }
        
        SendResultToPeer(packet->systemAddress, command, result);
        
        NetworkConsoleMessage msg;
        msg.senderId = senderId;
        msg.command = command;
        msg.result = result;
        msg.language = remoteLang;
        msg.timestamp = chrono::system_clock::now().time_since_epoch().count();
        LogNetworkMessage(msg);
        
    } catch (const Exception::Base& e) {
        string error = "Error: " + string(e.ToString().c_str());
        SendResultToPeer(packet->systemAddress, command, error);
        
        if (remoteLang != originalLang) {
            SetLanguage(originalLang);
        }
    }
}

void Console::HandleConsoleResult(RakNet::Packet* packet) {
    RakNet::BitStream bs(packet->data, packet->length, false);
    bs.IgnoreBytes(sizeof(RakNet::MessageID));
    
    string senderId, command, result;
    bs.Read(senderId);
    bs.Read(command);
    bs.Read(result);
    
    cout << rang::fg::green << "<- [" << senderId << "] Result: " << result 
         << rang::fg::reset << endl;
    
    NetworkConsoleMessage msg;
    msg.senderId = senderId;
    msg.command = command;
    msg.result = result;
    msg.timestamp = chrono::system_clock::now().time_since_epoch().count();
    LogNetworkMessage(msg);
}

void Console::HandleConsoleBroadcast(RakNet::Packet* packet) {
    RakNet::BitStream bs(packet->data, packet->length, false);
    bs.IgnoreBytes(sizeof(RakNet::MessageID));
    
    string senderId, command;
    int languageInt;
    bs.Read(senderId);
    bs.Read(command);
    bs.Read(languageInt);
    
    cout << rang::fg::magenta << "<< [BROADCAST from " << senderId << "] " 
         << command << rang::fg::reset << endl;
    
    Language originalLang = GetLanguage();
    Language remoteLang = static_cast<Language>(languageInt);
    
    try {
        if (remoteLang != originalLang) {
            SetLanguage(remoteLang);
        }
        
        Execute(String(command.c_str()));
        string result = WriteStack().c_str();
        cout << "   Result: " << result << endl;
        
        if (remoteLang != originalLang) {
            SetLanguage(originalLang);
        }
        
        NetworkConsoleMessage msg;
        msg.senderId = senderId + " [BROADCAST]";
        msg.command = command;
        msg.result = result;
        msg.language = remoteLang;
        msg.timestamp = chrono::system_clock::now().time_since_epoch().count();
        LogNetworkMessage(msg);
        
    } catch (const Exception::Base& e) {
        cout << rang::fg::red << "   Error: " << e.ToString().c_str() 
             << rang::fg::reset << endl;
        
        if (remoteLang != originalLang) {
            SetLanguage(originalLang);
        }
    }
}

void Console::HandleLanguageSwitch(RakNet::Packet* packet) {
    RakNet::BitStream bs(packet->data, packet->length, false);
    bs.IgnoreBytes(sizeof(RakNet::MessageID));
    
    string senderId;
    int languageInt;
    bs.Read(senderId);
    bs.Read(languageInt);
    
    Language newLang = static_cast<Language>(languageInt);
    cout << rang::fg::blue << "<- [" << senderId << "] switched to " 
         << (newLang == Language::Pi ? "Pi" : "Rho") << " language" 
         << rang::fg::reset << endl;
}

void Console::SendResultToPeer(const RakNet::SystemAddress& peer, 
                              const std::string& command, const std::string& result) {
    if (!peer_) return;
    
    RakNet::BitStream bs;
    bs.Write(static_cast<RakNet::MessageID>(NetworkMessageType::CONSOLE_RESULT));
    bs.Write(consoleId_);
    bs.Write(command);
    bs.Write(result);
    
    peer_->Send(&bs, RakNet::HIGH_PRIORITY, RakNet::RELIABLE_ORDERED, 0, peer, false);
}

void Console::AddPeer(const RakNet::SystemAddress& address) {
    lock_guard<mutex> lock(peersMutex_);
    connectedPeers_.push_back(address);
}

void Console::RemovePeer(const RakNet::SystemAddress& address) {
    lock_guard<mutex> lock(peersMutex_);
    connectedPeers_.erase(
        remove(connectedPeers_.begin(), connectedPeers_.end(), address),
        connectedPeers_.end());
}

void Console::LogNetworkMessage(const NetworkConsoleMessage& message) {
    messageHistory_.push_back(message);
    if (messageHistory_.size() > 1000) {
        messageHistory_.erase(messageHistory_.begin());
    }
    
    if (messageCallback_) {
        messageCallback_(message);
    }
}

std::string Console::GenerateConsoleId() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1000, 9999);
    return "Console-" + to_string(dis(gen));
}

std::string Console::AddressToString(const RakNet::SystemAddress& addr) const {
    return addr.ToString();
}

RakNet::SystemAddress Console::FindPeerByAddress(const std::string& addr) const {
    lock_guard<mutex> lock(const_cast<mutex&>(peersMutex_));
    
    for (const auto& peer : connectedPeers_) {
        string peerStr = AddressToString(peer);
        if (peerStr.find(addr) != string::npos || addr == to_string(&peer - &connectedPeers_[0])) {
            return peer;
        }
    }
    
    return RakNet::UNASSIGNED_SYSTEM_ADDRESS;
}

KAI_END

// EOF
