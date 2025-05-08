#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Rho/RhoParser.h>

KAI_BEGIN

bool RhoParser::Process(std::shared_ptr<Lexer> lex, Structure st) {
    lexer = lex;
    if (lex->Failed) return Fail(lex->Error);

    for (auto tok : lexer->GetTokens()) {
        if (tok.type != TokenEnum::Whitespace && tok.type != TokenEnum::Comment)
            tokens.push_back(tok);
    }

    root = NewNode(AstEnum::Program);

    return Run(st);
}

bool RhoParser::Run(Structure st) {
    switch (st) {
        case Structure::Statement:
            if (!Statement(root)) return CreateError("Statement expected");
            break;

        case Structure::Expression:
            if (!Expression()) return CreateError("Expression expected");
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
        if (!Statement(root)) return Fail("Statement expected");
    }

    return true;
}

bool RhoParser::Function(AstNodePtr node) {
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
    return node->Add(fun);
}

bool RhoParser::Block(AstNodePtr node) {
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
            return true;
        }

        if (level != indent) {
            Fail(Lexer::CreateErrorMessage(Current(), "Mismatch block indent"));
            return false;
        }

        return Statement(node);
    }

    return false;
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

        case TokenType::ForEachNetwork: {
            ForEachNetwork(block);
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

bool RhoParser::ParseMethodCall() {
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
                return CreateError("What is the next argument?");
            }

            args->Add(Pop());
        }
    }

    Push(call);
    Expect(TokenType::CloseParan);

    if (Try(TokenType::Replace)) return call->Add(Consume());
    return true;
}

bool RhoParser::ParseGetMember() {
    Consume();
    auto get = NewNode(NodeType::GetMember);
    get->Add(Pop());
    get->Add(Expect(TokenType::Ident));
    return Push(get);
}

bool RhoParser::IfCondition(AstNodePtr block) {
    if (!Try(TokenType::If)) return false;

    Consume();

    if (!Expression()) {
        return CreateError("If what?");
    }

    auto condition = Pop();
    auto trueClause = NewNode(NodeType::Block);

    if (!Block(trueClause)) {
        return CreateError("Block Execpected");
    }

    auto cond = NewNode(NodeType::Conditional);
    cond->Add(condition);
    cond->Add(trueClause);

    if (Try(TokenType::Else)) {
        Consume();
        auto falseClause = NewNode(NodeType::Block);
        Block(falseClause);
        cond->Add(falseClause);
    }

    return block->Add(cond);
}

bool RhoParser::ParseIndexOp() {
    Consume();
    auto index = NewNode(NodeType::IndexOp);
    index->Add(Pop());
    if (!Expression()) {
        return CreateError("Index what?");
    }

    Expect(TokenType::CloseSquareBracket);
    index->Add(Pop());
    return Push(index);
}

bool RhoParser::For(AstNodePtr block) {
    if (!Try(TokenType::For)) return false;

    Consume();

    if (!Expression()) return CreateError("For what?");

    auto f = NewNode(RhoAstNodeEnumType::For);
    if (Try(TokenType::In)) {
        Consume();
        f->Add(Pop());

        if (!Expression()) return CreateError("For each in what?");

        f->Add(Pop());
    } else {
        Expect(TokenType::Semi);
        f->Add(Pop());

        if (!Expression())
            return CreateError("When does the for statement stop?");

        f->Add(Pop());
        Expect(TokenType::Semi);

        if (!Expression())
            return CreateError("What happens when a for statement ends?");

        f->Add(Pop());
    }

    Expect(TokenType::NewLine);
    AddBlock(f);
    return block->Add(f);
}

bool RhoParser::While(AstNodePtr block) {
    auto while_ = NewNode(Consume());
    if (!Expression()) return CreateError("While what?");

    while_->Add(Pop());
    return block->Add(while_);
}

bool RhoParser::ForEachNetwork(AstNodePtr block) {
    if (!Try(TokenType::ForEachNetwork)) return false;

    Consume();  // Consume forEachNetwork token

    Expect(TokenType::OpenParan);  // Expect (

    // Parse the first argument (network node)
    if (!Expression()) return CreateError("ForEachNetwork requires a network node as first argument");
    auto forEach = NewNode(RhoAstNodeEnumType::ForEachNetwork);
    forEach->Add(Pop());  // Add network node

    Expect(TokenType::Comma);  // Expect ,

    // Parse the second argument (collection)
    if (!Expression()) return CreateError("ForEachNetwork requires a collection as second argument");
    forEach->Add(Pop());  // Add collection

    Expect(TokenType::Comma);  // Expect ,

    // Parse the third argument (function to apply)
    if (!Expression()) return CreateError("ForEachNetwork requires a function as third argument");
    forEach->Add(Pop());  // Add function

    Expect(TokenType::CloseParan);  // Expect )

    return block->Add(forEach);
}

bool RhoParser::CreateError(const char *text) {
    return Fail(Lexer::CreateErrorMessage(Current(), text));
}

bool RhoParser::AddBlock(AstNodePtr fun) {
    auto block = NewNode(RhoAstNodeEnumType::Block);
    return Block(block) && fun->Add(block);
}

bool RhoParser::ConsumeNewLines() {
    while (Try(TokenType::NewLine)) Consume();
    return true;
}

KAI_END
