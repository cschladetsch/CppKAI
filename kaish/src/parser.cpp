#include "ksh/parser.hpp"
#include <sstream>

namespace kai::ksh {

    CodeBlock Parser::parse_rho(std::string_view source_code) {
        CodeBlock block;
        std::stringstream ss{std::string(source_code)};
        std::string token;
        while (ss >> token) {
            block.instructions.push_back(std::make_shared<StringObject>(token));
        }
        return block;
    }

    CodeBlock Parser::parse_pi(std::string_view source_code) {
        return parse_rho(source_code);
    }

}
