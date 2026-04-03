#pragma once

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace kai {
namespace test {

// Helper function to load a Rho script from file
inline std::string LoadRhoScript(const std::string& filename) {
    std::string path = "Test/Language/TestRho/Scripts/" + filename;
    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open script file: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace test
}  // namespace kai