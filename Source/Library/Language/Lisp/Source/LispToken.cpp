#include "KAI/Language/Lisp/LispToken.h"

#include <ostream>

KAI_BEGIN

const char *LispTokenEnumType::ToString(Enum val) {
    switch (val) {
        case None:
            return "None";
        case Whitespace:
            return "Whitespace";
        case Tab:
            return "Tab";
        case NewLine:
            return "NewLine";
        case Comment:
            return "Comment";
        case OpenParan:
            return "OpenParan";
        case CloseParan:
            return "CloseParan";
        case Quote:
            return "Quote";
        case Int:
            return "Int";
        case Float:
            return "Float";
        case String:
            return "String";
        case True:
            return "True";
        case False:
            return "False";
        case Nil:
            return "Nil";
        case Ident:
            return "Ident";
    }

    return "UnknownLispToken";
}

std::ostream &operator<<(std::ostream &out, LispToken const &token) {
    out << LispTokenEnumType::ToString(token.type);
    const auto text = token.Text();
    if (!text.empty()) out << "='" << text << "'";
    return out;
}

KAI_END
