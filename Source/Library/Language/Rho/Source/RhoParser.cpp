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

    // Clear tokens and reset parser state
    tokens.clear();
    current = 0;  // Initialize current token index
    stack.clear();
    Failed = false;
    
    KAI_TRACE() << "Starting to process tokens";
    for (auto tok : lexer->GetTokens()) {
        if (tok.type != TokenEnum::Whitespace &&
            tok.type != TokenEnum::Comment) {
            tokens.push_back(tok);
            KAI_TRACE() << "Token: " << TokenEnumType::ToString(tok.type);
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
    KAI_TRACE() << "RhoParser::Program - Starting to parse program";

    // Continue parsing until we reach the end or encounter an error
    while (!Try(TokenType::None) && !Failed) {
        // Skip any newlines and/or semicolons between statements
        // This allows for Python-like syntax where either newlines or
        // semicolons can be used as statement separators, and both are optional
        while (Try(TokenType::NewLine) || Try(TokenType::Semi)) {
            if (Try(TokenType::NewLine)) {
                KAI_TRACE() << "RhoParser::Program - Skipping newline";
            } else {
                KAI_TRACE() << "RhoParser::Program - Skipping semicolon";
            }
            Consume();
        }

        // If we've reached the end, we're done
        if (Try(TokenType::None)) {
            break;
        }

        // Parse the next statement
        KAI_TRACE() << "RhoParser::Program - Parsing statement: "
                    << TokenEnumType::ToString(Current().type);
        if (!Statement(root)) {
            return Fail("Statement expected");
        }

        KAI_TRACE() << "RhoParser::Program - Successfully parsed statement";

        // Note: We don't need special handling for semicolons here anymore
        // as they are properly consumed in the while loop at the top
        // and Statement() handles its own terminations
    }

    KAI_TRACE() << "RhoParser::Program - Finished parsing program";
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
    bool hasName = Try(TokenType::Label);
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
        fun->Add(RhoToken(TokenEnum::Label, *lexer.get(), 0, anonymousSlice));
    }

    Expect(TokenType::OpenParan);
    std::shared_ptr<AstNode> args = NewNode(AstEnum::None);
    fun->Add(args);

    if (Try(TokenType::Label)) {
        args->Add(Consume());
        while (Try(TokenType::Comma)) {
            Consume();
            args->Add(Expect(TokenType::Label));
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
        
        // Count indentation level - handle both tabs and spaces
        // Each tab counts as 1 level, every 4 spaces count as 1 level
        int spaceCount = 0;
        while (Try(TokenType::Tab) || Try(TokenType::Whitespace)) {
            if (Try(TokenType::Tab)) {
                ++level;
                Consume();
            } else if (Try(TokenType::Whitespace)) {
                // Count spaces - assuming whitespace token contains spaces
                auto token = Current();
                spaceCount += token.Text().size();
                Consume();
                // Convert 4 spaces to 1 indent level
                while (spaceCount >= 4) {
                    ++level;
                    spaceCount -= 4;
                }
            }
        }

        if (Try(TokenType::NewLine)) {
            Consume();
            continue;
        }

        // close current block
        if (level < indent) {
            --indent;

            // rewind to start of indentation sequence to determine next block
            --current;
            while (Try(TokenType::Tab) || Try(TokenType::Whitespace)) --current;

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
    KAI_TRACE() << "RhoParser::Statement - Current token: "
                << TokenEnumType::ToString(Current().type);

    // Process statement terminators first
    if (Try(TokenType::NewLine)) {
        KAI_TRACE() << "RhoParser::Statement - Consuming statement terminator";
        Consume();
        return true;
    }

    // Handle each statement type
    switch (Current().type) {
        case TokenType::Assert: {
            KAI_TRACE() << "RhoParser::Statement - Processing Assert";
            auto ass = NewNode(Consume());

            // Handle parentheses in assert statements (e.g.,
            // assert(expression))
            bool hasParentheses = false;
            if (Try(TokenType::OpenParan)) {
                hasParentheses = true;
                Consume();
            }

            if (!Expression()) {
                Fail(Lexer::CreateErrorMessage(
                    Current(), "Assert needs an expression to test"));
                return false;
            }

            // If opening parenthesis was used, expect closing parenthesis
            if (hasParentheses) {
                if (!Try(TokenType::CloseParan)) {
                    Fail(Lexer::CreateErrorMessage(
                        Current(),
                        "Expected closing parenthesis after assert condition"));
                    return false;
                }
                Consume();
            }

            ass->Add(Pop());
            block->Add(ass);
            goto finis;
        }

        case TokenType::Return:
        case TokenType::Yield: {
            KAI_TRACE() << "RhoParser::Statement - Processing Return/Yield";
            auto ret = NewNode(Consume());
            if (Expression()) ret->Add(Pop());
            block->Add(ret);
            goto finis;
        }

        case TokenType::If: {
            KAI_TRACE() << "RhoParser::Statement - Processing If";
            return IfCondition(block);
        }

        case TokenType::While: {
            KAI_TRACE() << "RhoParser::Statement - Processing While";
            if (!WhileLoop(block)) {
                return false;
            }
            // after handling a while loop, there may be an optional semicolon
            return true;
        }

        case TokenType::For: {
            KAI_TRACE() << "RhoParser::Statement - Processing For";
            if (!ForLoop(block)) {
                return false;
            }
            // after handling a for loop, there may be an optional semicolon
            return true;
        }

        case TokenType::DoWhile: {
            KAI_TRACE() << "RhoParser::Statement - Processing Do-While loop";

            // Use the proper DoWhileLoop function to parse the do-while
            // statement
            if (!DoWhileLoop(block)) {
                return false;
            }

            // After handling a do-while loop, there may be an optional
            // semicolon, as semicolons are optional in Rho
            return true;
        }

        case TokenType::Fun: {
            KAI_TRACE() << "RhoParser::Statement - Processing Function";
            return Function(block);
        }

        case TokenType::None:
            // End of input is okay
            return true;

        default:
            // Default case: assume it's an expression
            break;
    }

    KAI_TRACE() << "RhoParser::Statement - Processing Expression";
    if (!Expression()) return false;

    block->Add(Pop());

finis:
    // In Rho, statements can optionally end with a newline or semicolon
    // Both terminators are optional, similar to Python's style
    if (!Try(TokenType::None)) {
        // If there's a newline or semicolon, consume it
        if (Try(TokenType::NewLine)) {
            KAI_TRACE() << "RhoParser::Statement - Consuming newline";
            Consume();
        } else if (Try(TokenType::Semi)) {
            KAI_TRACE() << "RhoParser::Statement - Consuming semicolon";
            Consume();

            // After a semicolon, there may be another statement on the same
            // line which should be handled by calling Statement again, but only
            // if we're not in a context where semicolons are part of syntax
            // (like for loops)
            if (!Try(TokenType::NewLine) && !Try(TokenType::None) &&
                !Try(TokenType::CloseBrace) && !Try(TokenType::CloseParan)) {
                return Statement(block);
            }
        }
        // Unlike before, we don't require a newline or semicolon
        // This makes both terminators optional, allowing for flexible syntax
    }

    return true;
}

bool RhoParser::Expression() {
    KAI_TRACE() << "RhoParser::Expression - Starting";
    if (!Logical()) {
        KAI_TRACE() << "RhoParser::Expression - Logical() failed";
        return false;
    }

    if (Try(TokenType::Assign) || Try(TokenType::PlusAssign) ||
        Try(TokenType::MinusAssign) || Try(TokenType::MulAssign) ||
        Try(TokenType::DivAssign) || Try(TokenType::ModAssign)) {
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
    if (!Bitwise()) return false;

    while (Try(TokenType::And) || Try(TokenType::Or)) {
        auto node = NewNode(Consume());
        node->Add(Pop());
        if (!Bitwise()) return CreateError("Bitwise expected");

        node->Add(Pop());
        Push(node);
    }

    return true;
}

bool RhoParser::Bitwise() {
    if (!Relational()) return false;

    while (Try(TokenType::BitAnd) || Try(TokenType::BitOr) || Try(TokenType::BitXor)) {
        auto node = NewNode(Consume());
        node->Add(Pop());
        if (!Relational()) return CreateError("Relational expected");

        node->Add(Pop());
        Push(node);
    }

    return true;
}

bool RhoParser::Relational() {
    if (!Shift()) return false;

    while (Try(TokenType::Less) || Try(TokenType::Greater) ||
           Try(TokenType::Equiv) || Try(TokenType::NotEquiv) ||
           Try(TokenType::LessEquiv) || Try(TokenType::GreaterEquiv)) {
        auto node = NewNode(Consume());
        node->Add(Pop());
        if (!Shift()) return CreateError("Shift expected");

        node->Add(Pop());
        Push(node);
    }

    return true;
}

bool RhoParser::Shift() {
    if (!Additive()) return false;

    while (Try(TokenType::LeftShift) || Try(TokenType::RightShift)) {
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

    if (Try(TokenType::Not) || Try(TokenType::BitNot)) {
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

    while (Try(TokenType::Mul) || Try(TokenType::Divide) || Try(TokenType::Mod)) {
        auto node = NewNode(Consume());
        node->Add(Pop());
        if (!Factor()) return CreateError("Factor expected with a term");

        node->Add(Pop());
        Push(node);
    }

    return true;
}

bool RhoParser::Factor() {
    KAI_TRACE() << "RhoParser::Factor - Current token: " << TokenEnumType::ToString(Current().type);
    
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

    if (Try(TokenType::OpenBrace)) {
        auto map = NewNode(NodeType::Map);
        Consume();
        
        // Handle empty map case
        if (Try(TokenType::CloseBrace)) {
            Consume();
            Push(map);
            return true;
        }
        
        // For now, we don't support map literal syntax with key:value pairs
        // Just support empty maps {}
        Fail("Map literals with initial values not yet supported");
        return false;
    }

    if (Try(TokenType::Int) || Try(TokenType::Float) ||
        Try(TokenType::String) || Try(TokenType::True) || Try(TokenType::False))
        return PushConsume();

    if (Try(TokenType::Self)) return PushConsume();

    if (Try(TokenType::Label)) return ParseFactorIdent();

    if (Try(TokenType::Pathname)) return ParseFactorIdent();
    
    // Handle Pi code blocks - pi { ... }
    if (Try(TokenType::ToPi)) {
        KAI_TRACE() << "RhoParser::Factor - Found ToPi token";
        Consume();  // consume 'pi'
        
        // Expect opening brace
        if (!Try(TokenType::OpenBrace)) {
            return CreateError("Expected '{' after 'pi'");
        }
        
        auto piBlock = NewNode(NodeType::ToPiLang);
        Consume();  // consume '{'
        
        // Collect all tokens until closing brace as Pi code
        auto piContent = NewNode(NodeType::List);
        while (!Try(TokenType::CloseBrace) && !Failed) {
            // Consume tokens as part of the Pi block
            piContent->Add(NewNode(Consume()));
        }
        
        if (!Try(TokenType::CloseBrace)) {
            return CreateError("Expected '}' to close Pi block");
        }
        Consume();  // consume '}'
        
        piBlock->Add(piContent);
        Push(piBlock);
        return true;
    }

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
    get->Add(Expect(TokenType::Label));
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

    // Semicolons are optional in Rho

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

    KAI_TRACE() << "RhoParser::WhileLoop - Found 'while' token";
    Consume();

    // In Rho, we don't use parentheses for conditions
    KAI_TRACE() << "RhoParser::WhileLoop - Parsing condition expression";
    if (!Expression()) {
        return CreateError("While what? Expected condition expression");
    }

    auto condition = Pop();
    KAI_TRACE() << "RhoParser::WhileLoop - Parsed condition expression";

    // Expect newline after condition
    if (!Try(TokenType::NewLine)) {
        return CreateError("Expected newline after while condition");
    }
    Consume();

    auto bodyClause = NewNode(NodeType::Block);
    KAI_TRACE() << "RhoParser::WhileLoop - Created body block node";

    // Parse the body block
    KAI_TRACE() << "RhoParser::WhileLoop - Parsing body block";
    if (!Block(bodyClause)) {
        return CreateError("Block Expected for While loop body");
    }
    KAI_TRACE() << "RhoParser::WhileLoop - Parsed body block";

    auto whileNode = NewNode(NodeType::While);
    whileNode->Add(condition);
    whileNode->Add(bodyClause);
    KAI_TRACE()
        << "RhoParser::WhileLoop - Created while node with condition and body";

    if (!block->Add(whileNode)) {
        KAI_TRACE_ERROR()
            << "RhoParser::WhileLoop - Failed to add while node to block";
        return false;
    }

    KAI_TRACE()
        << "RhoParser::WhileLoop - Successfully added while node to block";
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
        KAI_TRACE() << "Next token after whitespace: "
                    << TokenEnumType::ToString(Current().type);
    }

    // Check for the 'while' token
    if (current >= tokens.size() || !Try(TokenType::While)) {
        if (current < tokens.size()) {
            KAI_TRACE_ERROR() << "Expected 'while', got: "
                              << TokenEnumType::ToString(Current().type);
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
        return CreateError(
            "Failed to parse condition expression for do-while loop");
    }

    // If we had opening parenthesis, expect a closing one
    if (hasParentheses) {
        ConsumeWhitespace();
        if (!Try(TokenType::CloseParan)) {
            return CreateError(
                "Expected closing parenthesis after do-while condition");
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

    KAI_TRACE() << "Do-while node created with "
                << (int)doWhileNode->GetChildren().size() << " children";

    // Add the whole do-while construct to the block
    if (!block->Add(doWhileNode)) {
        return CreateError("Failed to add do-while node to parent block");
    }

    KAI_TRACE() << "Do-while construct added to block successfully";
    return true;
}

void RhoParser::ConsumeWhitespace() {
    // Skip any whitespace, tabs, newlines, or semicolons
    while (Try(TokenType::Tab) || Try(TokenType::Whitespace) ||
           Try(TokenType::NewLine) || Try(TokenType::Semi)) {
        Consume();
    }
}

bool RhoParser::AddBlock(AstNodePtr fun) {
    auto block = NewNode(RhoAstNodeEnumType::Block);
    return Block(block) && fun->Add(block);
}

bool RhoParser::ForLoop(AstNodePtr block) {
    if (!Try(TokenType::For)) return false;

    KAI_TRACE() << "RhoParser::ForLoop - Found 'for' token";
    Consume();

    // Rho for loops have the syntax:
    // for init; condition; increment
    //     body
    // Semicolons are REQUIRED between the three parts
    // Parentheses are NOT allowed

    // Parse the initialization expression
    KAI_TRACE() << "RhoParser::ForLoop - Parsing initialization";
    AstNodePtr initExpr = nullptr;
    if (!Try(TokenType::Semi)) {
        // We have an initialization expression
        if (!Expression()) {
            return CreateError(
                "Expected initialization expression in for loop");
        }
        initExpr = Pop();
    }

    // Expect semicolon after initialization
    if (!Try(TokenType::Semi)) {
        return CreateError(
            "Expected semicolon after initialization in for loop");
    }
    Consume();

    // Parse the condition expression
    KAI_TRACE() << "RhoParser::ForLoop - Parsing condition";
    AstNodePtr condExpr = nullptr;
    if (!Try(TokenType::Semi)) {
        // We have a condition expression
        if (!Expression()) {
            return CreateError("Expected condition expression in for loop");
        }
        condExpr = Pop();
    }

    // Expect semicolon after condition
    if (!Try(TokenType::Semi)) {
        return CreateError("Expected semicolon after condition in for loop");
    }
    Consume();

    // Parse the increment expression
    KAI_TRACE() << "RhoParser::ForLoop - Parsing increment";
    AstNodePtr incrExpr = nullptr;
    // Always expect an increment expression (can be empty)
    if (!Expression()) {
        return CreateError("Expected increment expression in for loop");
    }
    incrExpr = Pop();

    // Expect newline after for loop header
    if (!Try(TokenType::NewLine)) {
        return CreateError("Expected newline after for loop declaration");
    }
    Consume();

    // Parse the body using indentation
    auto bodyClause = NewNode(NodeType::Block);
    KAI_TRACE() << "RhoParser::ForLoop - Parsing body block";

    // Use the Block method for indented code blocks
    if (!Block(bodyClause)) {
        return CreateError("Block Expected for for loop body");
    }

    KAI_TRACE() << "RhoParser::ForLoop - Parsed body block";

    // Create the for loop AST node structure
    auto forNode = NewNode(NodeType::For);

    // Add all parts to the for node (init, condition, increment, body)
    if (initExpr) {
        forNode->Add(initExpr);
    } else {
        // If no init expression, add an empty placeholder
        forNode->Add(NewNode(NodeType::None));
    }

    if (condExpr) {
        forNode->Add(condExpr);
    } else {
        // If no condition, add a 'true' literal to make it an infinite loop
        auto trueNode =
            NewNode(RhoToken(TokenEnum::True, *lexer.get(), 0, Slice()));
        forNode->Add(trueNode);
    }

    if (incrExpr) {
        forNode->Add(incrExpr);
    } else {
        // If no increment expression, add an empty placeholder
        forNode->Add(NewNode(NodeType::None));
    }

    forNode->Add(bodyClause);

    KAI_TRACE() << "RhoParser::ForLoop - Created for node with init, "
                   "condition, increment, and body";

    if (!block->Add(forNode)) {
        KAI_TRACE_ERROR()
            << "RhoParser::ForLoop - Failed to add for node to block";
        return false;
    }

    KAI_TRACE() << "RhoParser::ForLoop - Successfully added for node to block";
    return true;
}

bool RhoParser::ConsumeNewLines() {
    // Consume any number of newlines or semicolons
    // This is used to skip over statement separators (either newlines or
    // semicolons) in places where whitespace is expected or ignored
    while (Try(TokenType::NewLine) || Try(TokenType::Semi)) {
        if (Try(TokenType::NewLine)) {
            KAI_TRACE() << "RhoParser::ConsumeNewLines - Consuming newline";
        } else {
            KAI_TRACE() << "RhoParser::ConsumeNewLines - Consuming semicolon";
        }
        Consume();
    }
    return true;
}

KAI_END
