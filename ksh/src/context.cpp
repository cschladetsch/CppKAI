#include "ksh/context.hpp"

namespace kai::ksh {

    void Context::step(const CodeBlock& block) {
        if (instruction_pointer >= block.instructions.size()) {
            running = false;
            return;
        }

        auto current_item = block.instructions[instruction_pointer++];
        if (current_item->is_executable()) {
            current_item->execute(*this);
        } else {
            data_stack.push_back(current_item);
        }
    }

    void Context::execute(const CodeBlock& block) {
        for (instruction_pointer = 0; instruction_pointer < block.instructions.size();) {
            step(block);
        }
    }

    ObjectPtr checkpoint_context(const Context& ctx) {
        auto continuation = std::make_shared<CodeBlock>();
        continuation->instructions = ctx.data_stack;
        return continuation;
    }

}
