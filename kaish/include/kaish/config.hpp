#pragma once

#include "backend.hpp"

#include <iosfwd>
#include <string>
#include <vector>

namespace kai::kaish {

    // ~/.kaish.json. Missing keys keep these defaults.
    struct Config {
        int stack_levels = 8;  // levels shown after each command
        Mode mode = Mode::Ps;  // starting mode
        bool show_stack = true;
        // Programs that need the real terminal (editors, pagers, REPLs). Their
        // output is not captured onto the stack.
        std::vector<std::string> passthrough{"vim", "vi", "nvim", "nano", "less", "more", "man", "ssh",
                                             "top", "htop", "python", "python3", "node", "pwsh",
                                             "powershell", "cmd", "bash", "wsl", "claude"};
    };

    // Unknown keys are ignored; bad values are reported to err and keep their defaults.
    Config parse_config(const std::string& json, std::ostream& err);
    std::string config_to_json(const Config& c);
    // Reads path; if it does not exist and create_if_missing is set, writes the defaults there.
    Config load_config(const std::string& path, std::ostream& err, bool create_if_missing);

}
