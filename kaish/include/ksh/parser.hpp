#pragma once

#include "object.hpp"
#include <string_view>

namespace kai::ksh {

    struct Parser {
        static CodeBlock parse_rho(std::string_view source_code);
        static CodeBlock parse_pi(std::string_view source_code);
    };

}
