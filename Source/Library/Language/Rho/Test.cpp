#include <KAI/Executor/Executor.h>
#include <KAI/Language/Common/ProcessCommon.h>
#include <KAI/Language/Rho/RhoParser.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <iostream>

using namespace KAI;
using namespace std;

int main() {
    // Create registry
    Registry reg;

    // Create parser
    auto parser = std::make_shared<RhoParser>(reg);

    // Create lexer
    auto lexer = std::make_shared<RhoLexer>(reg);

    // Create translator
    auto translator = std::make_shared<RhoTranslator>(reg);

    // Create executor
    Executor exec(reg);

    try {
        cout << "Testing do-while implementation..." << endl;

        // Simple do-while script
        const char *script =
            "i = 0\n"
            "do\n"
            "    i = i + 1\n"
            "while i < 3\n";

        cout << "Script:" << endl << script << endl;

        // Set trace level
        exec.SetTraceLevel(5);

        // Tokenize
        cout << "Tokenizing..." << endl;
        if (!lexer->Process(script)) {
            cout << "Lexer failed: " << lexer->GetError() << endl;
            return 1;
        }

        // Parse
        cout << "Parsing..." << endl;
        if (!parser->Process(lexer, Structure::Program)) {
            cout << "Parser failed: " << parser->GetError() << endl;
            return 1;
        }

        // Translate
        cout << "Translating..." << endl;
        translator->Process(parser);
        if (translator->Failed) {
            cout << "Translator failed: " << translator->Error << endl;
            return 1;
        }

        // Execute
        cout << "Executing..." << endl;
        auto continuation = translator->GetResult();
        exec.Continue(continuation);

        // Check result
        cout << "Checking results..." << endl;
        auto stack = exec.GetDataStack();
        cout << "Stack size: " << stack->Size() << endl;

        cout << "Test completed successfully!" << endl;
        return 0;
    } catch (Exception::Base &e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
    } catch (exception &e) {
        cerr << "Standard Exception: " << e.what() << endl;
    } catch (...) {
        cerr << "Unknown exception" << endl;
    }

    return 1;
}