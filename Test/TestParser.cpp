#include <fstream>
#include <iostream>
#include <sstream>

#include "KAI/Core/Config/Base.h"
#include "KAI/Language/Tau/TauParser.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <tau_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    kai::Registry r;
    auto lexer = std::make_shared<kai::tau::TauLexer>(content.c_str(), r);
    bool lexerResult = lexer->Process();

    std::cout << "Lexer result: " << (lexerResult ? "SUCCESS" : "FAILED")
              << std::endl;
    if (!lexerResult) {
        std::cout << "Lexer error: " << lexer->Error << std::endl;
        return 1;
    }

    std::cout << "Lexer tokens:\n" << lexer->Print() << std::endl;

    auto parser = std::make_shared<kai::tau::TauParser>(r);
    bool parserResult = parser->Process(lexer, kai::Structure::Module);

    std::cout << "Parser result: " << (parserResult ? "SUCCESS" : "FAILED")
              << std::endl;
    if (!parserResult) {
        std::cout << "Parser error: " << parser->Error << std::endl;
        return 1;
    }

    std::cout << "Parsing successful!" << std::endl;
    return 0;
}