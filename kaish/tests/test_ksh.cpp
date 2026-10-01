#include "ksh/context.hpp"
#include "ksh/object.hpp"
#include "ksh/parser.hpp"
#include <iostream>
#include <cassert>

void test_string_object() {
    auto strObj = std::make_shared<kai::ksh::StringObject>("test_value");
    assert(strObj->to_string() == "test_value");
    assert(!strObj->is_executable());
    std::cout << "[PASSED] test_string_object\n";
}

void test_stack_operations() {
    kai::ksh::Context ctx;
    auto obj1 = std::make_shared<kai::ksh::StringObject>("alpha");
    auto obj2 = std::make_shared<kai::ksh::StringObject>("beta");

    ctx.push(obj1);
    ctx.push(obj2);

    assert(ctx.data_stack.size() == 2);

    auto popped = ctx.pop<kai::ksh::StringObject>();
    assert(popped->value == "beta");
    assert(ctx.data_stack.size() == 1);
    std::cout << "[PASSED] test_stack_operations\n";
}

void test_parser_rho() {
    auto block = kai::ksh::Parser::parse_rho("ls -la /tmp");
    assert(block.instructions.size() == 3);
    assert(block.instructions[0]->to_string() == "ls");
    assert(block.instructions[1]->to_string() == "-la");
    assert(block.instructions[2]->to_string() == "/tmp");
    std::cout << "[PASSED] test_parser_rho\n";
}

int main() {
    std::cout << "Running ksh unit tests...\n";
    try {
        test_string_object();
        test_stack_operations();
        test_parser_rho();
        std::cout << "All tests passed successfully!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}
