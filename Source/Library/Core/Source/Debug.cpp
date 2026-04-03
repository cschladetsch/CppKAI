#include <KAI/Console/ConsoleColor.h>
#include <KAI/Core/Config/Base.h>
#include <KAI/Core/Debug.h>
#include <KAI/Core/Exception.h>
#include <KAI/Core/Logger.h>
#include <KAI/Core/Object/Object.h>

#include <iostream>

#include "rang.hpp"

using namespace std;

KAI_BEGIN

StringStream& operator<<(StringStream& s, const Structure& st) {
    KAI_NOT_IMPLEMENTED();
}

// this is just being used while I (re)sort out the cross-platform console color
// issues
ostream& operator<<(ostream& S, ConsoleColor::EType type) { return S; }

namespace debug {
bool Trace::TraceFileLocation = true;  // Always show file and line information
bool Trace::StripPath = true;
bool Trace::TraceFunction = false;

void MaxTrace() {
    Trace::TraceFileLocation = true;
    Trace::StripPath = false;
    Trace::TraceFunction = true;
}

void MinTrace() {
    Trace::TraceFileLocation = true;
    Trace::StripPath = true;
    Trace::TraceFunction = true;
}

StringStream& Trace::operator<<(Object const& obj) {
    return *this << obj.ToString();
}

const char* TypeToString(Trace::Type type) {
    switch (type) {
#define CASE(V)    \
    case Trace::V: \
        return #V;
        CASE(Warn);
        CASE(Error);
        CASE(Fatal);
        case Trace::Information:
            return "Info";
    }

    return "??";
}

// Convert Trace type to Logger level
Logger::Level TraceTypeToLoggerLevel(Trace::Type type) {
    switch (type) {
        case Trace::Information:
            return Logger::Level::Info;
        case Trace::Warn:
            return Logger::Level::Warning;
        case Trace::Error:
            return Logger::Level::Error;
        case Trace::Fatal:
            return Logger::Level::Fatal;
        default:
            return Logger::Level::Info;
    }
}

Trace::~Trace() {
    const auto val = ToString();

    // Ensure Logger is initialized
    if (!Logger::IsInitialized()) {
        Logger::Init();
    }

    // Create formatted message
    std::string logMessage;
    if (TraceFileLocation) {
        logMessage = file_location.ToString().c_str();
        logMessage += " ";
    }
    // Don't add the log level to the message as Logger will do this
    // automatically
    logMessage += val.c_str();

    // Log using the centralized Logger
    Logger::Log(TraceTypeToLoggerLevel(type), logMessage);

    // We removed explicit console output to avoid duplication
    // Logger will handle the coloring based on log level

    // We'll let the Logger handle the console output to avoid duplication
    // The Logger already prints to console with appropriate colors
}

}  // namespace debug

KAI_END