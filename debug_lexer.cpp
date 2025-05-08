#include <KAI/Language/Rho/RhoLexer.h>
#include <KAI/Language/Rho/RhoParser.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace KAI;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: debug_lexer <rho_file>" << endl;
        return 1;
    }
    
    // Read the file
    ifstream file(argv[1]);
    if (!file) {
        cout << "Could not open file: " << argv[1] << endl;
        return 1;
    }
    
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    
    cout << "File content: " << content << endl;
    
    // Create a lexer and process the content
    auto lexer = std::make_shared<RhoLexer>();
    lexer->Process(content);
    
    // Check for lexer errors
    if (lexer->Failed) {
        cout << "Lexer failed: " << lexer->Error << endl;
        return 1;
    }
    
    // Print all tokens
    cout << "Tokens generated:" << endl;
    for (auto& token : lexer->GetTokens()) {
        cout << "  - Type: " << RhoTokenEnumType::ToString(token.type) 
             << " (" << static_cast<int>(token.type) << ")" 
             << ", Text: \"" << token.Text() << "\"" << endl;
    }
    
    // Try to parse the content
    auto parser = std::make_shared<RhoParser>();
    if (!parser->Process(lexer, ParserBase::Structure::Program)) {
        cout << "Parser failed: " << parser->GetError() << endl;
        return 1;
    }
    
    cout << "Parsing succeeded!" << endl;
    return 0;
}