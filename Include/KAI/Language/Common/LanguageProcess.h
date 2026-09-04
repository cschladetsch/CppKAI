#pragma once

#include <KAI/Core/Config/Base.h>

#include <string>

KAI_BEGIN

// Common to all language processes. A simple and humble class with faults
// but crucial to the overall system.
struct Process {
    virtual ~Process() = default;
    static int trace;

    mutable bool Failed = false;
    mutable std::string Error;

   protected:
    bool Fail(const char *fmt, ...) const;
    bool Fail(const std::string &) const;

    // Object FailObject(const std::string &text) const
    //{
    //     KAI_TRACE_ERROR_1(Fail(text));
    //     return Object();
    // }
};

KAI_END
