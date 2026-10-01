#pragma once

#include "context.hpp"
#include <functional>
#include <unordered_map>
#include <string>

namespace kai::ksh {

    using PrimitiveFunc = std::function<void(Context&)>;

    struct Registry {
        std::unordered_map<std::string, PrimitiveFunc> primitives;

        void bind(std::string name, PrimitiveFunc func) {
            primitives[name] = std::move(func);
        }

        bool invoke(const std::string& name, Context& ctx) const {
            auto it = primitives.find(name);
            if (it != primitives.end()) {
                it->second(ctx);
                return true;
            }
            return false;
        }
    };

    Registry& global_registry();
    void register_shell_primitives(Registry& registry);

}
