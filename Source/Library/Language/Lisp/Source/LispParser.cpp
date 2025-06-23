#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Rho/RhoParser.h>

KAI_BEGIN

bool RhoParser::Process(std::shared_ptr<Lexer> lex, Structure st) {
    lexer = lex;
    if (lex->Failed) {
        return Fail(lex->Error);
    }

    // strip whitespace and comments
    for (auto tok : lexer->GetTokens()) {
        if (tok.type != TokenEnum::Whitespace &&
            tok.type != TokenEnum::Comment) {
            tokens.push_back(tok);
        }
    }

    root = NewNode(AstEnum::Program);

    return Run(st);
}

bool RhoParser::Run(Structure st) {
    switch (st) {
        case Structure::Statement:
            if (!Statement(root)) {
                return CreateError("Statement expected");
            }
            break;

        case Structure::Expression:
            if (!Expression()) {
                return CreateError("Expression expected");
            }
            root->Add(Pop());
            break;

        case Structure::Function:
            Function(root);
            break;

        case Structure::Program:
            Program();
            break;
    }

    ConsumeNewLines();
    if (!stack.empty())
        return Fail("[Internal] Error: Stack not empty after parsing");

    return true;
}

bool RhoParser::Program() {
    while (!Try(TokenType::None) && !Failed) {
        ConsumeNewLines();
        if (!Statement(root)) {
            Fail("Statement expected");
            return false;
        }
    }

    return true;
}

void RhoParser::Function(AstNodePtr node) {
    ConsumeNewLines();

    Expect(TokenType::Fun);
    Expect(TokenType::Ident);
    RhoToken name = Last();
    std::shared_ptr<AstNode> fun = NewNode(AstEnum::Function);
    fun->Add(name);
    Expect(TokenType::OpenParan);
    std::shared_ptr<AstNode> args = NewNode(AstEnum::None);
    fun->Add(args);

    if (Try(TokenType::Ident)) {
        args->Add(Consume());
        while (Try(TokenType::Comma)) {
            Consume();
            args->Add(Expect(TokenType::Ident));
        }
    }

    Expect(TokenType::CloseParan);
    Expect(TokenType::NewLine);

    AddBlock(fun);
    node->Add(fun);
}

void RhoParser::Block(AstNodePtr node) {
    ConsumeNewLines();

    ++indent;
    while (!Failed) {
        int level = 0;
        while (Try(TokenType::Tab)) {
            ++level;
            Consume();
        }

        if (Try(TokenType::NewLine)) {
            Consume();
            continue;
        }

        // close current block
        if (level < indent) {
            --indent;

            // rewind to start of tab sequence to determine next block
            --current;
            while (Try(TokenType::Tab)) --current;

            ++current;
            return;
        }

        if (level != indent) {
            Fail(Lexer::CreateErrorMessage(Current(), "Mismatch block indent"));
            return;
        }

        Statement(node);
    }
}

bool RhoParser::Statement(AstNodePtr block) {
    switch (Current().type) {
        case TokenType::Assert: {
            auto ass = NewNode(Consume());
            if (!Expression()) {
                Fail(Lexer::CreateErrorMessage(
                    Current(), "Assert needs an expression to test"));
                return false;
            }

            ass->Add(Pop());
            block->Add(ass);
            goto finis;
        }

        case TokenType::Return:
        case TokenType::Yield: {
            auto ret = NewNode(Consume());
            if (Expression()) ret->Add(Pop());
            block->Add(ret);
            goto finis;
        }

        case TokenType::While: {
            While(block);
            return true;
        }

        case TokenType::For: {
            For(block);
            return true;
        }

        case TokenType::If: {
            IfCondition(block);
            return true;
        }

        case TokenType::Fun: {
            Function(block);
            return true;
        }
    }

    ConsumeNewLines();

    if (Try(TokenType::None)) return true;

    if (!Expression()) return false;

    block->Add(Pop());

finis:
    // statements can end with an optional semi followed by a new line
    if (Try(TokenType::Semi)) Consume();

    if (!Try(TokenType::None)) Expect(TokenType::NewLine);

    return true;
}

bool RhoParser::Expression() {
    if (!Logical()) return false;

    if (Try(TokenType::Assign) || Try(TokenType::PlusAssign) ||
        Try(TokenType::MinusAssign) || Try(TokenType::MulAssign) ||
        Try(TokenType::DivAssign)) {
        auto node = NewNode(Consume());
        auto ident = Pop();
        if (!Logical()) {
            Fail(Lexer::CreateErrorMessage(
                Current(), "Assignment requires an expression"));
            return false;
        }

        node->Add(Pop());
        node->Add(ident);
        Push(node);
    }

    return true;
}

bool RhoParser::Logical() {
    if (!Relational()) return false;

    while (Try(TokenType::And) || Try(TokenType::Or)) {
        auto node = NewNode(Consume());
        node->Add(Pop());
        if (!Relational()) return CreateError("Relational expected");

        node->Add(Pop());
        Push(node);
    }

    return true;
}

bool RhoParser::Relational() {
    if (!Additive()) return false;

    while (Try(TokenType::Less) || Try(TokenType::Greater) ||
           Try(TokenType::Equiv) || Try(TokenType::NotEquiv) ||
           Try(TokenType::LessEquiv) || Try(TokenType::GreaterEquiv)) {
        auto node = NewNode(Consume());
        node->Add(Pop());
        if (!Additive()) return CreateError("Additive expected");

        node->Add(Pop());
        Push(node);
    }

    return true;
}

bool RhoParser::Additive() {
    // unary +/- operator
    if (Try(TokenType::Plus) || Try(TokenType::Minus)) {
        auto uniSigned = NewNode(Consume());
        if (!Term()) return CreateError("Term expected");

        uniSigned->Add(Pop());
        Push(uniSigned);
        return true;
    }

    if (Try(TokenType::Not)) {
        auto negate = NewNode(Consume());
        if (!Additive()) return CreateError("Additive expected");

        negate->Add(Pop());
        Push(negate);
        return true;
    }

    if (!Term()) return false;

    while (Try(TokenType::Plus) || Try(TokenType::Minus)) {
        //[1]
        auto node = NewNode(Consume());
        node->Add(Pop());
        if (!Term()) return CreateError("Term expected");

        node->Add(Pop());
        Push(node);
    }

    return true;
}

bool RhoParser::Term() {
    if (!Factor()) return false;

    while (Try(TokenType::Mul) || Try(TokenType::Divide)) {
        auto node = NewNode(Consume());
        node->Add(Pop());
        if (!Factor()) return CreateError("Factor expected with a term");

        node->Add(Pop());
        Push(node);
    }

    return true;
}

bool RhoParser::Factor() {
    if (Try(TokenType::OpenParan)) {
        auto exp = NewNode(Consume());
        if (!Expression())
            return CreateError(
                "Expected an expression for a factor in parenthesis");

        Expect(TokenType::CloseParan);
        exp->Add(Pop());
        Push(exp);
        return true;
    }

    if (Try(TokenType::OpenSquareBracket)) {
        auto list = NewNode(NodeType::List);
        do {
            Consume();
            if (Try(TokenType::CloseSquareBracket)) break;
            if (Expression())
                list->Add(Pop());
            else {
                Fail("Badly formed array");
                return false;
            }
        } while (Try(TokenType::Comma));

        Expect(TokenType::CloseSquareBracket);
        Push(list);

        return true;
    }

    if (Try(TokenType::Int) || Try(TokenType::Float) ||
        Try(TokenType::String) || Try(TokenType::True) || Try(TokenType::False))
        return PushConsume();

    if (Try(TokenType::Self)) return PushConsume();

    //    while (Try(TokenType::Lookup))
    //        return PushConsume();

    if (Try(TokenType::Ident)) return ParseFactorIdent();

    if (Try(TokenType::Pathname)) return ParseFactorIdent();

    return false;
}

bool RhoParser::ParseFactorIdent() {
    PushConsume();

    while (!Failed) {
        if (Try(TokenType::Dot)) {
            ParseGetMember();
            continue;
        }

        if (Try(TokenType::OpenParan)) {
            ParseMethodCall();
            continue;
        }

        if (Try(TokenType::OpenSquareBracket)) {
            ParseIndexOp();
            continue;
        }

        break;
    }

    return true;
}

void RhoParser::ParseMethodCall() {
    Consume();
    auto call = NewNode(NodeType::Call);
    call->Add(Pop());
    auto args = NewNode(NodeType::ArgList);
    call->Add(args);

    if (Expression()) {
        args->Add(Pop());
        while (Try(TokenType::Comma)) {
            Consume();
            if (!Expression()) {
                CreateError("What is the next argument?");
                return;
            }

            args->Add(Pop());
        }
    }

    Push(call);
    Expect(TokenType::CloseParan);

    if (Try(TokenType::Replace)) call->Add(Consume());
}

void RhoParser::ParseGetMember() {
    Consume();
    auto get = NewNode(NodeType::GetMember);
    get->Add(Pop());
    get->Add(Expect(TokenType::Ident));
    Push(get);
}

void RhoParser::IfCondition(AstNodePtr block) {
    if (!Try(TokenType::If)) return;

    Consume();

    if (!Expression()) {
        CreateError("If what?");
        return;
    }

    auto condition = Pop();

    // get the true-clause
    auto trueClause = NewNode(NodeType::Block);
    Block(trueClause);

    // make the conditional node in AST
    auto cond = NewNode(NodeType::Conditional);
    cond->Add(condition);
    cond->Add(trueClause);

    // if there's an else, add it as well
    if (Try(TokenType::Else)) {
        Consume();
        auto falseClause = NewNode(NodeType::Block);
        Block(falseClause);
        cond->Add(falseClause);
    }

    block->Add(cond);
}

void RhoParser::ParseIndexOp() {
    Consume();
    auto index = NewNode(NodeType::IndexOp);
    index->Add(Pop());
    if (!Expression()) {
        CreateError("Index what?");
        return;
    }

    Expect(TokenType::CloseSquareBracket);
    index->Add(Pop());
    Push(index);
}

void RhoParser::For(AstNodePtr block) {
    if (!Try(TokenType::For)) return;

    Consume();

    auto f = NewNode(RhoAstNodeEnumType::For);
    if (!Expression()) {
        CreateError("For what?");
        return;
    }

    if (Try(TokenType::In)) {
        Consume();
        f->Add(Pop());

        if (!Expression()) {
            CreateError("For each in what?");
            return;
        }

        f->Add(Pop());
    } else {
        Expect(TokenType::Semi);
        f->Add(Pop());

        if (!Expression()) {
            CreateError("When does the for statement stop?");
            return;
        }

        f->Add(Pop());
        Expect(TokenType::Semi);

        if (!Expression()) {
            CreateError("What happens when a for statement ends?");
            return;
        }

        f->Add(Pop());
    }

    Expect(TokenType::NewLine);
    AddBlock(f);
    block->Add(f);
}

void RhoParser::While(AstNodePtr block) {
    auto w = NewNode(Consume());
    if (!Expression()) {
        CreateError("While what?");
        return;
    }

    w->Add(Pop());
    block->Add(w);
}

bool RhoParser::CreateError(const char *text) {
    return Fail(Lexer::CreateErrorMessage(Current(), text));
}

void RhoParser::AddBlock(AstNodePtr fun) {
    auto block = NewNode(RhoAstNodeEnumType::Block);
    Block(block);
    fun->Add(block);
}

void RhoParser::ConsumeNewLines() {
    while (Try(TokenType::NewLine)) Consume();
}

KAI_END
