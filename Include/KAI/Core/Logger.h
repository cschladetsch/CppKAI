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

    // Standard logging methods
    static void Log(Level level, const std::string& message);
    static void Debug(const std::string& message);
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);
    static void Fatal(const std::string& message);

    // Enhanced logging with file and line information
    static void LogWithLocation(Level level, const std::string& message,
                                const char* file, int line);
    static void DebugWithLocation(const std::string& message, const char* file,
                                  int line);
    static void InfoWithLocation(const std::string& message, const char* file,
                                 int line);
    static void WarningWithLocation(const std::string& message,
                                    const char* file, int line);
    static void ErrorWithLocation(const std::string& message, const char* file,
                                  int line);
    static void FatalWithLocation(const std::string& message, const char* file,
                                  int line);

    // Helper to get a filename for a specific module
    static std::string GetLogFilename(const std::string& module);

    // Configuration for console output
    static void SetConsoleOutputForInfoDebug(bool enabled);
    static bool GetConsoleOutputForInfoDebug();

   private:
    static std::string LevelToString(Level level);
    static bool ShouldLog(Level level);

    static Level s_level;
    static std::string s_logDirectory;
    static bool s_initialized;
    static bool s_consoleOutputForInfoDebug;
};

KAI_END