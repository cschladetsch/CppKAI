#include "KAI/Language/Hlsl/HlslLang.h"

#include <KAI/Core/File.h>

#include <fstream>

#include "KAI/Executor/Executor.h"

using namespace std;

KAI_BEGIN

void HlslLang::Print() {
    /*
    cout << "Input--" << endl;
    cout << lex->GetInput() << endl;

    cout << "Lexer--" << endl;
    lex->Print();

    cout << "Parser--" << endl;
    parse->Print();

    cout << "Trans--" << endl;
    cout << trans->Result() << endl;
     */
}

Pointer<Continuation> HlslLang::TranslateFile(const char *name, Structure st) {
    ifstream file(name, ios::binary);
    if (!file) return Object();

    return Translate(ReadTextFile(name).c_str(), st);
}

Pointer<Continuation> HlslLang::Translate(const char *text, Structure st) {
    KAI_UNUSED_2(text, st);
    return Object();

    //    trans = std::make_shared<Translator>(reg);
    //    trans->Translate(text);
    //
    //    if (lex->Failed)
    //        Fail(lex->Error);
    //
    //    if (parse->Failed)
    //        Fail(parse->Error);
    //
    //    if (trans->Failed)
    //        Fail(trans->Error);
    //
    //    return !Failed;
}

KAI_END
