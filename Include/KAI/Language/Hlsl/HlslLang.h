#pragma once

#include <KAI/Language/Common/LangCommon.h>
#include <KAI/Language/Common/Structure.h>
#include <KAI/Language/Hlsl/HlslTranslator.h>

KAI_BEGIN

class HlslLang : Lang<HlslTranslator> {
    typedef Lang<HlslTranslator> Parent;

    HlslLang(Registry &r) : Parent(r) {}

    Pointer<Continuation> Translate(const char *text,
                                    Structure st = Structure::Expression);
    Pointer<Continuation> TranslateFile(const char *name,
                                        Structure st = Structure::Program);

    void Print();

   protected:
    using Parent::reg;
};

KAI_END
