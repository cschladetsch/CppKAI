#pragma once

#include <KAI/Language/Common/LangCommon.h>
#include <KAI/Language/Pi/PiTranslator.h>

KAI_BEGIN

// the specialised PiLang translator.
class PiLang : Lang<PiTranslator> {
    typedef Lang<PiTranslator> Parent;

    PiLang(Registry &r) : Parent(r) {}

    Pointer<Executor> Exec(const char *text,
                           Structure st = Structure::Expression);
    Pointer<Continuation> Translate(const char *text,
                                    Structure st = Structure::Expression);
    Pointer<Continuation> TranslateFile(const char *name,
                                        Structure st = Structure::Program);

    void Print();
};

KAI_END
