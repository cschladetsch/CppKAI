#include "ksh/object.hpp"
#include "ksh/context.hpp"

namespace kai::ksh {

    void CodeBlock::execute(Context& ctx) {
        for (const auto& instr : instructions) {
            if (instr->is_executable()) {
                instr->execute(ctx);
            } else {
                ctx.push(instr);
            }
        }
    }

    std::string CodeBlock::to_string() const {
        std::string s = "{ ";
        for (const auto& i : instructions) {
            s += i->to_string() + " ";
        }
        return s + "}";
    }

}
