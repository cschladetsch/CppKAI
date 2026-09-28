#include "ksh/registry.hpp"

namespace kai::ksh {

    Registry& global_registry() {
        static Registry reg;
        return reg;
    }

    void register_shell_primitives(Registry& registry) {
        registry.bind("echo", [](Context& ctx) {
            auto obj = ctx.pop_any();
            ctx.push(obj);
        });
    }

}
