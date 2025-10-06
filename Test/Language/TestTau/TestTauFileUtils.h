#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace tau_test_utils {

inline std::filesystem::path ResolveScriptPath(const std::string &relative) {
    namespace fs = std::filesystem;
    fs::path current = fs::current_path();
    for (int depth = 0; depth < 10; ++depth) {
        const fs::path candidate = current / "Test" / "Language" /
                                  "TestTau" / "Scripts" / fs::path(relative);
        if (fs::exists(candidate)) {
            return candidate;
        }
        if (!current.has_parent_path()) {
            break;
        }
        current = current.parent_path();
    }
    return fs::path(relative);
}

inline std::string LoadScriptText(const std::string &relative) {
    const auto path = ResolveScriptPath(relative);
    std::ifstream file(path);
    if (!file.is_open()) {
        return {};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace tau_test_utils
