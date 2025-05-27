#include <KAI/Core/BuiltinTypes/All.h>
#include <KAI/Language/Rho/RhoLexer.h>
#include <KAI/Language/Rho/RhoParser.h>

#include <iostream>

using namespace kai;

int main() {
    try {
        std::cout << "Creating registry..." << std::endl;
        Registry reg;

        std::cout << "Creating lexer with simple input..." << std::endl;
        const char* input = "x = 10";
        RhoLexer lexer(input, reg);

        std::cout << "Adding keywords..." << std::endl;
        lexer.AddKeyWords();

        std::cout << "Running lexer..." << std::endl;
        lexer.Process();

        std::cout << "Lexer output: " << lexer.ToString() << std::endl;

        std::cout << "Creating parser..." << std::endl;
        RhoParser parser(reg);

        std::cout << "Parsing tokens..." << std::endl;
        auto lexer_ptr = std::make_shared<RhoLexer>(lexer);
        bool success = parser.Process(lexer_ptr, Structure::Expression);

        if (success) {
            std::cout << "Parse tree: " << parser.PrintTree() << std::endl;
        } else {
            std::cout << "Parsing failed!" << std::endl;
            std::cout << "Error: " << parser.Error << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}