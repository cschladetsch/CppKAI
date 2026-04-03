#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Hlsl/HlslParser.h>

#include <iostream>
#include <vector>

KAI_BEGIN

using namespace std;

bool HlslParser::Process(std::shared_ptr<Lexer> lex, Structure st) {
    lexer = lex;
    if (lex->Failed) {
        return Fail(lex->Error);
    }

    // strip whitespace and comments
    for (auto tok : lexer->GetTokens()) {
        if (tok.type != TokenEnum::Whitespace &&
            tok.type != TokenEnum::Comment) {
            tokens.push_back(tok);
            std::cout << tok.ToString() << ' ';
        }
    }

    root = NewNode(AstEnum::Program);

    return Run(st);
}

bool HlslParser::Run(Structure st) {
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

bool HlslParser::Program() {
    while (!Try(TokenType::None) && !Failed) {
        ConsumeNewLines();
        if (!Statement(root)) {
            return Fail("Statement expected");
        }
    }

    return true;
}

void HlslParser::Function(AstNodePtr node) {
    ConsumeNewLines();

    // Expect(TokenType::Fun);
    Expect(TokenType::Ident);
    HlslToken name = Last();
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

    if (Try(TokenType::Colon)) {
        Fail("Not implemented");
        KAI_NOT_IMPLEMENTED();
    }

    Expect(TokenType::CloseParan);
    Expect(TokenType::NewLine);

    AddBlock(fun);
    node->Add(fun);
}

void HlslParser::Block(AstNodePtr node) {
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

bool HlslParser::Statement(AstNodePtr block) {
    switch (Current().type) {
            // case TokenType::Assert:
            //{
            //     auto ass = NewNode(Consume());
            //     if (!Expression())
            //     {
            //         Fail(Lexer::CreateErrorMessage(Current(), "Assert needs
            //         an expression to test")); return false;
            //     }

            //    ass->Add(Pop());
            //    block->Add(ass);
            //    goto finis;
            //}

        case TokenType::Return:
            // case TokenType::Yield:
            {
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

            // case TokenType::Fun:
            //{
            //     Function(block);
            //     return true;
            // }
    }

    ConsumeNewLines();

    if (Try(TokenType::None)) return true;

    if (Declaration(block)) return true;

    if (!Expression()) return false;

    block->Add(Pop());

finis:
    // statements can end with an optional semi followed by a new line
    if (Try(TokenType::Semi)) Consume();

    if (!Try(TokenType::None)) Expect(TokenType::NewLine);

    return true;
}

bool HlslParser::Expression() {
    //    bool paran = Try(TokenType::OpenParan);
    //    if (paran)
    //    {
    //        auto exp = NewNode(Consume());
    //        if (!Logical())
    //            return false;
    //
    //        Expect(TokenType::CloseParan);
    //        exp->Add(Pop());
    //        Push(exp);
    //
    ////        while (Factor())
    ////            ;
    ////
    ////        return true;
    //    }

    if (!Logical()) return false;

    if (Try(TokenType::Assign) || Try(TokenType::PlusAssign) ||
        Try(TokenType::MinusAssign) || Try(TokenType::MulAssign) ||
        Try(TokenType::DivAssign)) {
        auto node = NewNode(Consume());
        const auto ident = Pop();
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

// https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/dx-graphics-hlsl-variables
//  [Storage_Class] [Type_Modifier] Type Name[Index] [: Semantic] [: Packoffset]
//  [: Register]; [Annotations] [= Initial_Value]
bool HlslParser::Declaration(AstNodePtr block) {
    static vector<TokenType::Enum> storageClass = {
        TokenType::Extern,  TokenType::Nointerpolation, TokenType::Precise,
        TokenType::Shared,  TokenType::GroupShared,     TokenType::Static,
        TokenType::Uniform, TokenType::Volatile};
    static vector<TokenType::Enum> typeMod = {
        TokenType::Const, TokenType::RowMajor, TokenType::ColumnMajor};
    static vector<TokenType::Enum> scalarType = {
        TokenType::Bool,     TokenType::Int,        TokenType::Uint,
        TokenType::Dword,    TokenType::Half,       TokenType::Float,
        TokenType::Double,   TokenType::Min16Float, TokenType::Min10Float,
        TokenType::Min16Int, TokenType::Min12Int,   TokenType::Min16Uint};
    static TokenType::Enum nonScalarType[] = {TokenType::Vector,
                                              TokenType::Matrix};

    auto current = Current();
    cout << current.ToString() << endl;
    std::vector<TokenNode> decl;
    if (Try(storageClass)) decl.push_back(Consume());
    if (Try(typeMod)) decl.push_back(Consume());
    if (Try(scalarType)) decl.push_back(Consume());
    if (decl.empty()) return false;

    const auto id = Expect(TokenType::Ident);
    auto node = NewNode(AstEnum::Declaration);
    for (auto d : decl) node->Add(d);
    node->Add(id);

    if (Try(TokenType::Assign)) {
        Next();
        if (!Initialiser()) {
            Fail(Lexer::CreateErrorMessage(
                Current(), "Declaration requires an expression"));
            return false;
        }
        node->Add(Pop());
    }
    block->Add(node);

    Expect(TokenType::Semi);

    return true;
}

bool HlslParser::Initialiser() {
    if (Try(TokenType::OpenBrace)) {
        if (!ExpressionList()) return false;
        Expect(TokenType::CloseBrace);
    }

    return Expression();
}

bool HlslParser::ExpressionList() {
    while (Expression()) {
        if (!Try(TokenType::Comma)) return true;
    }
    return false;
}

bool HlslParser::Logical() {
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

bool HlslParser::Relational() {
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

bool HlslParser::Additive() {
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

bool HlslParser::Term() {
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

bool HlslParser::Factor() {
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

    if (Try(TokenType::Ident)) return ParseFactorIdent();

    return false;
}

bool HlslParser::ParseFactorIdent() {
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

void HlslParser::ParseMethodCall() {
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
}

void HlslParser::ParseGetMember() {
    Consume();
    auto get = NewNode(NodeType::GetMember);
    get->Add(Pop());
    get->Add(Expect(TokenType::Ident));
    Push(get);
}

void HlslParser::IfCondition(AstNodePtr block) {
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

void HlslParser::ParseIndexOp() {
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

void HlslParser::For(AstNodePtr block) {
    if (!Try(TokenType::For)) return;

    Consume();

    auto f = NewNode(HlslAstNodeEnumType::For);
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

void HlslParser::While(AstNodePtr block) {
    auto w = NewNode(Consume());
    if (!Expression()) {
        CreateError("While what?");
        return;
    }

    w->Add(Pop());
    block->Add(w);
}

bool HlslParser::CreateError(const char *text) {
    return Fail(Lexer::CreateErrorMessage(Current(), text));
}

void HlslParser::AddBlock(AstNodePtr fun) {
    auto block = NewNode(HlslAstNodeEnumType::Block);
    Block(block);
    fun->Add(block);
}

void HlslParser::ConsumeNewLines() {
    while (Try(TokenType::NewLine)) Consume();
}

KAI_END
