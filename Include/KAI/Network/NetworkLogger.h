#pragma once

#include <string>
#include <KAI/Core/Base.h>
#include <KAI/Core/Logger.h>
#include <KAI/Network/Config.h>

KAI_NET_BEGIN

class NetworkLogger {
public:
    enum class Category {
        Connection,
        Message,
        Discovery,
        Status
    };
    
    static void Init(const std::string& logDirectory = "/home/xian/local/KAI/Logs");
    
    static void LogConnection(const std::string& message);
    static void LogMessage(const std::string& message);
    static void LogDiscovery(const std::string& message);
    static void LogStatus(const std::string& message);
    
    static void Log(Category category, const std::string& message);
    
private:
    static std::string CategoryToString(Category category);
    static std::string GetLogFilename(Category category);
    
    static bool s_initialized;
};

KAI_NET_END