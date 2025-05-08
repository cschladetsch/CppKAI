#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Rho/RhoParser.h>

#include <iostream>

KAI_BEGIN

bool RhoParser::Process(std::shared_ptr<Lexer> lex, Structure st) {
    lexer = lex;
    if (lex->Failed) {
        KAI_TRACE_ERROR() << "Lexer failed: " << lex->Error;
        return Fail(lex->Error);
    }

    KAI_TRACE() << "Starting to process tokens";
    for (auto tok : lexer->GetTokens()) {
        if (tok.type != TokenEnum::Whitespace &&
            tok.type != TokenEnum::Comment) {
            tokens.push_back(tok);
            KAI_TRACE() << "Token";
        }
    }

    root = NewNode(AstEnum::Program);
    KAI_TRACE() << "Created root Program node";

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
    std::cout << "RhoParser::Program - Starting to parse program" << std::endl;

    // Continue parsing until we reach the end or encounter an error
    while (!Try(TokenType::None) && !Failed) {
        // Skip any newlines or semicolons between statements
        while (Try(TokenType::NewLine) || Try(TokenType::Semi)) {
            std::cout << "RhoParser::Program - Skipping newline or semicolon"
                      << std::endl;
            Consume();
        }

        // If we've reached the end, we're done
        if (Try(TokenType::None)) {
            break;
        }

        // Parse the next statement
        std::cout << "RhoParser::Program - Parsing statement: "
                  << TokenEnumType::ToString(Current().type) << std::endl;
        if (!Statement(root)) {
            return Fail("Statement expected");
        }

        std::cout << "RhoParser::Program - Successfully parsed statement"
                  << std::endl;
    }

    std::cout << "RhoParser::Program - Finished parsing program" << std::endl;
    return true;
}

bool RhoParser::Function(AstNodePtr node) {
    ConsumeNewLines();

    // We need to handle both formats:
    // 1. fun name(args)
    //    block
    // 2. fun(args) { block }

    Expect(TokenType::Fun);

    // Check if this is "fun name(args)" or "fun(args)"
    bool hasName = Try(TokenType::Ident);
    RhoToken name;

    if (hasName) {
        name = Consume();
    }

    std::shared_ptr<AstNode> fun = NewNode(AstEnum::Function);

    if (hasName) {
        fun->Add(name);
    } else {
        // If no name is provided, add a temporary placeholder
        // Create a slice for the anonymous identifier
        Slice anonymousSlice;
        fun->Add(RhoToken(TokenEnum::Ident, *lexer.get(), 0, anonymousSlice));
    }

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

    // Handle optional open brace after function declaration
    bool hasOpenBrace = false;
    if (Try(TokenType::OpenBrace)) {
        Consume();
        hasOpenBrace = true;
    } else {
        Expect(TokenType::NewLine);
    }

    auto block = NewNode(RhoAstNodeEnumType::Block);

    // Parse the body
    if (hasOpenBrace) {
        // Parse statements in the function body until we hit a closing brace
        while (!Try(TokenType::CloseBrace) && !Try(TokenType::None) &&
               !Failed) {
            if (!Statement(block))
                return CreateError("Statement expected in function body");

            // After each statement, consume any semicolons
            while (Try(TokenType::Semi)) {
                Consume();
                if (!Try(TokenType::CloseBrace) && !Try(TokenType::None) &&
                    !Try(TokenType::NewLine)) {
                    // If there's another statement after the semicolon, process
                    // it
                    if (!Statement(block))
                        return CreateError(
                            "Statement expected after semicolon");
                }
            }
        }

        if (Try(TokenType::CloseBrace)) {
            Consume();
        } else {
            return CreateError("Expected closing brace for function body");
        }
    } else {
        // Use the old Block method for indented code blocks
        if (!Block(block)) {
            return CreateError("Block Expected for function body");
        }
    }

    fun->Add(block);

    // Handle optional semicolon after the entire function declaration
    if (Try(TokenType::Semi)) Consume();

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

        if (!Statement(node)) {
            return false;
        }

        // Continue parsing more statements in this block
        continue;
    }

    return false;
}

bool RhoParser::Statement(AstNodePtr block) {
    std::cout << "RhoParser::Statement - Current token: "
              << TokenEnumType::ToString(Current().type) << std::endl;

    // Process statement terminators first
    if (Try(TokenType::NewLine) || Try(TokenType::Semi)) {
        std::cout << "RhoParser::Statement - Consuming statement terminator"
                  << std::endl;
        Consume();
        return true;
    }

    // Handle each statement type
    switch (Current().type) {
        case TokenType::Assert: {
            std::cout << "RhoParser::Statement - Processing Assert"
                      << std::endl;
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
            std::cout << "RhoParser::Statement - Processing Return/Yield"
                      << std::endl;
            auto ret = NewNode(Consume());
            if (Expression()) ret->Add(Pop());
            block->Add(ret);
            goto finis;
        }

        case TokenType::If: {
            std::cout << "RhoParser::Statement - Processing If" << std::endl;
            return IfCondition(block);
        }

        case TokenType::While: {
            std::cout << "RhoParser::Statement - Processing While" << std::endl;
            if (!WhileLoop(block)) {
                return false;
            }
            // after handling a while loop, there may be an optional semicolon
            if (Try(TokenType::Semi)) Consume();
            return true;
        }

        case TokenType::DoWhile: {
            std::cout << "RhoParser::Statement - Processing DoWhile"
                      << std::endl;
            if (!DoWhileLoop(block)) {
                return false;
            }
            // after handling a do-while loop, there may be an optional
            // semicolon
            if (Try(TokenType::Semi)) Consume();
            return true;
        }

        case TokenType::Fun: {
            std::cout << "RhoParser::Statement - Processing Function"
                      << std::endl;
            return Function(block);
        }

        case TokenType::None:
            // End of input is okay
            return true;

        default:
            // Default case: assume it's an expression
            break;
    }

    std::cout << "RhoParser::Statement - Processing Expression" << std::endl;
    if (!Expression()) return false;

    block->Add(Pop());

finis:
    // statements can end with an optional semi followed by a new line
    if (Try(TokenType::Semi)) {
        std::cout << "RhoParser::Statement - Consuming trailing semicolon"
                  << std::endl;
        Consume();
    }

    // Accept either a semicolon, a newline, or none at the end of file
    if (!Try(TokenType::None) && !Try(TokenType::CloseBrace)) {
        if (Try(TokenType::NewLine)) {
            std::cout << "RhoParser::Statement - Consuming newline"
                      << std::endl;
            Consume();
        } else if (!Try(TokenType::Semi) && !Try(TokenType::While) &&
                   !Try(TokenType::For) && !Try(TokenType::If) &&
                   !Try(TokenType::Else) && !Try(TokenType::Fun) &&
                   !Try(TokenType::DoWhile)) {
            // Only expect a newline if the next token isn't another statement
            // starter
            std::cout << "RhoParser::Statement - Expecting NewLine"
                      << std::endl;
            Expect(TokenType::NewLine);
        }
    }

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

    // Handle optional parentheses around condition
    bool hasParens = false;
    if (Try(TokenType::OpenParan)) {
        Consume();
        hasParens = true;
    }

    if (!Expression()) {
        return CreateError("If what?");
    }

    auto condition = Pop();

    // Consume closing parenthesis if we had an opening one
    if (hasParens) {
        if (!Try(TokenType::CloseParan)) {
            return CreateError("Expected closing parenthesis for if condition");
        }
        Consume();
    }

    // Handle optional open brace
    bool hasOpenBrace = false;
    if (Try(TokenType::OpenBrace)) {
        Consume();
        hasOpenBrace = true;
    }

    auto trueClause = NewNode(NodeType::Block);

    if (!Block(trueClause)) {
        return CreateError("Block Expected for if body");
    }

    // If we opened with a brace, expect a closing brace
    if (hasOpenBrace) {
        if (!Try(TokenType::CloseBrace)) {
            return CreateError("Expected closing brace for if body");
        }
        Consume();
    }

    auto cond = NewNode(NodeType::Conditional);
    cond->Add(condition);
    cond->Add(trueClause);

    if (Try(TokenType::Else)) {
        Consume();

        // Handle optional open brace for else block
        bool hasElseOpenBrace = false;
        if (Try(TokenType::OpenBrace)) {
            Consume();
            hasElseOpenBrace = true;
        }

        auto falseClause = NewNode(NodeType::Block);
        Block(falseClause);

        // If else had an open brace, expect a closing brace
        if (hasElseOpenBrace) {
            if (!Try(TokenType::CloseBrace)) {
                return CreateError("Expected closing brace for else body");
            }
            Consume();
        }

        cond->Add(falseClause);
    }

    // After handling an if condition, there may be an optional semicolon
    if (Try(TokenType::Semi)) Consume();

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

bool RhoParser::CreateError(const char *text) {
    return Fail(Lexer::CreateErrorMessage(Current(), text));
}

bool RhoParser::WhileLoop(AstNodePtr block) {
    if (!Try(TokenType::While)) return false;

    std::cout << "RhoParser::WhileLoop - Found 'while' token" << std::endl;
    Consume();

    // Handle optional parentheses around condition
    bool hasParens = false;
    if (Try(TokenType::OpenParan)) {
        std::cout << "RhoParser::WhileLoop - Found open parenthesis"
                  << std::endl;
        Consume();
        hasParens = true;
    } else {
        std::cout << "RhoParser::WhileLoop - No opening parenthesis found, "
                     "current token: "
                  << TokenEnumType::ToString(Current().type) << std::endl;
    }

    std::cout << "RhoParser::WhileLoop - Parsing condition expression"
              << std::endl;
    if (!Expression()) {
        return CreateError("While what? Expected condition expression");
    }

    auto condition = Pop();
    std::cout << "RhoParser::WhileLoop - Parsed condition expression"
              << std::endl;

    // Consume closing parenthesis if we had an opening one
    if (hasParens) {
        if (!Try(TokenType::CloseParan)) {
            std::cout << "RhoParser::WhileLoop - Missing closing parenthesis, "
                         "current token: "
                      << TokenEnumType::ToString(Current().type) << std::endl;
            return CreateError(
                "Expected closing parenthesis for while condition");
        }
        std::cout << "RhoParser::WhileLoop - Found close parenthesis"
                  << std::endl;
        Consume();
    }

    // Handle optional open brace
    bool hasOpenBrace = false;
    if (Try(TokenType::OpenBrace)) {
        std::cout << "RhoParser::WhileLoop - Found open brace" << std::endl;
        Consume();
        hasOpenBrace = true;
    } else {
        std::cout
            << "RhoParser::WhileLoop - No open brace found, current token: "
            << TokenEnumType::ToString(Current().type) << std::endl;
    }

    auto bodyClause = NewNode(NodeType::Block);
    std::cout << "RhoParser::WhileLoop - Created body block node" << std::endl;

    // Parse the body block
    std::cout << "RhoParser::WhileLoop - Parsing body block" << std::endl;
    if (!Block(bodyClause)) {
        return CreateError("Block Expected for While loop body");
    }
    std::cout << "RhoParser::WhileLoop - Parsed body block" << std::endl;

    // If we opened with a brace, expect a closing brace
    if (hasOpenBrace) {
        if (!Try(TokenType::CloseBrace)) {
            std::cout << "RhoParser::WhileLoop - Missing closing brace, "
                         "current token: "
                      << TokenEnumType::ToString(Current().type) << std::endl;
            return CreateError("Expected closing brace for while loop body");
        }
        std::cout << "RhoParser::WhileLoop - Found close brace" << std::endl;
        Consume();
    }

    auto whileNode = NewNode(NodeType::While);
    whileNode->Add(condition);
    whileNode->Add(bodyClause);
    std::cout
        << "RhoParser::WhileLoop - Created while node with condition and body"
        << std::endl;

    if (!block->Add(whileNode)) {
        std::cout << "RhoParser::WhileLoop - Failed to add while node to block"
                  << std::endl;
        return false;
    }

    std::cout << "RhoParser::WhileLoop - Successfully added while node to block"
              << std::endl;
    return true;
}

bool RhoParser::DoWhileLoop(AstNodePtr block) {
    // Safety check - ensure we have tokens to process
    if (tokens.empty() || current >= tokens.size()) {
        std::cout
            << "RhoParser::DoWhileLoop - No tokens to process: tokens.size="
            << tokens.size() << ", current=" << current << std::endl;
        return CreateError("No tokens to process in DoWhileLoop");
    }

    if (!Try(TokenType::DoWhile)) return false;

    std::cout << "RhoParser::DoWhileLoop - Found 'do' token" << std::endl;
    Consume();

    // Handle optional open brace
    bool hasOpenBrace = false;
    if (Try(TokenType::OpenBrace)) {
        std::cout << "RhoParser::DoWhileLoop - Found open brace" << std::endl;
        Consume();
        hasOpenBrace = true;
    } else {
        if (current < tokens.size()) {
            std::cout
                << "RhoParser::DoWhileLoop - No open brace, current token: "
                << TokenEnumType::ToString(Current().type) << std::endl;
        } else {
            std::cout << "RhoParser::DoWhileLoop - No open brace, current "
                         "token index out of range"
                      << std::endl;
        }
    }

    auto bodyClause = NewNode(NodeType::Block);

    // Parse the body block
    if (hasOpenBrace) {
        // Parse statements in the do body until we hit a closing brace
        std::cout
            << "RhoParser::DoWhileLoop - Parsing statements until closing brace"
            << std::endl;
        while (current < tokens.size() && !Try(TokenType::CloseBrace) &&
               !Try(TokenType::None) && !Failed) {
            if (!Statement(bodyClause)) {
                return CreateError("Statement expected in do-while body");
            }

            // After each statement, consume any semicolons
            while (current < tokens.size() && Try(TokenType::Semi)) {
                Consume();
                if (current < tokens.size() && !Try(TokenType::CloseBrace) &&
                    !Try(TokenType::None) && !Try(TokenType::NewLine)) {
                    // If there's another statement after the semicolon, process
                    // it
                    if (!Statement(bodyClause)) {
                        return CreateError(
                            "Statement expected after semicolon in do-while "
                            "body");
                    }
                }
            }
        }

        if (current < tokens.size() && Try(TokenType::CloseBrace)) {
            std::cout << "RhoParser::DoWhileLoop - Found closing brace"
                      << std::endl;
            Consume();
        } else {
            std::cout
                << "RhoParser::DoWhileLoop - Missing closing brace at position "
                << current << std::endl;
            return CreateError("Expected closing brace for do-while body");
        }
    } else {
        // Use the Block method for indented code blocks
        std::cout << "RhoParser::DoWhileLoop - Using Block for body"
                  << std::endl;
        if (!Block(bodyClause)) {
            return CreateError("Block Expected for do-while body");
        }
    }

    // After the body, expect 'while' keyword
    if (current >= tokens.size() || !Try(TokenType::While)) {
        if (current < tokens.size()) {
            std::cout
                << "RhoParser::DoWhileLoop - Expected 'while', current token: "
                << TokenEnumType::ToString(Current().type) << std::endl;
        } else {
            std::cout << "RhoParser::DoWhileLoop - Expected 'while', but "
                         "reached end of tokens"
                      << std::endl;
        }
        return CreateError("Expected 'while' after do-while body");
    }

    std::cout << "RhoParser::DoWhileLoop - Found 'while' token after do block"
              << std::endl;
    Consume();

    // Handle optional parentheses around condition
    bool hasParens = false;
    if (current < tokens.size() && Try(TokenType::OpenParan)) {
        std::cout << "RhoParser::DoWhileLoop - Found opening parenthesis for "
                     "condition"
                  << std::endl;
        Consume();
        hasParens = true;
    }

    std::cout << "RhoParser::DoWhileLoop - Parsing condition expression"
              << std::endl;
    if (!Expression()) {
        return CreateError("Do-while what? Expected condition expression");
    }

    auto condition = Pop();

    // Consume closing parenthesis if we had an opening one
    if (hasParens) {
        if (current >= tokens.size() || !Try(TokenType::CloseParan)) {
            if (current < tokens.size()) {
                std::cout << "RhoParser::DoWhileLoop - Missing closing "
                             "parenthesis, current token: "
                          << TokenEnumType::ToString(Current().type)
                          << std::endl;
            } else {
                std::cout << "RhoParser::DoWhileLoop - Missing closing "
                             "parenthesis, reached end of tokens"
                          << std::endl;
            }
            return CreateError(
                "Expected closing parenthesis for do-while condition");
        }
        std::cout << "RhoParser::DoWhileLoop - Found closing parenthesis for "
                     "condition"
                  << std::endl;
        Consume();
    }

    std::cout << "RhoParser::DoWhileLoop - Creating DoWhile node" << std::endl;
    auto doWhileNode = NewNode(NodeType::DoWhile);
    doWhileNode->Add(bodyClause);
    doWhileNode->Add(
        condition);  // Note: condition is second for do-while (after body)

    return block->Add(doWhileNode);
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
