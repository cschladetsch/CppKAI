#include "KAI/Core/Logger.h"

KAI_BEGIN

// Initialize static members
Logger::Level Logger::s_level = Logger::Level::Info;
std::string Logger::s_logDirectory = "/home/xian/local/KAI/Logs";
bool Logger::s_initialized = false;

void Logger::Init(const std::string& logDirectory) {
    s_logDirectory = logDirectory;
    
    // Create the logs directory if it doesn't exist
    try {
        std::filesystem::create_directories(s_logDirectory);
        s_initialized = true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to create log directory: " << e.what() << std::endl;
        s_initialized = false;
    }
}

bool Logger::IsInitialized() {
    return s_initialized;
}

void Logger::SetLevel(Level level) {
    s_level = level;
}

Logger::Level Logger::GetLevel() {
    return s_level;
}

std::string Logger::LevelToString(Level level) {
    switch (level) {
        case Level::Debug:   return "DEBUG";
        case Level::Info:    return "INFO";
        case Level::Warning: return "WARNING";
        case Level::Error:   return "ERROR";
        case Level::Fatal:   return "FATAL";
        default:             return "UNKNOWN";
    }
}

bool Logger::ShouldLog(Level level) {
    return static_cast<int>(level) >= static_cast<int>(s_level);
}

void Logger::Log(Level level, const std::string& message) {
    if (!ShouldLog(level)) {
        return;
    }
    
    // Create timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream timestamp;
    timestamp << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    
    // Format message
    std::string levelStr = LevelToString(level);
    std::string formattedMessage = "[" + timestamp.str() + "] [" + levelStr + "] " + message;
    
    // Print to console
    std::cout << formattedMessage << std::endl;
    
    // Ensure the log directory exists
    if (!IsInitialized()) {
        Init();
    }
    
    // Write to file
    if (IsInitialized()) {
        std::string logFilename = s_logDirectory + "/kai.log";
        std::ofstream logFile(logFilename, std::ios_base::app);
        if (logFile) {
            logFile << formattedMessage << std::endl;
            logFile.close();
        }
    }
}

std::string Logger::GetLogFilename(const std::string& module) {
    if (!IsInitialized()) {
        Init();
    }
    
    return s_logDirectory + "/" + module + ".log";
}

void Logger::Debug(const std::string& message) {
    Log(Level::Debug, message);
}

void Logger::Info(const std::string& message) {
    Log(Level::Info, message);
}

void Logger::Warning(const std::string& message) {
    Log(Level::Warning, message);
}

void Logger::Error(const std::string& message) {
    Log(Level::Error, message);
}

void Logger::Fatal(const std::string& message) {
    Log(Level::Fatal, message);
}

KAI_END