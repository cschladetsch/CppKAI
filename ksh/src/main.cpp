#include "ksh/context.hpp"
#include "ksh/parser.hpp"
#include "ksh/registry.hpp"
#include <iostream>

int main() {
    using namespace kai::ksh;
    
    register_shell_primitives(global_registry());
    Context ctx;
    std::string input;

    std::cout << "ksh (KAI Object Shell) v0.1\n";
    while (std::cout << "ksh> " && std::getline(std::cin, input)) {
        if (input == "exit") break;
        if (input.empty()) continue;

        try {
            CodeBlock block = Parser::parse_rho(input);
            block.execute(ctx);

            if (!ctx.data_stack.empty()) {
                std::cout << "=> " << ctx.data_stack.back()->to_string() << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}
