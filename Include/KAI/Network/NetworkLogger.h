#pragma once

#include <KAI/Core/Base.h>
#include <KAI/Core/Logger.h>
#include <KAI/Network/Config.h>

#include <string>

KAI_NET_BEGIN

class NetworkLogger {
   public:
    enum class Category { Connection, Message, Discovery, Status };

    static void Init(
        const std::string& logDirectory = "/home/xian/local/KAI/Logs");

    // Standard logging methods
    static void LogConnection(const std::string& message);
    static void LogMessage(const std::string& message);
    static void LogDiscovery(const std::string& message);
    static void LogStatus(const std::string& message);

    static void Log(Category category, const std::string& message);

    // Enhanced logging methods with file and line information
    static void LogConnectionWithLocation(const std::string& message,
                                          const char* file, int line);
    static void LogMessageWithLocation(const std::string& message,
                                       const char* file, int line);
    static void LogDiscoveryWithLocation(const std::string& message,
                                         const char* file, int line);
    static void LogStatusWithLocation(const std::string& message,
                                      const char* file, int line);

    static void LogWithLocation(Category category, const std::string& message,
                                const char* file, int line);

   private:
    static std::string CategoryToString(Category category);
    static std::string GetLogFilename(Category category);
    static std::string ExtractFilename(const char* fullPath);

    static bool s_initialized;
};

// Macros for network logging with file and line information
#define KAI_NET_LOG_CONNECTION(msg)                                  \
    KAI_NAMESPACE(Network::NetworkLogger::LogConnectionWithLocation) \
    (msg, __FILE__, __LINE__)
#define KAI_NET_LOG_MESSAGE(msg)                                  \
    KAI_NAMESPACE(Network::NetworkLogger::LogMessageWithLocation) \
    (msg, __FILE__, __LINE__)
#define KAI_NET_LOG_DISCOVERY(msg)                                  \
    KAI_NAMESPACE(Network::NetworkLogger::LogDiscoveryWithLocation) \
    (msg, __FILE__, __LINE__)
#define KAI_NET_LOG_STATUS(msg)                                  \
    KAI_NAMESPACE(Network::NetworkLogger::LogStatusWithLocation) \
    (msg, __FILE__, __LINE__)

KAI_NET_END