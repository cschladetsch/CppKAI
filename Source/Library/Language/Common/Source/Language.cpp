#include <KAI/Base.h>
#include <KAI/Language/Common/Language.h>

KAI_BEGIN

const char *ToString(Language lang) {
    switch (lang) {
        case Language::None:
            return "None";
        case Language::Pi:
            return "Pi";
        case Language::Rho:
            return "Rho";
        case Language::Tau:
            return "Tau";
        case Language::Hlsl:
            return "Hlsl";
        case Language::Sigma:
            return "Sigma";
        default:
            return "Unknown";
    }
}

KAI_END