#pragma once

#include "object.hpp"
#include <vector>
#include <stdexcept>

namespace kai::ksh {

    struct Context {
        std::vector<ObjectPtr> data_stack;
        std::vector<ObjectPtr> return_stack;
        size_t instruction_pointer = 0;
        bool running = true;

        void push(ObjectPtr obj) {
            data_stack.push_back(std::move(obj));
        }

        template <typename T>
        std::shared_ptr<T> pop() {
            if (data_stack.empty()) {
                throw std::runtime_error("Stack underflow");
            }
            auto top = data_stack.back();
            data_stack.pop_back();
            return std::dynamic_pointer_cast<T>(top);
        }

        ObjectPtr pop_any() {
            if (data_stack.empty()) {
                throw std::runtime_error("Stack underflow");
            }
            auto top = data_stack.back();
            data_stack.pop_back();
            return top;
        }

        void step(const CodeBlock& block);
        void execute(const CodeBlock& block);
    };

    ObjectPtr checkpoint_context(const Context& ctx);

}
