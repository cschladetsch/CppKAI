#pragma once

#include <KAI/Language/Common/LangCommon.h>
#include <KAI/Language/Common/Structure.h>
#include <KAI/Language/Lisp/LispTranslator.h>

KAI_BEGIN

class LispLang : Lang<LispTranslator> {
    typedef Lang<LispTranslator> Parent;

    LispLang(Registry &r) : Parent(r) {}

    Pointer<Executor> Exec(const char *text,
                           Structure st = Structure::Expression);
    Pointer<Continuation> Translate(const char *text,
                                    Structure st = Structure::Expression);
    Pointer<Continuation> TranslateFile(const char *name,
                                        Structure st = Structure::Program);

    void Print() const;

   protected:
    using Parent::reg;
};

KAI_END
