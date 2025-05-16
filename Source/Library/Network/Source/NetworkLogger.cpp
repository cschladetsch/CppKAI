#include <KAI/Core/Logger.h>
#include <KAI/Network/NetworkLogger.h>

KAI_NET_BEGIN

bool NetworkLogger::s_initialized = false;

void NetworkLogger::Init(const std::string& logDirectory) {
    // Initialize the main logger
    Logger::Init(logDirectory);
    s_initialized = true;
}

std::string NetworkLogger::CategoryToString(Category category) {
    switch (category) {
        case Category::Connection:
            return "connection";
        case Category::Message:
            return "message";
        case Category::Discovery:
            return "discovery";
        case Category::Status:
            return "status";
        default:
            return "network";
    }
}

std::string NetworkLogger::GetLogFilename(Category category) {
    return Logger::GetLogFilename("network_" + CategoryToString(category));
}

// Extract just the filename from a full path
std::string NetworkLogger::ExtractFilename(const char* fullPath) {
    std::string path(fullPath);
    size_t lastSlash = path.find_last_of("/\\");
    return lastSlash != std::string::npos ? path.substr(lastSlash + 1) : path;
}

void NetworkLogger::Log(Category category, const std::string& message) {
    if (!s_initialized) {
        Init();
    }

    // Log to the main logger
    Logger::Info(CategoryToString(category) + ": " + message);

    // Also log to category-specific file
    std::string filename = GetLogFilename(category);
    // Logger will handle writing to the specific file
}

void NetworkLogger::LogWithLocation(Category category, const std::string& message, const char* file, int line) {
    if (!s_initialized) {
        Init();
    }

    // Create location string
    std::string locationInfo = "[" + ExtractFilename(file) + ":" + std::to_string(line) + "] ";
    
    // Log with location information
    Logger::Info(CategoryToString(category) + ": " + locationInfo + message);
}

void NetworkLogger::LogConnection(const std::string& message) {
    Log(Category::Connection, message);
}

void NetworkLogger::LogMessage(const std::string& message) {
    Log(Category::Message, message);
}

void NetworkLogger::LogDiscovery(const std::string& message) {
    Log(Category::Discovery, message);
}

void NetworkLogger::LogStatus(const std::string& message) {
    Log(Category::Status, message);
}

// Location-aware logging methods
void NetworkLogger::LogConnectionWithLocation(const std::string& message, const char* file, int line) {
    LogWithLocation(Category::Connection, message, file, line);
}

void NetworkLogger::LogMessageWithLocation(const std::string& message, const char* file, int line) {
    LogWithLocation(Category::Message, message, file, line);
}

void NetworkLogger::LogDiscoveryWithLocation(const std::string& message, const char* file, int line) {
    LogWithLocation(Category::Discovery, message, file, line);
}

void NetworkLogger::LogStatusWithLocation(const std::string& message, const char* file, int line) {
    LogWithLocation(Category::Status, message, file, line);
}

KAI_NET_END