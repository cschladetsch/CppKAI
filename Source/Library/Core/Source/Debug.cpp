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
bool Trace::TraceFileLocation = false;
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
    logMessage += std::string(TypeToString(type)) + ": " + val.c_str();

    // Log using the centralized Logger
    Logger::Log(TraceTypeToLoggerLevel(type), logMessage);

    // Get appropriate colors based on trace type
    rang::fg typeColor;
    switch (type) {
        case Information:
            typeColor = rang::fg::green;
            break;
        case Warn:
            typeColor = rang::fg::yellow;
            break;
        case Error:
            typeColor = rang::fg::red;
            break;
        case Fatal:
            typeColor = rang::fg::red;
            break;
        default:
            typeColor = rang::fg::reset;
    }
    
    // Style for file location
    const auto filelocColor = rang::fg::gray;
    
    // Also output to console with colors (for terminal output)
    if (TraceFileLocation) {
        cout << filelocColor
             << file_location.ToString().c_str() << " ";
    }

    cout << rang::style::bold << typeColor << "[" << TypeToString(type) << "] " 
         << rang::style::reset << rang::fg::reset << val.c_str() 
         << std::endl;
}

}  // namespace debug

KAI_END