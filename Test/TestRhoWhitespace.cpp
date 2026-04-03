#include <KAI/Core/BuiltinTypes.h>
#include <KAI/Core/Console.h>
#include <KAI/Language/Rho/RhoAstNode.h>
#include <KAI/Language/Rho/RhoLexer.h>
#include <KAI/Language/Rho/RhoParser.h>

#include <iostream>
#include <string>

using namespace kai;
using namespace kai::rho;
using namespace std;

void PrintTokens(const vector<Token> &tokens) {
    cout << "=== TOKENS ===" << endl;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto &tok = tokens[i];
        cout << i << ": " << tok << " (line=" << tok.lineNumber << ", slice='"
             << tok.slice.str() << "')" << endl;
    }
    cout << "=== END TOKENS ===" << endl << endl;
}

void PrintAst(AstNodePtr node, int indent = 0) {
    if (!node) {
        cout << string(indent * 2, ' ') << "(null)" << endl;
        return;
    }

    cout << string(indent * 2, ' ') << "Node: " << node->GetType()
         << " token=" << node->GetToken() << endl;

    for (const auto &child : node->GetChildren()) {
        PrintAst(child, indent + 1);
    }
}

int main() {
    try {
        // Test case: simple if statement with indentation
        string code = "result = 0\nif true\n    result = 42\nresult";

        cout << "=== INPUT CODE ===" << endl;
        cout << code << endl;
        cout << "=== END INPUT ===" << endl << endl;

        // Create registry
        Registry reg;

        // Create and run lexer
        cout << "Creating lexer..." << endl;
        RhoLexer lexer(code);
        if (!lexer.Process()) {
            cerr << "Lexer failed!" << endl;
            if (lexer.HasError()) {
                cerr << "Error: " << lexer.GetError() << endl;
            }
            return 1;
        }

        // Get tokens
        auto tokens = lexer.GetTokens();
        cout << "Lexer produced " << tokens.size() << " tokens" << endl;
        PrintTokens(tokens);

        // Create parser
        cout << "Creating parser..." << endl;
        RhoParser parser(reg);

        // Enable debug output if possible
        // parser.EnableDebug(); // Uncomment if this method exists

        // Process tokens
        cout << "Processing tokens..." << endl;
        bool parseResult = parser.Process(lexer, Structure::Statement);

        if (!parseResult) {
            cerr << "Parser failed!" << endl;
            if (parser.Failed()) {
                cerr << "Error: " << parser.GetError() << endl;
                cerr << "Error detail: " << parser.GetErrorMessage() << endl;
            }

            // Try to get partial AST
            cout << "\n=== PARTIAL AST ===" << endl;
            auto ast = parser.GetRoot();
            if (ast) {
                PrintAst(ast);
            } else {
                cout << "No AST root available" << endl;
            }

            return 1;
        }

        cout << "Parser succeeded!" << endl;

        // Print AST
        cout << "\n=== AST ===" << endl;
        auto ast = parser.GetRoot();
        PrintAst(ast);

        // Try to translate if we have a translator
        cout << "\n=== TRANSLATION ===" << endl;
        auto trans = parser.GetTranslator();
        if (trans) {
            trans->Translate(ast);
            if (trans->Failed()) {
                cerr << "Translation failed: " << trans->GetError() << endl;
            } else {
                cout << "Translation succeeded" << endl;
                // Print continuation or other output if available
            }
        } else {
            cout << "No translator available" << endl;
        }

    } catch (const exception &e) {
        cerr << "Exception: " << e.what() << endl;
        return 1;
    }

    return 0;
}