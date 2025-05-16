#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

// Simple test file to verify logging with file and line information
// This is a standalone test that simulates the behavior of the enhanced logging system

// Simplified logger implementation
class Logger {
public:
    enum class Level { Debug, Info, Warning, Error, Fatal };
    
    static void Log(Level level, const std::string& message) {
        std::string levelStr;
        switch (level) {
            case Level::Debug: levelStr = "DEBUG"; break;
            case Level::Info: levelStr = "INFO"; break;
            case Level::Warning: levelStr = "WARNING"; break;
            case Level::Error: levelStr = "ERROR"; break;
            case Level::Fatal: levelStr = "FATAL"; break;
        }
        
        std::cout << "[" << levelStr << "] " << message << std::endl;
    }
    
    static void LogWithLocation(Level level, const std::string& message, const char* file, int line) {
        std::string filename = ExtractFilename(file);
        std::string locationInfo = "[" + filename + ":" + std::to_string(line) + "] ";
        Log(level, locationInfo + message);
    }
    
    static void Info(const std::string& message) {
        Log(Level::Info, message);
    }
    
    static void InfoWithLocation(const std::string& message, const char* file, int line) {
        LogWithLocation(Level::Info, message, file, line);
    }
    
private:
    static std::string ExtractFilename(const char* fullPath) {
        std::string path(fullPath);
        size_t lastSlash = path.find_last_of("/\\");
        return lastSlash != std::string::npos ? path.substr(lastSlash + 1) : path;
    }
};

// Macro definitions similar to the ones we added to the KAI system
#define LOG_INFO(msg) Logger::Info(msg)
#define LOG_INFO_WITH_LOCATION(msg) Logger::InfoWithLocation(msg, __FILE__, __LINE__)

int main() {
    std::cout << "Testing logging with file and line information..." << std::endl;
    
    // Standard logging without location
    LOG_INFO("Standard log message without location");
    
    // Enhanced logging with location
    LOG_INFO_WITH_LOCATION("Enhanced log message with file and line information");
    
    // Direct call with location
    Logger::InfoWithLocation("Direct call with location", __FILE__, __LINE__);
    
    // Multiple calls from different locations
    LOG_INFO_WITH_LOCATION("First location in main");
    
    {
        // This should show a different line number
        LOG_INFO_WITH_LOCATION("Second location in a nested block");
    }
    
    std::cout << "Test completed successfully." << std::endl;
    return 0;
}