#include <KAI/Core/Logger.h>
#include <KAI/Network/NetworkLogger.h>

#include <KAI/Console/rang.hpp>

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

// Get appropriate color for each network category
rang::fg GetColorForCategory(NetworkLogger::Category category) {
    switch (category) {
        case NetworkLogger::Category::Connection:
            return rang::fg::magenta;
        case NetworkLogger::Category::Message:
            return rang::fg::blue;
        case NetworkLogger::Category::Discovery:
            return rang::fg::cyan;
        case NetworkLogger::Category::Status:
            return rang::fg::green;
        default:
            return rang::fg::blue;
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

    // Create formatted message with category prefix
    std::string categoryStr = CategoryToString(category);
    std::string formattedMessage = categoryStr + ": " + message;

    // Get color for this category
    rang::fg color = GetColorForCategory(category);

    // Log to the console with color
    std::cout << rang::style::bold << color << "[NETWORK] " << formattedMessage
              << rang::style::reset << rang::fg::reset << std::endl;

    // Also log to the main logger (which will handle file output)
    Logger::Info(formattedMessage);

    // Category-specific file is handled by the Logger
}

void NetworkLogger::LogWithLocation(Category category,
                                    const std::string& message,
                                    const char* file, int line) {
    if (!s_initialized) {
        Init();
    }

    // Create location string
    std::string filename = ExtractFilename(file);
    std::string locationInfo =
        "[" + filename + ":" + std::to_string(line) + "] ";

    // Create formatted message with category prefix and location
    std::string categoryStr = CategoryToString(category);
    std::string formattedMessage = categoryStr + ": " + locationInfo + message;

    // Get color for this category
    rang::fg color = GetColorForCategory(category);

    // Log to the console with color
    std::cout << rang::style::bold << color << "[NETWORK] " << formattedMessage
              << rang::style::reset << rang::fg::reset << std::endl;

    // Also log to the main logger (which will handle file output)
    Logger::Info(formattedMessage);
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
void NetworkLogger::LogConnectionWithLocation(const std::string& message,
                                              const char* file, int line) {
    LogWithLocation(Category::Connection, message, file, line);
}

void NetworkLogger::LogMessageWithLocation(const std::string& message,
                                           const char* file, int line) {
    LogWithLocation(Category::Message, message, file, line);
}

void NetworkLogger::LogDiscoveryWithLocation(const std::string& message,
                                             const char* file, int line) {
    LogWithLocation(Category::Discovery, message, file, line);
}

void NetworkLogger::LogStatusWithLocation(const std::string& message,
                                          const char* file, int line) {
    LogWithLocation(Category::Status, message, file, line);
}

KAI_NET_END