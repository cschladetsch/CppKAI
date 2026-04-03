#include <KAI/Executor/Executor.h>
#include <KAI/Language/Common/ProcessCommon.h>
#include <KAI/Language/Rho/RhoParser.h>
#include <KAI/Language/Rho/RhoTranslator.h>

#include <iostream>
#include <string>

using namespace KAI;
using namespace std;

int main() {
    try {
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

        cout << "=== Testing do-while implementation ===" << endl;

        // Simple do-while script
        const char* script =
            "// Simple do-while test with proper indentation\n"
            "i = 0\n"
            "do\n"
            "    i = i + 1\n"
            "    print(\"iteration: \" + i)\n"
            "while i < 3\n"
            "print(\"Final i: \" + i)\n";

        cout << "Script:" << endl << script << endl;

        // Set trace level
        exec.SetTraceLevel(5);

        // Tokenize
        cout << "Tokenizing..." << endl;
        if (!lexer->Process(script)) {
            cout << "Lexer failed: " << lexer->GetError() << endl;
            return 1;
        }

        cout << "Tokens:" << endl;
        for (const auto& token : lexer->GetTokens()) {
            cout << "  " << TokenEnumType::ToString(token.type) << " '"
                 << token.text << "'" << endl;
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

        // Print the entire stack
        if (stack->Size() > 0) {
            cout << "Stack contents:" << endl;
            for (int i = 0; i < stack->Size(); i++) {
                Object obj = stack->At(i);
                if (obj.GetClass()) {
                    cout << "  [" << i << "]: " << obj.ToString()
                         << " (Type: " << obj.GetClass()->GetName() << ")"
                         << endl;
                } else {
                    cout << "  [" << i << "]: <no class>" << endl;
                }
            }
        }

        cout << "Test completed successfully!" << endl;
        return 0;
    } catch (Exception::Base& e) {
        cerr << "KAI Exception: " << e.ToString() << endl;
    } catch (exception& e) {
        cerr << "Standard Exception: " << e.what() << endl;
    } catch (...) {
        cerr << "Unknown exception" << endl;
    }

    return 1;
}