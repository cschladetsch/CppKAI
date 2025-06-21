#include <iostream>
#include <fstream>
#include <filesystem>

int main() {
    // Check if log file exists and show recent entries
    std::string logPath = std::string(getenv("HOME")) + "/local/KAI/Logs/kai.log";
    
    if (std::filesystem::exists(logPath)) {
        std::cout << "Log file exists at: " << logPath << std::endl;
        
        // Get file size
        auto fileSize = std::filesystem::file_size(logPath);
        std::cout << "Log file size: " << fileSize << " bytes" << std::endl;
        
        // Read last few lines
        std::ifstream logFile(logPath);
        std::string line;
        std::vector<std::string> lastLines;
        
        while (std::getline(logFile, line)) {
            lastLines.push_back(line);
            if (lastLines.size() > 10) {
                lastLines.erase(lastLines.begin());
            }
        }
        
        std::cout << "\nLast 10 log entries:" << std::endl;
        for (const auto& l : lastLines) {
            std::cout << l << std::endl;
        }
    } else {
        std::cout << "Log file does not exist at: " << logPath << std::endl;
    }
    
    return 0;
}