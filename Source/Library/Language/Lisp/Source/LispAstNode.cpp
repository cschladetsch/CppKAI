#include "KAI/Language/Lisp/LispAstNode.h"

KAI_BEGIN

const char *LispAstNodeEnumType::ToString(Enum val) {
    switch (val) {
        case None:
            return "None";
        case TokenType:
            return "Atom";
        case Program:
            return "Program";
        case List:
            return "List";
        case Quote:
            return "Quote";
        case Object:
            return "Object";
    }

    return "UnknownLispAstNode";
}

KAI_END
