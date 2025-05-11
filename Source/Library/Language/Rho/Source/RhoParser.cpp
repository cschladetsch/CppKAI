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
        // Skip any newlines between statements
        while (Try(TokenType::NewLine)) {
            std::cout << "RhoParser::Program - Skipping newline"
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

    // Expect newline after function declaration
    Expect(TokenType::NewLine);

    auto block = NewNode(RhoAstNodeEnumType::Block);

    // Use the Block method for indented code blocks
    if (!Block(block)) {
        return CreateError("Block Expected for function body");
    }

    fun->Add(block);

    // No trailing semicolon in Rho

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
    if (Try(TokenType::NewLine)) {
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
            // Rho doesn't use semicolons
            return true;
        }

        case TokenType::DoWhile: {
            std::cout << "RhoParser::Statement - Processing Do-While loop" << std::endl;

            // Use the proper DoWhileLoop function to parse the do-while statement
            if (!DoWhileLoop(block)) {
                return false;
            }

            // After handling a do-while loop, there may be an optional semicolon
            // Rho doesn't use semicolons
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
    // In Rho, statements end with a newline
    // Accept a newline or none at the end of file
    if (!Try(TokenType::None)) {
        if (Try(TokenType::NewLine)) {
            std::cout << "RhoParser::Statement - Consuming newline"
                      << std::endl;
            Consume();
        } else if (!Try(TokenType::While) && !Try(TokenType::For) &&
                   !Try(TokenType::If) && !Try(TokenType::Else) &&
                   !Try(TokenType::Fun) && !Try(TokenType::DoWhile)) {
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

    // No parentheses in Rho
    if (!Expression()) {
        return CreateError("If what?");
    }

    auto condition = Pop();

    // Expect newline after condition
    if (!Try(TokenType::NewLine)) {
        return CreateError("Expected newline after if condition");
    }
    Consume();

    auto trueClause = NewNode(NodeType::Block);

    if (!Block(trueClause)) {
        return CreateError("Block Expected for if body");
    }

    auto cond = NewNode(NodeType::Conditional);
    cond->Add(condition);
    cond->Add(trueClause);

    if (Try(TokenType::Else)) {
        Consume();

            // Expect newline after else
        if (!Try(TokenType::NewLine)) {
            return CreateError("Expected newline after else");
        }
        Consume();

        auto falseClause = NewNode(NodeType::Block);
        Block(falseClause);

        cond->Add(falseClause);
    }

    // Rho doesn't use semicolons

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

    // In Rho, we don't use parentheses for conditions
    std::cout << "RhoParser::WhileLoop - Parsing condition expression"
              << std::endl;
    if (!Expression()) {
        return CreateError("While what? Expected condition expression");
    }

    auto condition = Pop();
    std::cout << "RhoParser::WhileLoop - Parsed condition expression"
              << std::endl;

    // Expect newline after condition
    if (!Try(TokenType::NewLine)) {
        return CreateError("Expected newline after while condition");
    }
    Consume();

    auto bodyClause = NewNode(NodeType::Block);
    std::cout << "RhoParser::WhileLoop - Created body block node" << std::endl;

    // Parse the body block
    std::cout << "RhoParser::WhileLoop - Parsing body block" << std::endl;
    if (!Block(bodyClause)) {
        return CreateError("Block Expected for While loop body");
    }
    std::cout << "RhoParser::WhileLoop - Parsed body block" << std::endl;

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
        KAI_TRACE_ERROR() << "No tokens to process in DoWhileLoop";
        return CreateError("No tokens to process in DoWhileLoop");
    }

    if (!Try(TokenType::DoWhile)) return false;

    KAI_TRACE() << "Found 'do' token";
    Consume();

    // Expect newline after 'do'
    if (!Try(TokenType::NewLine)) {
        return CreateError("Expected newline after 'do'");
    }
    Consume();

    // Create a body block for the do-while body
    auto bodyClause = NewNode(NodeType::Block);

    // Use the Block method for indented code blocks
    KAI_TRACE() << "Parsing body block";
    if (!Block(bodyClause)) {
        return CreateError("Block Expected for do-while body");
    }

    // After the body, expect 'while' keyword - handle whitespace gracefully
    KAI_TRACE() << "Looking for 'while' token";

    // Skip any whitespace or tabs
    ConsumeWhitespace();

    // Debug current token
    if (current < tokens.size()) {
        KAI_TRACE() << "Next token after whitespace: " << TokenEnumType::ToString(Current().type);
    }

    // Check for the 'while' token
    if (current >= tokens.size() || !Try(TokenType::While)) {
        if (current < tokens.size()) {
            KAI_TRACE_ERROR() << "Expected 'while', got: " << TokenEnumType::ToString(Current().type);
        } else {
            KAI_TRACE_ERROR() << "Expected 'while', but reached end of tokens";
        }
        return CreateError("Expected 'while' after do-while body");
    }

    KAI_TRACE() << "Found 'while' token after do block";
    Consume();

    // Skip any whitespace before the condition
    ConsumeWhitespace();

    // Handle parentheses if present (optional in syntax, but common in usage)
    bool hasParentheses = false;
    if (Try(TokenType::OpenParan)) {
        hasParentheses = true;
        Consume();
        ConsumeWhitespace();
    }

    // Parse the condition expression
    KAI_TRACE() << "Parsing condition expression";
    if (!Expression()) {
        return CreateError("Do-while what? Expected condition expression");
    }

    // Get the condition expression
    auto condition = Pop();

    if (!condition) {
        return CreateError("Failed to parse condition expression for do-while loop");
    }

    // If we had opening parenthesis, expect a closing one
    if (hasParentheses) {
        ConsumeWhitespace();
        if (!Try(TokenType::CloseParan)) {
            return CreateError("Expected closing parenthesis after do-while condition");
        }
        Consume();
    }

    KAI_TRACE() << "Successfully parsed condition";

    // Create the DoWhile node
    KAI_TRACE() << "Creating DoWhile node";
    auto doWhileNode = NewNode(NodeType::DoWhile);

    // Add body and condition to the do-while node (order matters!)
    if (!doWhileNode->Add(bodyClause)) {
        return CreateError("Failed to add body clause to do-while node");
    }

    if (!doWhileNode->Add(condition)) {
        return CreateError("Failed to add condition to do-while node");
    }

    KAI_TRACE() << "Do-while node created with " << (int)doWhileNode->GetChildren().size() << " children";

    // Add the whole do-while construct to the block
    if (!block->Add(doWhileNode)) {
        return CreateError("Failed to add do-while node to parent block");
    }

    KAI_TRACE() << "Do-while construct added to block successfully";
    return true;
}

void RhoParser::ConsumeWhitespace() {
    // Skip any whitespace, tabs, or newlines
    while (Try(TokenType::Tab) || Try(TokenType::Whitespace) || Try(TokenType::NewLine)) {
        Consume();
    }
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
