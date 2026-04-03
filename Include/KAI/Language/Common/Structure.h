#pragma once

#include <KAI/Core/Base.h>

KAI_BEGIN

// A syntactic structure.
enum class Structure {
    None,
    Single,      // an expression or such
    Sequence,    // a contaniner or such
    Module,      // a self-contianed collection of reflected things
    Statement,   // a simple statement with no context
    Expression,  // an expression using operators
    Function,    // a thing that gives output given some or no input
    Program,     // deprecated. to be replaced by module/namespace
    Namespace,   // a scoped collection of things
    Class,       // a collection of properties_, events, and methods_
};

StringStream &operator<<(StringStream &S, const Structure &T);
StringStream &operator>>(StringStream &S, Structure &T);

KAI_END
