#pragma once

#include <KAI/Core/Config/Base.h>

#ifdef KAI_
#include <ostream>

KAI_BEGIN

struct Color {
    enum Type {
        Prompt,
        Input,
        Trace,
        Error,
        Warning,
        StackEntry,
        StackIndex,
        LanguageName,
        NumObjects,
        Pathname,
        Last
    };

    Color();
    ~Color();

    static void SetColor(Type c);
};

inline std::ostream& operator<<(std::ostream& out, Color::Type c) {
    Color::SetColor(c);
    return out;
}

KAI_END
