#include "KAI/Core/Logger.h"

#include <chrono>
#include <filesystem>
#include <sstream>
#include <thread>

// Include the rang.hpp header for colored output
#include "rang.hpp"

// Choose the appropriate format implementation
#ifdef KAI_FORMAT_COMPATIBLE
#include <format>
#else
#include <iomanip>
#endif

KAI_BEGIN

// Initialize static members with the centralized log directory from CMake
#ifdef KAI_LOG_DIR
std::string default_log_dir = KAI_LOG_DIR;
#else
// Fallback to a default logs directory in the user's home folder
std::string default_log_dir = std::string(getenv("HOME")) + "/local/KAI/Logs";
#endif

Logger::Level Logger::s_level = Logger::Level::Info;
std::string Logger::s_logDirectory = default_log_dir;
bool Logger::s_initialized = false;
bool Logger::s_consoleOutputForInfoDebug =
    false;  // Default: INFO/DEBUG only go to files

// Helper function to create a formatted timestamp, compatible with older
// compilers
std::string FormatTimestamp(const std::tm& time_info) {
#ifdef KAI_FORMAT_COMPATIBLE
    try {
        return std::format("{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
                           time_info.tm_mon + 1, time_info.tm_mday,
                           time_info.tm_hour, time_info.tm_min,
                           time_info.tm_sec);
    } catch (...) {
        // Fallback if format fails
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << (time_info.tm_mon + 1) << "-"
           << std::setw(2) << time_info.tm_mday << " " << std::setw(2)
           << time_info.tm_hour << ":" << std::setw(2) << time_info.tm_min
           << ":" << std::setw(2) << time_info.tm_sec;
        return ss.str();
    }
#else
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << (time_info.tm_mon + 1) << "-"
       << std::setw(2) << time_info.tm_mday << " " << std::setw(2)
       << time_info.tm_hour << ":" << std::setw(2) << time_info.tm_min << ":"
       << std::setw(2) << time_info.tm_sec;
    return ss.str();
#endif
}

// Get the appropriate color for a log level
rang::fg GetColorForLevel(Logger::Level level) {
    switch (level) {
        case Logger::Level::Debug:
            return rang::fg::cyan;
        case Logger::Level::Info:
            return rang::fg::green;
        case Logger::Level::Warning:
            return rang::fg::yellow;
        case Logger::Level::Error:
            return rang::fg::red;
        case Logger::Level::Fatal:
            return rang::fg::red;
        default:
            return rang::fg::reset;
    }
}

// Get the appropriate style for a log level
rang::style GetStyleForLevel(Logger::Level level) {
    if (level == Logger::Level::Fatal) {
        return rang::style::bold;
    }
    if (level == Logger::Level::Error) {
        return rang::style::bold;
    }
    return rang::style::reset;
}

void Logger::Init(const std::string& logDirectory) {
    // If a specific directory is provided, use it
    // Otherwise keep using the default from KAI_LOG_DIR
    if (!logDirectory.empty()) {
        s_logDirectory = logDirectory;
    }

    // Create the logs directory if it doesn't exist
    try {
        std::filesystem::create_directories(s_logDirectory);
        s_initialized = true;

        // Write a startup marker to the log
        std::string startupFile = s_logDirectory + "/kai_startup.log";
        std::ofstream logFile(startupFile, std::ios_base::app);
        if (logFile) {
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            auto localtime = *std::localtime(&time_t_now);
            auto threadId = std::this_thread::get_id();

            // Format message using compatibility function
            std::string timestamp = FormatTimestamp(localtime);
            std::stringstream ss;
            ss << "[" << timestamp
               << "] Logger initialized. Log directory: " << s_logDirectory
               << ", Thread ID: " << std::hex << std::showbase
               << reinterpret_cast<uintptr_t>(&threadId);

            logFile << ss.str() << std::endl;
            logFile.close();
        }
    } catch (const std::exception& e) {
        std::cerr << rang::fg::red
                  << "Failed to create log directory: " << e.what()
                  << rang::fg::reset << std::endl;
        s_initialized = false;
    }
}

bool Logger::IsInitialized() { return s_initialized; }

void Logger::SetLevel(Level level) { s_level = level; }

Logger::Level Logger::GetLevel() { return s_level; }

void Logger::SetConsoleOutputForInfoDebug(bool enabled) {
    s_consoleOutputForInfoDebug = enabled;
}

bool Logger::GetConsoleOutputForInfoDebug() {
    return s_consoleOutputForInfoDebug;
}

std::string Logger::LevelToString(Level level) {
    switch (level) {
        case Level::Debug:
            return "DEBUG";
        case Level::Info:
            return "INFO ";
        case Level::Warning:
            return "WARN ";
        case Level::Error:
            return "ERROR";
        case Level::Fatal:
            return "FATAL";
        default:
            return "UNKN ";
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
    auto localtime = *std::localtime(&time_t_now);

    // Use the format helper to ensure compatibility
    std::string timestamp = FormatTimestamp(localtime);
    std::string levelStr = LevelToString(level);

    // Create formatted message
    std::stringstream ss;
    ss << "[" << timestamp << "] [" << levelStr << "] " << message;
    std::string formattedMessage = ss.str();

    // Get color and style for this log level
    rang::fg color = GetColorForLevel(level);
    rang::style style = GetStyleForLevel(level);

    // Determine if we should output to console
    bool outputToConsole = false;
    if (level == Level::Warning || level == Level::Error ||
        level == Level::Fatal) {
        // Always output warnings, errors, and fatal to console
        outputToConsole = true;
    } else if ((level == Level::Info || level == Level::Debug) &&
               s_consoleOutputForInfoDebug) {
        // Only output info/debug to console if explicitly enabled
        outputToConsole = true;
    }

    // Print to console if needed
    if (outputToConsole) {
        if (level == Level::Error || level == Level::Fatal) {
            std::cerr << rang::fg::gray << "[" << timestamp << "] " << style
                      << color << "[" << levelStr << "] " << rang::style::reset
                      << rang::fg::reset << message << std::endl;
        } else {
            std::cout << rang::fg::gray << "[" << timestamp << "] " << style
                      << color << "[" << levelStr << "] " << rang::style::reset
                      << rang::fg::reset << message << std::endl;
        }
    }

    // Ensure the log directory exists
    if (!IsInitialized()) {
        Init();
    }

    // Write to appropriate log files
    if (IsInitialized()) {
        // Write to main log file (without colors)
        std::string mainLogFilename = s_logDirectory + "/kai.log";
        std::ofstream mainLogFile(mainLogFilename, std::ios_base::app);
        if (mainLogFile) {
            mainLogFile << formattedMessage << std::endl;
            mainLogFile.close();
        }

        // Also write to level-specific log file for errors and fatal messages
        if (level == Level::Error || level == Level::Fatal) {
            std::string errorLogFilename = s_logDirectory + "/errors.log";
            std::ofstream errorLogFile(errorLogFilename, std::ios_base::app);
            if (errorLogFile) {
                errorLogFile << formattedMessage << std::endl;
                errorLogFile.close();
            }
        }
    }
}

std::string Logger::GetLogFilename(const std::string& module) {
    if (!IsInitialized()) {
        Init();
    }

    return s_logDirectory + "/" + module + ".log";
}

void Logger::Debug(const std::string& message) { Log(Level::Debug, message); }

void Logger::Info(const std::string& message) { Log(Level::Info, message); }

void Logger::Warning(const std::string& message) {
    Log(Level::Warning, message);
}

void Logger::Error(const std::string& message) { Log(Level::Error, message); }

void Logger::Fatal(const std::string& message) { Log(Level::Fatal, message); }

// Helper function to extract filename from path
std::string ExtractFilename(const char* fullPath) {
    std::string path(fullPath);
    size_t lastSlash = path.find_last_of("/\\");
    return lastSlash != std::string::npos ? path.substr(lastSlash + 1) : path;
}

// Enhanced versions with file and line information
void Logger::LogWithLocation(Level level, const std::string& message,
                             const char* file, int line) {
    // Create location string with highlighted filename and line number
    std::string filename = ExtractFilename(file);
    std::string locationInfo =
        "[" + filename + ":" + std::to_string(line) + "] ";

    // Log with location information
    Log(level, locationInfo + message);
}

void Logger::DebugWithLocation(const std::string& message, const char* file,
                               int line) {
    LogWithLocation(Level::Debug, message, file, line);
}

void Logger::InfoWithLocation(const std::string& message, const char* file,
                              int line) {
    LogWithLocation(Level::Info, message, file, line);
}

void Logger::WarningWithLocation(const std::string& message, const char* file,
                                 int line) {
    LogWithLocation(Level::Warning, message, file, line);
}

void Logger::ErrorWithLocation(const std::string& message, const char* file,
                               int line) {
    LogWithLocation(Level::Error, message, file, line);
}

void Logger::FatalWithLocation(const std::string& message, const char* file,
                               int line) {
    LogWithLocation(Level::Fatal, message, file, line);
}

KAI_END
