#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "KAI/Core/Base.h"

KAI_BEGIN

class Logger {
   public:
    enum class Level { Debug, Info, Warning, Error, Fatal };

    // Uses the KAI_LOG_DIR define from CMake if available
    static void Init(const std::string& logDirectory = "");
    static bool IsInitialized();

    static void SetLevel(Level level);
    static Level GetLevel();

    static void Log(Level level, const std::string& message);
    static void Debug(const std::string& message);
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);
    static void Fatal(const std::string& message);

    // Helper to get a filename for a specific module
    static std::string GetLogFilename(const std::string& module);

   private:
    static std::string LevelToString(Level level);
    static bool ShouldLog(Level level);

    static Level s_level;
    static std::string s_logDirectory;
    static bool s_initialized;
};

KAI_END