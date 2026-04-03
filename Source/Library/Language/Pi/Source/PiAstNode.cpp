#include "KAI/Language/Pi/PiAstNode.h"

#include <stdio.h>
#include <stdlib.h>

#include "KAI/Executor/Executor.h"
#include "KAI/Language/Pi/PiToken.h"

KAI_BEGIN

const char* PiAstNodeEnumType::ToString(Enum ty) {
    switch (ty) {
#define CASE(X)                      \
    case PiAstNodeEnumType::Enum::X: \
        return #X;
        CASE(Program)
        CASE(Operation)
        CASE(List)
        CASE(Map)
        CASE(Set)
        CASE(Array)
        CASE(Continuation)

        // Special case to avoid noisey 'TokenType:' debug traces.
        case PiAstNodeEnumType::Enum::TokenType:
            return "";

        case PiAstNodeEnumType::Enum::None:
            return "";
    }

    // sorry :'(
    static char b[100];
    sprintf(b, "%d", (int)ty);
    return b;
}

std::ostream& operator<<(std::ostream& out, const PiAstNode& node) {
    return out << node.ToString();
}

KAI_END
