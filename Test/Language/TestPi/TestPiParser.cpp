#include <KAI/Language/Pi/Pi.h>

#include "TestCommon.h"

using namespace kai;
using namespace std;

static bool trace = false;

namespace {
TEST(PiParser, TestSimple) {
    auto input = "1";
    Registry reg;

    shared_ptr<PiLexer> lex = make_shared<PiLexer>(input, reg);
    lex->Process();
    if (trace) KAI_TRACE_1(lex->Print());

    shared_ptr<PiParser> parse = make_shared<PiParser>(reg);
    parse->Process(lex, Structure::Sequence);

    if (trace) KAI_TRACE_1(parse->PrintTree());
    ASSERT_FALSE(lex->Failed);
    ASSERT_FALSE(parse->Failed);

    auto root = parse->GetRoot();
    ASSERT_EQ(root->GetChildren().size(), 1);
    ASSERT_EQ(root->GetChild(0)->GetToken().type, PiTokens::Int);
}

TEST(PiParser, TestCompoundWorks) {
    auto input = "[[[]]]";
    Registry reg;
    shared_ptr<PiLexer> lex = make_shared<PiLexer>(input, reg);
    lex->Process();
    if (trace) KAI_TRACE_1(lex->Print());
    if (lex->Failed) KAI_TRACE_1(lex->Error);

    shared_ptr<PiParser> parser = make_shared<PiParser>(reg);
    parser->Process(lex, Structure::Sequence);

    ASSERT_FALSE(lex->Failed);
    ASSERT_FALSE(parser->Failed);

    if (trace) parser->PrintTree();

    auto root = parser->GetRoot();
    ASSERT_EQ(root->GetChildren().size(), 1);

    auto ch0 = root->GetChildren()[0];
    auto ch1 = ch0->GetChildren()[0];
    auto ch2 = ch1->GetChildren()[0];

    ASSERT_EQ(ch0->GetType(), PiAstNodes::Array);
    ASSERT_EQ(ch1->GetType(), PiAstNodes::Array);
    ASSERT_EQ(ch2->GetType(), PiAstNodes::Array);
}

TEST(PiParser, TestMixedCompound) {
    auto input = "{ [ 1 2 3 ] 'a # }";
    Registry reg;

    shared_ptr<PiLexer> lex = make_shared<PiLexer>(input, reg);
    lex->Process();
    if (trace) KAI_TRACE_1(lex->Print());
    if (lex->Failed) KAI_TRACE_1(lex->Error);

    shared_ptr<PiParser> parse = make_shared<PiParser>(reg);
    parse->Process(lex, Structure::Sequence);

    if (trace) KAI_TRACE_1(parse->PrintTree());
    ASSERT_FALSE(lex->Failed);
    ASSERT_FALSE(parse->Failed);
}

TEST(PiParser, TestCompoundFail) {
    auto input = "[";
    Registry reg;
    shared_ptr<PiLexer> lex = make_shared<PiLexer>(input, reg);
    lex->Process();
    if (trace) KAI_TRACE_1(lex->Print());
    if (lex->Failed) KAI_TRACE_1(lex->Error);

    shared_ptr<PiParser> parse = make_shared<PiParser>(reg);
    parse->Process(lex, Structure::Sequence);

    if (trace) KAI_TRACE_1(parse->PrintTree());
    ASSERT_FALSE(lex->Failed);
    ASSERT_TRUE(parse->Failed);
}

TEST(PiParser, TestCompoundFail2) {
    auto input = "[[]";
    Registry reg;

    shared_ptr<PiLexer> lex = make_shared<PiLexer>(input, reg);
    lex->Process();
    if (trace) KAI_TRACE_1(lex->Print());
    if (lex->Failed) KAI_TRACE_1(lex->Error);

    shared_ptr<PiParser> parse = make_shared<PiParser>(reg);
    parse->Process(lex, Structure::Sequence);

    // Disabled for cleaner output
    // KAI_TRACE_1(parse->PrintTree());
    ASSERT_FALSE(lex->Failed);
    ASSERT_TRUE(parse->Failed);
}

TEST(PiParser, TestCompoundFail3) {
    auto input = "]";
    Registry reg;

    shared_ptr<PiLexer> lex = make_shared<PiLexer>(input, reg);
    lex->Process();
    if (trace) KAI_TRACE_1(lex->Print());
    if (lex->Failed) KAI_TRACE_1(lex->Error);

    shared_ptr<PiParser> parse = make_shared<PiParser>(reg);
    parse->Process(lex, Structure::Sequence);

    if (trace) KAI_TRACE_1(parse->PrintTree());
    ASSERT_FALSE(lex->Failed);
    ASSERT_TRUE(parse->Failed);
}
}  // namespace
