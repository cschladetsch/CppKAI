// #include "TestCommon.h"
// #include "KAI/Language/Hlsl/Hlsl.h"
//
//  USING_NAMESPACE_KAI
//
//  using namespace std;
//
//  TEST(TestHlsl, Test)
//{
//     Console console;
//     Registry &reg = console.GetRegistry();
//     const char *text =( "float res = foo(pw*a) + bar(pw*123);");
//     auto lex = make_shared<HlslLexer>(text, reg);
//     lex->Process();
//     TEST_COUT << text << endl;
//     TEST_COUT << lex->Print() << endl;
//
//     HlslParser parse(reg);
//     parse.Process(lex, Structure::Statement);
//     TEST_COUT << parse.PrintTree() << endl;
// }
