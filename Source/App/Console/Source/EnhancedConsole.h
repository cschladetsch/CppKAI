#pragma once

#include "KAI/Console/Console.h"
#include "KAI/Language/Pi/PiTranslator.h"
#include "KAI/Language/Rho/RhoTranslator.h"

namespace kai {

class EnhancedConsole : public Console {
   public:
    EnhancedConsole();

    // Override to provide language-specific translation
    virtual Pointer<Continuation> Translate(
        const String &text, Structure st = Structure::Expression) override;

   private:
    // Cache translators for each language
    mutable Pointer<PiTranslator> piTranslator;
    mutable Pointer<RhoTranslator> rhoTranslator;
};

}  // namespace kai