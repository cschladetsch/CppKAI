#pragma once

#include <KAI/Console/Console.h>
#include <KAI/Language/Pi/PiTranslator.h>
#include <KAI/Language/Rho/RhoTranslator.h>

namespace kai {
namespace test {

// Helper function to set up language translators for a Console instance
inline void SetupConsoleTranslators(Console& console) {
    auto compiler = console.GetCompiler();
    if (!compiler.Exists()) {
        return;
    }

    auto& reg = console.GetRegistry();

    // Create translators for each language as shared pointers
    auto piTranslator = std::make_shared<PiTranslator>(reg);
    auto rhoTranslator = std::make_shared<RhoTranslator>(reg);

    // Set up the translation function
    compiler->SetTranslateFunction(
        [=](const String& text, Structure st) -> Pointer<Continuation> {
            int lang = compiler->GetLanguage();
            int traceLevel = compiler->GetTraceLevel();

            switch (static_cast<Language>(lang)) {
                case Language::Pi: {
                    piTranslator->trace = traceLevel;
                    auto result = piTranslator->Translate(text.c_str(), st);
                    if (piTranslator->Failed) {
                        KAI_TRACE_ERROR() << piTranslator->Error;
                        return Object();
                    }
                    return result;
                }
                case Language::Rho: {
                    rhoTranslator->trace = traceLevel;
                    auto result = rhoTranslator->Translate(text.c_str(), st);
                    if (rhoTranslator->Failed) {
                        KAI_TRACE_ERROR() << rhoTranslator->Error;
                        return Object();
                    }
                    return result;
                }
                default:
                    return Object();
            }
        });
}

}  // namespace test
}  // namespace kai