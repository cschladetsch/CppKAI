#pragma once

#include <KAI/Core/BuiltinTypes/String.h>

KAI_BEGIN

/// A location in a source file represented as the file name_ and a line number
struct FileLocation {
    String file;
    String function;
    int line;

    FileLocation() : line(0) {}
    FileLocation(const char *F, int L, const char *G = "")
        : file(F), line(L), function(G) {}
    FileLocation(const char *G) : function(G), line(0) {}

    String ToString() const;
    void AddLocation(StringStream &) const;
    void AddFunction(StringStream &) const;
};

KAI_END
