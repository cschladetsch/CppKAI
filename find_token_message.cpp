#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

bool searchInFile(const fs::path& filePath, const std::string& searchTerm) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    std::regex pattern(".*" + searchTerm + ".*");
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        if (std::regex_search(line, pattern)) {
            std::cout << "Found in " << filePath.string() << " at line " << lineNumber << ": " << line << std::endl;
            return true;
        }
    }
    
    return false;
}

void searchDirectory(const fs::path& directory, const std::string& searchTerm) {
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Directory does not exist: " << directory << std::endl;
        return;
    }
    
    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            // Only search in .cpp and .h files
            std::string ext = entry.path().extension().string();
            if (ext == ".cpp" || ext == ".h") {
                searchInFile(entry.path(), searchTerm);
            }
        }
    }
}

int main() {
    std::string baseDir = "/home/xian/local/KAI";
    
    // Search for both the exact string and potential components
    searchDirectory(baseDir, "Info: Token");
    searchDirectory(baseDir, "Info.*Token");
    searchDirectory(baseDir, "Logger.*Info.*Token");
    searchDirectory(baseDir, "\"Token\"");
    
    return 0;
}