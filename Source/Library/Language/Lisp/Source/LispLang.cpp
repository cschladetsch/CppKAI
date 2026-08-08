#include <KAI/Core/File.h>

#include <fstream>
#include <iostream>

#include "KAI/Executor/Executor.h"
#include "KAI/Language/Lisp/LispLang.h"

using namespace std;

KAI_BEGIN

void LispLang::Print() const {
    cout << "Input:" << endl;
    KAI_TRACE_1(lex->GetInput());

    cout << "Lexer:" << endl;
    KAI_TRACE_1(lex->ToString());

    cout << "Parser:" << endl;
    KAI_TRACE_1(parse->ToString());

    cout << "Trans:" << endl;
    KAI_TRACE_1(trans->ToString());
}

Pointer<Continuation> LispLang::TranslateFile(const char *name, Structure st) {
    return Translate(File::ReadAllText(name).c_str(), st);
}

Pointer<Continuation> LispLang::Translate(const char *text, Structure st) {
    if (lex->Failed) return Fail(lex->Error), Object();

    if (parse->Failed) return Fail(parse->Error), Object();

    auto trans = make_shared<Translator>(reg);
    auto cont = trans->Translate(text, st);
    if (trans->Failed) return Fail(trans->Error), Object();

    return cont;
}

KAI_END

// EOF
