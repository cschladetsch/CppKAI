#include <KAI/Language/Common/ParserCommon.h>
#include <KAI/Language/Rho/RhoParser.h>

#include <iostream>
#include <unordered_set>

KAI_BEGIN

// Pi keywords that cannot be used as Rho variable names
static const std::unordered_set<std::string> piKeywords = {
    "if", "ife", "for", "foreach", "break", "continue", "true", "false", "self",
    "while", "assert", "div", "rho", "rho{", "to_str", "not", "and", "or", "xor",
    "exists", "drop", "dup", "dup2", "drop2", "pick", "over", "swap", "rot", "rotn",
    "roll", "toarray", "gc", "clear", "expand", "cd", "pwd", "type",
    "size", "depth", "new", "print", "dropn", "tolist", "tomap", "toset", "mul",
    "mod", "noteq", "lls", "ls", "freeze", "thaw", "at"
};

static bool IsPiKeyword(const std::string& name) {
    return piKeywords.find(name) != piKeywords.end();
}

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
    bool atLineStart = true;
    for (auto tok : lexer->GetTokens()) {
        // Keep whitespace at the beginning of lines for indentation
        if (tok.type == TokenEnum::Whitespace && atLineStart) {
            tokens.push_back(tok);
            KAI_TRACE() << "Token: " << TokenEnumType::ToString(tok.type)
                        << " (line start whitespace)";
        } else if (tok.type != TokenEnum::Whitespace &&
                   tok.type != TokenEnum::Comment) {
            tokens.push_back(tok);
            KAI_TRACE() << "Token: " << TokenEnumType::ToString(tok.type);
            atLineStart = false;
        }

        // Track when we're at the start of a new line
        if (tok.type == TokenEnum::NewLine) {
            atLineStart = true;
        }
    }

    root = NewNode(AstEnum::Program);
    KAI_TRACE() << "Created root Program node";

    return Run(st);
}

bool RhoParser::Run(Structure st) {
    switch (st) {
        case Structure::Statement:
            Program();
            break;

        case Structure::Expression:
            if (!Expression()) return CreateError("Expression expected");
            root->Add(Pop());
            break;

        case Structure::Function:
            // Functions are now only created via assignment syntax: a =
            // fun(b,c) So we treat it as an expression
            if (!Expression()) return CreateError("Expression expected");
            root->Add(Pop());
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

    // Skip any leading whitespace, newlines, or semicolons
    while (Try(TokenType::Whitespace) || Try(TokenType::NewLine) ||
           Try(TokenType::Semi)) {
        if (Try(TokenType::NewLine)) {
            KAI_TRACE() << "RhoParser::Program - Skipping initial newline";
        } else if (Try(TokenType::Whitespace)) {
            KAI_TRACE() << "RhoParser::Program - Skipping initial whitespace";
        } else {
            KAI_TRACE() << "RhoParser::Program - Skipping initial semicolon";
        }
        Consume();
    }

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
                    << TokenEnumType::ToString(Current().type) << " '"
                    << Current().Text() << "'" << " at position "
                    << (int)current;
        if (!Statement(root)) {
            return Fail("Statement expected");
        }

        KAI_TRACE() << "RhoParser::Program - Successfully parsed statement, "
                       "now at position "
                    << (int)current;

        // Note: We don't need special handling for semicolons here anymore
        // as they are properly consumed in the while loop at the top
        // and Statement() handles its own terminations
    }

    KAI_TRACE() << "RhoParser::Program - Finished parsing program";
    return true;
}

bool RhoParser::Block(AstNodePtr node) {
    ConsumeNewLines();

    ++indent;
    KAI_TRACE() << "Block: Starting with indent level " << indent;
    KAI_TRACE() << "Block: Current token after ConsumeNewLines: "
                << TokenEnumType::ToString(Current().type) << " '"
                << Current().Text() << "' at position " << (int)current;

    while (!Failed) {
        int level = 0;

        // Skip any newlines at the beginning of the block
        while (Try(TokenType::NewLine)) {
            Consume();
        }

        // Count indentation level - handle both tabs and spaces
        // Each tab counts as 1 level, every 4 spaces count as 1 level
        int spaceCount = 0;
        KAI_TRACE() << "Block: Checking for indentation at position "
                    << (int)current << ", current token: "
                    << TokenEnumType::ToString(Current().type) << " '"
                    << Current().Text() << "'";
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

        KAI_TRACE() << "Block: Found indent level " << level << " (expecting "
                    << indent << ") at position " << (int)current;

        if (Try(TokenType::NewLine)) {
            Consume();
            continue;
        }

        // close current block
        if (level < indent) {
            --indent;

            // rewind to start of indentation sequence to determine next block
            KAI_TRACE() << "Block: Before rewind, position " << (int)current
                        << ", token: "
                        << TokenEnumType::ToString(Current().type) << " '"
                        << Current().Text() << "'";

            // Special handling: if we're at an 'else' token, DON'T rewind
            // This allows the parent IfCondition to see the else
            if (current < tokens.size() && Current().type == TokenType::Else) {
                KAI_TRACE() << "Block: Exiting at else token, not rewinding";
                return true;
            }

            // Rewind to start of indentation sequence to determine next block
            --current;
            while (Try(TokenType::Tab) || Try(TokenType::Whitespace)) --current;

            ++current;

            KAI_TRACE() << "Block: After rewind, position " << (int)current
                        << ", token: "
                        << TokenEnumType::ToString(Current().type) << " '"
                        << Current().Text() << "'";

            // IMPORTANT: When we're exiting a block due to outdenting, we need
            // to preserve the ability for the parent to check what token caused
            // the outdent. This is especially important for 'else' tokens in
            // nested if statements.
            return true;
        }

        if (level != indent) {
            Fail(Lexer::CreateErrorMessage(Current(), "Mismatch block indent"));
            return false;
        }

        // Special case: if we see an 'else' token at the current indent level,
        // it might belong to a parent if statement, not this block
        if (Try(TokenType::Else)) {
            return true;
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
    KAI_TRACE() << "RhoParser::Statement - Current token type: "
                << (int)Current().type << " (" << Current().ToString() << ")";
    switch (Current().type) {
        case TokenType::Assert: {
            KAI_TRACE() << "RhoParser::Statement - Processing Assert";
            auto ass = NewNode(Consume());

            // Assert only supports the syntax: assert expression
            // No parentheses allowed
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
            KAI_TRACE() << "RhoParser::Statement - Processing Return/Yield";
            auto ret = NewNode(Consume());
            if (Expression()) ret->Add(Pop());
            block->Add(ret);
            goto finis;
        }

        case TokenType::Break: {
            KAI_TRACE() << "RhoParser::Statement - Processing Break";
            auto brk = NewNode(Consume());
            block->Add(brk);
            goto finis;
        }

        case TokenType::Continue: {
            KAI_TRACE() << "RhoParser::Statement - Processing Continue";
            auto cont = NewNode(Consume());
            block->Add(cont);
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
            // `fun` can start either:
            // 1) a named declaration: `fun name(args) ...`
            // 2) an anonymous function expression statement: `fun(args) ...`
            // Route to named-definition parsing only when a label follows.
            KAI_TRACE() << "RhoParser::Statement - Processing Fun";

            const int saved = current;
            Consume();  // consume 'fun'
            while (Try(TokenType::Whitespace) || Try(TokenType::Tab)) {
                Consume();
            }
            const bool isNamedFunction = Try(TokenType::Label);
            current = saved;

            if (isNamedFunction) {
                KAI_TRACE() << "RhoParser::Statement - Named function definition";
                return FunctionDefinition(block);
            }

            KAI_TRACE() << "RhoParser::Statement - Anonymous function expression";
            break;
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

    // Ternary conditional: condition ? thenExpr : elseExpr
    if (Try(TokenType::Question)) {
        auto condExpr = Pop();
        Consume();  // consume '?'

        if (!Expression()) {
            return CreateError("Expected expression after '?'");
        }
        auto trueExpr = Pop();

        if (!Try(TokenType::Colon)) {
            return CreateError("Expected ':' in ternary expression");
        }
        Consume();

        if (!Expression()) {
            return CreateError("Expected expression after ':'");
        }
        auto falseExpr = Pop();

        auto ternary = NewNode(NodeType::Conditional);
        auto trueBlock = NewNode(NodeType::Block);
        auto falseBlock = NewNode(NodeType::Block);
        trueBlock->Add(trueExpr);
        falseBlock->Add(falseExpr);
        ternary->Add(condExpr);
        ternary->Add(trueBlock);
        ternary->Add(falseBlock);
        Push(ternary);
        return true;
    }

    if (Try(TokenType::Assign) || Try(TokenType::PlusAssign) ||
        Try(TokenType::MinusAssign) || Try(TokenType::MulAssign) ||
        Try(TokenType::DivAssign) || Try(TokenType::ModAssign)) {
        auto node = NewNode(Consume());
        auto ident = Pop();

        // Validate that variable name is not a Pi keyword
        if (ident && ident->GetToken().type == TokenType::Label) {
            std::string varName = ident->GetTokenText();
            if (IsPiKeyword(varName)) {
                return CreateError(("Variable name '" + varName +
                    "' conflicts with Pi keyword and cannot be used").c_str());
            }
        }

        if (!Expression()) {
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

    while (Try(TokenType::BitAnd) || Try(TokenType::BitOr) ||
           Try(TokenType::BitXor)) {
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

    while (Try(TokenType::Mul) || Try(TokenType::Divide) ||
           Try(TokenType::Mod)) {
        auto node = NewNode(Consume());
        node->Add(Pop());
        if (!Factor()) return CreateError("Factor expected with a term");

        node->Add(Pop());
        Push(node);
    }

    return true;
}

bool RhoParser::Factor() {
    KAI_TRACE() << "RhoParser::Factor - Current token: "
                << TokenEnumType::ToString(Current().type);

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
        Consume();  // consume '['

        auto consumeLayout = [this]() {
            while (Try(TokenType::Whitespace) || Try(TokenType::Tab) ||
                   Try(TokenType::NewLine)) {
                Consume();
            }
        };

        consumeLayout();
        while (!Try(TokenType::CloseSquareBracket)) {
            if (Try(TokenType::CloseSquareBracket)) break;
            if (Expression()) {
                list->Add(Pop());
            } else {
                Fail("Badly formed array");
                return false;
            }

            consumeLayout();
            if (Try(TokenType::Comma)) {
                Consume();
                consumeLayout();
                continue;
            }

            if (!Try(TokenType::CloseSquareBracket)) {
                Fail("Expected ',' or ']' in array literal");
                return false;
            }
        }

        Expect(TokenType::CloseSquareBracket);
        Push(list);

        return true;
    }

    if (Try(TokenType::OpenBrace)) {
        auto map = NewNode(NodeType::Map);
        Consume();

        auto consumeLayout = [this]() {
            while (Try(TokenType::Whitespace) || Try(TokenType::Tab) ||
                   Try(TokenType::NewLine)) {
                Consume();
            }
        };

        consumeLayout();

        // Handle empty map case
        if (Try(TokenType::CloseBrace)) {
            Consume();
            Push(map);
            return true;
        }

        // Parse key-value pairs
        do {
            consumeLayout();

            // Parse key (must be a string for now)
            if (!Try(TokenType::String)) {
                Fail("Map key must be a string");
                return false;
            }

            auto key = Current();
            Consume();

            consumeLayout();

            // Expect colon
            if (!Try(TokenType::Colon)) {
                Fail("Expected ':' after map key");
                return false;
            }
            Consume();

            consumeLayout();

            // Parse value expression
            if (!Expression()) {
                Fail("Expected expression for map value");
                return false;
            }

            // Pop the value
            auto value = Pop();

            // Add key-value pair to map node
            // Store key as a token node and value as the expression
            auto keyNode = NewNode(key);
            map->Add(keyNode);
            map->Add(value);

            consumeLayout();

            // Check for more pairs
            if (Try(TokenType::Comma)) {
                Consume();
                continue;
            }

            // Must end with close brace
            consumeLayout();
            if (!Try(TokenType::CloseBrace)) {
                Fail("Expected '}' or ',' in map literal");
                return false;
            }
            break;

        } while (true);

        Consume();  // consume the CloseBrace
        Push(map);
        return true;
    }

    if (Try(TokenType::Int) || Try(TokenType::Float) ||
        Try(TokenType::String) || Try(TokenType::True) ||
        Try(TokenType::False) || Try(TokenType::ShellCommand))
        return PushConsume();

    if (Try(TokenType::Self)) return PushConsume();

    if (Try(TokenType::Label)) return ParseFactorIdent();

    if (Try(TokenType::Pathname)) return ParseFactorIdent();

    // Handle function expressions - fun(args) { ... }
    if (Try(TokenType::Fun)) {
        KAI_TRACE() << "RhoParser::Factor - Found function expression";
        Consume();  // consume 'fun'

        // Create function node without a name (anonymous function)
        auto fun = NewNode(AstEnum::Function);

        // Add empty name token for anonymous function
        Slice anonymousSlice;
        fun->Add(RhoToken(TokenEnum::Label, *lexer.get(), 0, anonymousSlice));

        // Parse parameters with parentheses (required for clarity)
        Expect(TokenType::OpenParan);
        auto args = NewNode(AstEnum::None);
        fun->Add(args);

        if (Try(TokenType::Label)) {
            args->Add(Consume());
            while (Try(TokenType::Comma)) {
                Consume();
                args->Add(Expect(TokenType::Label));
            }
        }

        Expect(TokenType::CloseParan);

        auto block = NewNode(RhoAstNodeEnumType::Block);

        // Rho supports two anonymous function body styles:
        // 1. Inline expression with braces: fun(args) { expr }
        // 2. Indented block: fun(args)\n    body
        if (Try(TokenType::OpenBrace)) {
            Consume();  // consume '{'

            if (!Expression()) {
                return CreateError(
                    "Expected expression inside anonymous function braces");
            }

            auto expr = Pop();
            block->Add(expr);

            if (!Try(TokenType::CloseBrace)) {
                return CreateError(
                    "Expected '}' after anonymous function expression");
            }
            Consume();  // consume '}'
        } else {
            Expect(TokenType::NewLine);

            if (!Block(block)) {
                return CreateError("Block Expected for function body");
            }
        }

        fun->Add(block);
        Push(fun);
        return true;
    }

    // Handle Pi code blocks - pi { ... }
    if (Try(TokenType::ToPi)) {
        KAI_TRACE() << "RhoParser::Factor - Found ToPi token";
        Consume();  // consume 'pi'
        ConsumeWhitespace();

        // Expect opening brace
        if (!Try(TokenType::OpenBrace)) {
            return CreateError("Expected '{' after 'pi'");
        }

        auto piBlock = NewNode(NodeType::ToPiLang);
        Consume();  // consume '{'

        // Collect all tokens until closing brace as Pi code
        // Need to handle nested braces properly
        auto piContent = NewNode(NodeType::List);
        int braceCount = 1;  // We've already consumed the opening brace

        while (braceCount > 0 && !Failed) {
            if (Try(TokenType::OpenBrace)) {
                braceCount++;
                piContent->Add(NewNode(Consume()));
            } else if (Try(TokenType::CloseBrace)) {
                braceCount--;
                if (braceCount > 0) {
                    // This is a nested closing brace, include it in Pi content
                    piContent->Add(NewNode(Consume()));
                } else {
                    Consume();
                }
                // If braceCount == 0, we've found the closing brace of the Pi
                // block
            } else {
                // Regular token, add to Pi content
                piContent->Add(NewNode(Consume()));
            }
        }

        if (braceCount != 0) {
            return CreateError("Expected '}' to close Pi block");
        }
        // The closing brace has already been consumed by the loop

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
    KAI_TRACE() << "IfCondition: Entry";
    if (!Try(TokenType::If)) {
        KAI_TRACE() << "IfCondition: Not an If token, returning false";
        return false;
    }

    KAI_TRACE() << "IfCondition: Found If token, consuming";
    Consume();

    // No parentheses in Rho
    if (!Expression()) {
        return CreateError("If what?");
    }

    auto condition = Pop();

    auto parseInlineBlock = [this](AstNodePtr inlineBlock) -> bool {
        if (!Try(TokenType::OpenBrace)) {
            return false;
        }
        Consume();  // consume '{'

        while (!Try(TokenType::CloseBrace) && !Try(TokenType::None)) {
            // Skip whitespace tokens inside inline blocks
            while (Try(TokenType::Whitespace) || Try(TokenType::Tab) ||
                   Try(TokenType::NewLine)) {
                Consume();
            }

            if (Try(TokenType::CloseBrace)) {
                break;
            }

            if (!Statement(inlineBlock)) {
                return CreateError("Expected statement in inline if block");
            }
        }

        if (!Try(TokenType::CloseBrace)) {
            return CreateError("Expected '}' to close inline if block");
        }
        Consume();  // consume '}'
        return true;
    };

    while (Try(TokenType::Whitespace) || Try(TokenType::Tab)) {
        Consume();
    }

    auto trueClause = NewNode(NodeType::Block);

    if (Try(TokenType::OpenBrace)) {
        if (!parseInlineBlock(trueClause)) {
            return CreateError("Inline block expected for if body");
        }
    } else {
        if (!Try(TokenType::NewLine)) {
            return CreateError("Expected newline or '{' after if condition");
        }
        Consume();
        if (!Block(trueClause)) {
            return CreateError("Block Expected for if body");
        }
    }

    auto cond = NewNode(NodeType::Conditional);
    cond->Add(condition);
    cond->Add(trueClause);

    KAI_TRACE() << "IfCondition: Checking for else at position " << (int)current
                << ", token: " << TokenEnumType::ToString(Current().type);

    if (Try(TokenType::Else)) {
        KAI_TRACE() << "IfCondition: Found else at position " << (int)current;
        Consume();
        KAI_TRACE() << "IfCondition: After consuming else, position "
                    << (int)current;

        // Check if this is an "else if" case
        if (Try(TokenType::If)) {
            KAI_TRACE() << "IfCondition: Found 'else if' pattern";
            // This is an "else if" - parse it as a nested if statement
            auto falseClause = NewNode(NodeType::Block);
            if (!IfCondition(falseClause)) {
                return CreateError("Failed to parse else if condition");
            }
            cond->Add(falseClause);
        } else {
            auto falseClause = NewNode(NodeType::Block);

            // Regular else block - allow inline braces or indented block
            if (Try(TokenType::OpenBrace)) {
                if (!parseInlineBlock(falseClause)) {
                    return CreateError("Inline block expected for else body");
                }
            } else {
                if (!Try(TokenType::NewLine)) {
                    return CreateError("Expected newline or '{' after else");
                }
                Consume();

                KAI_TRACE() << "IfCondition: Parsing else block";
                Block(falseClause);
                KAI_TRACE() << "IfCondition: Finished else block, position "
                            << (int)current;
            }

            cond->Add(falseClause);
        }
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

    if (!Try(TokenType::CloseSquareBracket)) {
        return CreateError("Expected ']' after index expression");
    }
    Consume();
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

    // Expect newline, semicolon, or brace after while condition
    while (Try(TokenType::Whitespace) || Try(TokenType::Tab)) {
        Consume();
    }
    bool whileSingleLine = false;
    bool whileBraceBlock = false;
    if (Try(TokenType::Semi)) {
        whileSingleLine = true;
        Consume();
    } else if (Try(TokenType::OpenBrace)) {
        whileBraceBlock = true;
        Consume();  // consume '{'
    } else if (Try(TokenType::NewLine)) {
        Consume();
    } else {
        return CreateError("Expected newline, ';', or '{' after while condition");
    }

    auto bodyClause = NewNode(NodeType::Block);
    KAI_TRACE() << "RhoParser::WhileLoop - Parsing body block (single-line: "
                << whileSingleLine << ", brace: " << whileBraceBlock << ")";

    if (whileSingleLine) {
        if (!Statement(bodyClause)) {
            return CreateError("Statement expected for while loop body");
        }
    } else if (whileBraceBlock) {
        if (!Block(bodyClause)) {
            return CreateError("Block Expected inside while loop braces");
        }
        while (Try(TokenType::NewLine) || Try(TokenType::Whitespace) || Try(TokenType::Tab)) {
            Consume();
        }
        if (!Try(TokenType::CloseBrace)) {
            return CreateError("Expected '}' to close while loop body");
        }
        Consume();
    } else if (!Block(bodyClause)) {
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

    bool headerHasParens = false;
    if (Try(TokenType::OpenParan)) {
        headerHasParens = true;
        Consume();
    }

    // Rho for loops support TWO syntaxes:
    // 1. C-style: for i = 0; i < 10; i = i + 1
    // 2. Iterator-style: for x in container
    //
    // Strategy: Peek ahead to detect which syntax

    // Peek to see if this is iterator-style (identifier followed by 'in')
    if (Try(TokenType::Label)) {
        // Save current position
        int savedPos = current;

        // Create node for the identifier (consume it)
        auto loopVar = NewNode(Consume());

        // Validate that loop variable name is not a Pi keyword
        std::string loopVarName = loopVar->GetTokenText();
        if (IsPiKeyword(loopVarName)) {
            return CreateError(("Loop variable name '" + loopVarName +
                "' conflicts with Pi keyword and cannot be used").c_str());
        }

        // Check if next token is 'in'
        if (Try(TokenType::In)) {
            // Iterator-style: for x in container
            KAI_TRACE() << "RhoParser::ForLoop - Iterator-style 'for x in collection'";
            Consume(); // consume 'in'

            // Parse the collection expression
            if (!Expression()) {
                return CreateError("Expected collection expression after 'in'");
            }
            AstNodePtr collection = Pop();

            if (headerHasParens) {
                if (!Try(TokenType::CloseParan)) {
                    return CreateError("Expected ')' after for-loop header");
                }
                Consume();
            }

            // Expect newline
            if (!Try(TokenType::NewLine)) {
                return CreateError("Expected newline after for-loop header");
            }
            Consume();

            // Parse body
            auto bodyClause = NewNode(NodeType::Block);
            if (!Block(bodyClause)) {
                return CreateError("Block expected for for-loop body");
            }

            // Create ForEach node (iterator-style for loops use foreach internally)
            auto forEachNode = NewNode(NodeType::ForEach);
            forEachNode->Add(loopVar);      // loop variable
            forEachNode->Add(collection);   // collection
            forEachNode->Add(bodyClause);   // body
            block->Add(forEachNode);

            KAI_TRACE() << "RhoParser::ForLoop - Iterator-style for-loop complete";
            return true;
        }

        // Not iterator-style, restore position and parse as C-style
        current = savedPos;
    }

    // C-style: for i = 0; i < 10; i = i + 1
    KAI_TRACE() << "RhoParser::ForLoop - C-style for-loop";

    // Parse the initialization expression
    if (!Expression()) {
        return CreateError("Expected initialization expression in for-loop");
    }
    AstNodePtr initExpr = Pop();

    // Expect semicolon after initialization
    if (!Try(TokenType::Semi)) {
        return CreateError("Expected semicolon after initialization in for loop");
    }
    Consume();

    // Parse the condition expression
    KAI_TRACE() << "RhoParser::ForLoop - Parsing condition";
    AstNodePtr condExpr = nullptr;
    if (!Try(TokenType::Semi)) {
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
    // Increment expression is optional (can be empty before newline)
    if (!Try(TokenType::NewLine) && !(headerHasParens && Try(TokenType::CloseParan))) {
        if (!Expression()) {
            return CreateError("Expected increment expression in for loop");
        }
        incrExpr = Pop();
    }

    if (headerHasParens) {
        if (!Try(TokenType::CloseParan)) {
            return CreateError("Expected ')' after for-loop header");
        }
        Consume();
    }

    // Skip optional whitespace between header and body delimiter.
    while (Try(TokenType::Whitespace) || Try(TokenType::Tab)) {
        Consume();
    }

    // Parse the body: three forms are accepted.
    //   '{' ... '}'  — brace-delimited inline block (e.g. C-style)
    //   '\n' BLOCK   — indented multi-line block (Rho default)
    //   ';'  STMT    — single-statement on the same line
    auto bodyClause = NewNode(NodeType::Block);
    KAI_TRACE() << "RhoParser::ForLoop - Parsing body block";

    if (Try(TokenType::OpenBrace)) {
        // Brace block: consume '{', then statements until '}'
        Consume();
        while (!Try(TokenType::CloseBrace) && !Try(TokenType::None)) {
            while (Try(TokenType::Whitespace) || Try(TokenType::Tab) ||
                   Try(TokenType::NewLine)) {
                Consume();
            }
            if (Try(TokenType::CloseBrace)) break;
            if (!Statement(bodyClause)) {
                return CreateError("Expected statement in for loop body");
            }
        }
        if (!Try(TokenType::CloseBrace)) {
            return CreateError("Expected '}' to close for loop body");
        }
        Consume();
    } else if (Try(TokenType::NewLine)) {
        Consume();
        if (!Block(bodyClause)) {
            return CreateError("Block Expected for for loop body");
        }
    } else if (Try(TokenType::Semi)) {
        // Single-statement form: "for init; cond; incr; stmt"
        Consume();
        if (!Statement(bodyClause)) {
            return CreateError("Statement expected for for loop body");
        }
    } else {
        return CreateError("Expected '{', newline, or ';' after for loop header");
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

bool RhoParser::ForEachLoop(AstNodePtr block) {
    KAI_TRACE() << "RhoParser::ForEachLoop - Starting foreach loop parsing";

    // foreach item in collection
    //     body

    // Create the foreach node
    auto forEachNode = NewNode(NodeType::ForEach);
    if (!forEachNode) {
        KAI_TRACE_ERROR()
            << "RhoParser::ForEachLoop - Failed to create foreach node";
        return false;
    }

    // Consume the 'foreach' keyword
    Consume();
    KAI_TRACE() << "RhoParser::ForEachLoop - Consumed 'foreach' keyword";

    // Expect an identifier for the loop variable
    if (!Try(TokenType::Label)) {
        return CreateError("Expected identifier after 'foreach'");
    }

    auto loopVar = NewNode(Consume());
    if (!loopVar) {
        KAI_TRACE_ERROR()
            << "RhoParser::ForEachLoop - Failed to create loop variable node";
        return false;
    }
    forEachNode->Add(loopVar);
    KAI_TRACE() << "RhoParser::ForEachLoop - Added loop variable: "
                << loopVar->GetToken().Text();

    // Expect 'in' keyword
    if (!Try(TokenType::In)) {
        KAI_TRACE_ERROR()
            << "ForEachLoop: Expected 'in' after loop variable, got: "
            << Current().ToString();
        return CreateError("Expected 'in' after loop variable");
    }

    Consume();  // consume 'in'
    KAI_TRACE() << "RhoParser::ForEachLoop - Consumed 'in' keyword";

    // Parse the collection expression
    if (!Expression()) {
        return CreateError("Expected collection expression after 'in'");
    }

    auto collection = Pop();
    if (!collection) {
        KAI_TRACE_ERROR()
            << "RhoParser::ForEachLoop - Failed to get collection expression";
        return false;
    }
    forEachNode->Add(collection);
    KAI_TRACE() << "RhoParser::ForEachLoop - Added collection expression";

    // Expect newline before body
    if (!Try(TokenType::NewLine)) {
        return CreateError("Expected newline after foreach header");
    }
    Consume();

    // Parse the body as a block
    auto body = NewNode(NodeType::Block);
    if (!body) {
        KAI_TRACE_ERROR()
            << "RhoParser::ForEachLoop - Failed to create body block";
        return false;
    }

    // Use the standard Block parsing method
    KAI_TRACE() << "RhoParser::ForEachLoop - Parsing body block";
    if (!Block(body)) {
        return CreateError("Block expected for foreach loop body");
    }
    KAI_TRACE() << "RhoParser::ForEachLoop - Parsed body block";
    forEachNode->Add(body);
    KAI_TRACE() << "RhoParser::ForEachLoop - Added body block with "
                << static_cast<int>(body->GetChildren().size())
                << " statements";

    if (!block->Add(forEachNode)) {
        KAI_TRACE_ERROR()
            << "RhoParser::ForEachLoop - Failed to add foreach node to block";
        return false;
    }

    KAI_TRACE()
        << "RhoParser::ForEachLoop - Successfully added foreach node to block";
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

bool RhoParser::FunctionDefinition(AstNodePtr block) {
    KAI_TRACE() << "RhoParser::FunctionDefinition - Processing named function definition";
    
    // Consume the 'fun' token
    if (!Try(TokenType::Fun)) {
        return CreateError("Expected 'fun' keyword");
    }
    Consume();
    
    // Create function node
    auto fun = NewNode(AstEnum::Function);
    
    // Parse function name
    if (!Try(TokenType::Label)) {
        return CreateError("Expected function name after 'fun'");
    }
    fun->Add(Consume());  // Add function name
    
    // Parse parameters with parentheses
    if (!Try(TokenType::OpenParan)) {
        return CreateError("Expected '(' after function name");
    }
    Expect(TokenType::OpenParan);
    
    auto args = NewNode(AstEnum::None);
    fun->Add(args);
    
    if (Try(TokenType::Label)) {
        args->Add(Consume());
        while (Try(TokenType::Comma)) {
            Consume();
            args->Add(Expect(TokenType::Label));
        }
    }
    
    if (!Try(TokenType::CloseParan)) {
        return CreateError("Expected ')' after function parameters");
    }
    Expect(TokenType::CloseParan);
    
    auto functionBlock = NewNode(RhoAstNodeEnumType::Block);

    // Rho supports two function body styles:
    // 1. Python-like indented blocks (multiline):
    //    fun name(args)
    //        body
    // 2. Inline expression with braces (single-line):
    //    fun name(args) { expr }

    if (Try(TokenType::OpenBrace)) {
        KAI_TRACE() << "RhoParser::FunctionDefinition - Parsing brace-delimited function body";
        Consume();  // consume '{'

        // Check if this is a multi-line block (newline after '{') or inline expression
        if (Try(TokenType::NewLine)) {
            // Multi-line brace block: parse as indented block, then consume '}'
            if (!Block(functionBlock)) {
                return CreateError("Failed to parse multi-line function body");
            }
            // Skip any trailing whitespace/newlines before '}'
            while (Try(TokenType::NewLine) || Try(TokenType::Whitespace) || Try(TokenType::Tab)) {
                Consume();
            }
            if (!Try(TokenType::CloseBrace)) {
                return CreateError("Expected '}' to close multi-line function body");
            }
            Consume();  // consume '}'
        } else {
            // Inline brace block: parse statements until '}'
            // Handles both simple expressions and complex statements (if/else, etc.)
            while (!Try(TokenType::CloseBrace) && !Try(TokenType::None)) {
                while (Try(TokenType::Whitespace) || Try(TokenType::Tab) || Try(TokenType::NewLine)) {
                    Consume();
                }
                if (Try(TokenType::CloseBrace)) break;
                if (!Statement(functionBlock)) {
                    return CreateError("Expected statement in inline function body");
                }
            }
            if (!Try(TokenType::CloseBrace)) {
                return CreateError("Expected '}' after inline function body");
            }
            Consume();  // consume '}'
        }
    } else {
        // Python-like indentation-style function bodies
        if (!Try(TokenType::NewLine)) {
            return CreateError("Expected newline or '{' after function declaration");
        }
        Expect(TokenType::NewLine);

        // Use the Block method for indented code blocks
        if (!Block(functionBlock)) {
            return CreateError("Failed to parse function body");
        }
    }
    
    fun->Add(functionBlock);
    block->Add(fun);
    
    KAI_TRACE() << "RhoParser::FunctionDefinition - Successfully parsed function definition";
    return true;
}

KAI_END
