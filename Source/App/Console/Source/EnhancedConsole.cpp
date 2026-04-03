#include "EnhancedConsole.h"

namespace kai {

EnhancedConsole::EnhancedConsole() : Console() {
    // Initialize translators will be done lazily when needed
}

Pointer<Continuation> EnhancedConsole::Translate(const String &text,
                                                 Structure st) {
    if (text.empty()) return Object();

    switch (GetLanguage()) {
        case Language::None:
            return Object();

        case Language::Pi: {
            if (!piTranslator.Exists()) {
                piTranslator = reg_->New<PiTranslator>(*reg_);
            }
            piTranslator->SetTraceLevel(compiler->GetTraceLevel());
            piTranslator->SetStructure(st);
            auto result = piTranslator->Translate(text);
            if (piTranslator->Failed) {
                KAI_TRACE_ERROR() << piTranslator->Error;
                return Object();
            }
            return result;
        }

        case Language::Rho: {
            if (!rhoTranslator.Exists()) {
                rhoTranslator = reg_->New<RhoTranslator>(*reg_);
            }
            rhoTranslator->SetTraceLevel(compiler->GetTraceLevel());
            rhoTranslator->SetStructure(st);
            auto result = rhoTranslator->Translate(text);
            if (rhoTranslator->Failed) {
                KAI_TRACE_ERROR() << rhoTranslator->Error;
                return Object();
            }
            return result;
        }

        case Language::Tau:
            KAI_NOT_IMPLEMENTED();  // Tau is not a compiled language
            break;
    }

    return Object();
}

}  // namespace kai