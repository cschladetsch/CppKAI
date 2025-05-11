#include <KAI/Network/NetworkLogger.h>
#include <KAI/Core/Logger.h>

KAI_NET_BEGIN

bool NetworkLogger::s_initialized = false;

void NetworkLogger::Init(const std::string& logDirectory) {
    // Initialize the main logger
    Logger::Init(logDirectory);
    s_initialized = true;
}

std::string NetworkLogger::CategoryToString(Category category) {
    switch (category) {
        case Category::Connection: return "connection";
        case Category::Message: return "message";
        case Category::Discovery: return "discovery";
        case Category::Status: return "status";
        default: return "network";
    }
}

std::string NetworkLogger::GetLogFilename(Category category) {
    return Logger::GetLogFilename("network_" + CategoryToString(category));
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

KAI_NET_END