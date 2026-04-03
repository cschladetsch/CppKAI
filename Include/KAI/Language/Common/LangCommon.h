#pragma once

#include <KAI/Core/Registry.h>
#include <KAI/Executor/Executor.h>
#include <KAI/Language/Common/Base.h>
#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Common/Process.h>
#include <KAI/Language/Common/Structure.h>

KAI_BEGIN

// Common for all all languages, given the Translator Tr
// which transforms raw text into a Continuation for an Executor
template <class Tr>
class Lang : public Process {
   public:
    typedef Tr Translator;
    typedef typename Tr::Lexer Lexer;
    typedef typename Tr::Parser Parser;

   protected:
    Registry &reg;
    std::shared_ptr<Lexer> lex;
    std::shared_ptr<Parser> parse;
    std::shared_ptr<Translator> trans;
    Pointer<Executor> exec;

   public:
    Lang(const Lang &) = delete;
    Lang(Registry &r) : reg(r) {}

    virtual Pointer<Executor> Exec(const char *text,
                                   Structure st = Structure::Expression) = 0;
    virtual Pointer<Continuation> Translate(
        const char *text, Structure st = Structure::Expression) = 0;
    virtual Pointer<Continuation> TranslateFile(
        const char *name, Structure st = Structure::Program) = 0;
};

KAI_END
