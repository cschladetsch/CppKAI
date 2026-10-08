#include <KAI/Language/Sigma/SigmaParser.h>

#include <format>
#include <algorithm>
#include <array>
#include <string_view>

KAI_BEGIN

namespace {
// Sigma compiles to Rho, which compiles to Pi; these names are Pi words and
// cannot be bound by Rho code (see RhoParser).
bool IsPiKeyword(const std::string &name) {
    constexpr auto words = std::to_array<std::string_view>({
        "if",    "ife",    "for",    "foreach", "break", "continue", "true",  "false", "self",  "while",
        "assert", "div",   "rho",    "rho{",    "to_str", "not",     "and",   "or",    "xor",   "exists",
        "drop",  "dup",    "dup2",   "drop2",   "pick",  "over",     "swap",  "rot",   "rotn",  "roll",
        "toarray", "gc",   "clear",  "expand",  "cd",    "pwd",      "type",  "size",  "depth", "new",
        "print", "dropn",  "tolist", "tomap",   "toset", "mul",      "mod",   "noteq", "lls",   "ls",
        "freeze", "thaw",  "at"});
    return std::find(words.begin(), words.end(), name) != words.end();
}
}  // namespace

const char *SigmaAstNodeEnumType::ToString(Enum val) {
    switch (val) {
#define CASE(N) \
    case N:     \
        return #N;
        CASE(None) CASE(TokenType) CASE(Object) CASE(Program) CASE(Block) CASE(Function) CASE(Params) CASE(Param)
        CASE(Type) CASE(Declaration) CASE(Assignment) CASE(If) CASE(While) CASE(DoWhile) CASE(For) CASE(ForEach)
        CASE(Return) CASE(Break) CASE(Continue) CASE(Assert) CASE(ExprStatement) CASE(Binary) CASE(Unary)
        CASE(Ternary) CASE(Call) CASE(Args) CASE(Index) CASE(Member) CASE(List) CASE(Map) CASE(MapEntry)
        CASE(PiBlock) CASE(TypeParams) CASE(Expand) CASE(Fold)
#undef CASE
    }
    return "Unknown";
}

SigmaParser::TokenNode const &SigmaParser::Peek(size_t ahead) const {
    const size_t at = current + ahead;
    return at < tokens_.size() ? tokens_[at] : tokens_.back();
}

bool SigmaParser::Is(Tok::Enum type) const { return Peek().type == type; }

SigmaParser::TokenNode SigmaParser::Take() {
    TokenNode tok = Peek();
    if (current < tokens_.size() - 1) ++current;  // never step past End
    return tok;
}

bool SigmaParser::Accept(Tok::Enum type) {
    if (!Is(type)) return false;
    Take();
    return true;
}

bool SigmaParser::Require(Tok::Enum type, const char *what) {
    if (Accept(type)) return true;
    Error(std::format("expected {}", what).c_str());
    return false;
}

SigmaParser::AstNodePtr SigmaParser::Error(TokenNode const &at, const std::string &message) {
    if (!failed) Fail(std::format("{}:{}: {}", at.lineNumber + 1, at.slice.Start + 1, message));
    return nullptr;
}

SigmaParser::AstNodePtr SigmaParser::Error(const char *message) {
    auto const &at = Peek();
    std::string found;
    switch (at.type) {
        case Tok::NewLine: found = "end of line"; break;
        case Tok::End: found = "end of input"; break;
        case Tok::Indent: found = "indent"; break;
        case Tok::Dedent: found = "dedent"; break;
        default: found = "'" + at.Text() + "'"; break;
    }
    return Error(at, std::format("{} (found {})", message, found));
}

bool SigmaParser::CheckName(TokenNode const &tok, const char *what) {
    if (!IsPiKeyword(tok.Text())) return true;
    Error(tok, std::format("'{}' is a Pi word and cannot be used as a {} name", tok.Text(), what));
    return false;
}

bool SigmaParser::Process(std::shared_ptr<Lexer> lex, Structure) {
    lexer_ = lex;
    tokens_.assign(lex->GetTokens().begin(), lex->GetTokens().end());
    current = 0;
    root_ = NewNode(Ast::Program);
    if (tokens_.empty()) {
        Fail("1:1: no input");
        return false;
    }
    return Program();
}

bool SigmaParser::Program() {
    while (!failed && !Is(Tok::End)) {
        if (Accept(Tok::NewLine)) continue;
        if (Is(Tok::Indent)) return Error("unexpected indent") != nullptr;
        auto stmt = Statement();
        if (!stmt) return false;
        root_->Add(stmt);
    }
    return !failed;
}

bool SigmaParser::EndOfStatement() {
    if (Accept(Tok::NewLine)) return true;
    if (Accept(Tok::Semi)) {
        Accept(Tok::NewLine);
        return true;
    }
    if (Is(Tok::End) || Is(Tok::Dedent)) return true;
    Error("expected end of statement");
    return false;
}

SigmaParser::AstNodePtr SigmaParser::Statement() {
    switch (Peek().type) {
        case Tok::Fun:
            if (Peek(1).type != Tok::Name) return Error("anonymous functions are not supported in Sigma yet");
            return FunctionDefinition();
        case Tok::If: return IfStatement();
        case Tok::While: return WhileStatement();
        case Tok::Do: return DoWhileStatement();
        case Tok::For: return ForStatement();

        case Tok::Return: {
            auto ret = ReturnStatement();
            return ret && EndOfStatement() ? ret : nullptr;
        }
        case Tok::Break:
        case Tok::Continue: {
            const auto tok = Take();
            auto node = NewNode(tok.type == Tok::Break ? Ast::Break : Ast::Continue, tok);
            return EndOfStatement() ? node : nullptr;
        }
        case Tok::Assert: {
            auto node = NewNode(Ast::Assert, Take());
            if (!Require(Tok::OpenParen, "'(' after assert")) return nullptr;
            auto cond = Expression();
            if (!cond || !Require(Tok::CloseParen, "')'")) return nullptr;
            node->Add(cond);
            return EndOfStatement() ? node : nullptr;
        }
        case Tok::Yield: return Error("'yield' is not supported in Sigma yet");
        case Tok::Else: return Error("'else' without a matching 'if'");
        default: {
            auto stmt = SimpleStatement();
            return stmt && EndOfStatement() ? stmt : nullptr;
        }
    }
}

SigmaParser::AstNodePtr SigmaParser::Block() {
    if (!Require(Tok::NewLine, "a new line before the block")) return nullptr;
    if (!Is(Tok::Indent)) return Error("expected an indented block");
    auto block = NewNode(Ast::Block, Take());
    while (!failed && !Is(Tok::Dedent) && !Is(Tok::End)) {
        if (Accept(Tok::NewLine)) continue;
        auto stmt = Statement();
        if (!stmt) return nullptr;
        block->Add(stmt);
    }
    if (!Require(Tok::Dedent, "the end of the block")) return nullptr;
    if (block->GetChildren().empty()) return Error("empty block");
    return block;
}

SigmaParser::AstNodePtr SigmaParser::FunctionDefinition() {
    if (functionDepth_ > 0) return Error("nested functions are not supported in Sigma yet");
    Take();  // fun
    const auto name = Take();
    if (!CheckName(name, "function")) return nullptr;
    auto fun = NewNode(Ast::Function, name);

    // Template parameters: `fun max[T](a: T, b: T) -> T`. The type arguments
    // are inferred at each call; the checker checks the body once per
    // distinct set of them. Rho is untyped, so the output is the same.
    AstNodePtr typeParams;
    if (Is(Tok::OpenSquare)) {
        typeParams = NewNode(Ast::TypeParams, Take());
        bool pack = false;
        do {
            if (pack) return Error("a type parameter pack ('...Ts') must be the last type parameter");
            pack = Accept(Tok::Ellipsis);
            if (!Is(Tok::Name)) return Error(pack ? "expected a name after '...'" : "expected a type parameter name");
            const auto t = Take();
            if (!CheckName(t, "type parameter")) return nullptr;
            typeParams->Add(pack ? NewNode(Ast::Expand, t) : NewNode(t));
        } while (Accept(Tok::Comma));
        if (!Require(Tok::CloseSquare, "']' after the type parameters")) return nullptr;
    }

    if (!Require(Tok::OpenParen, "'(' after the function name")) return nullptr;
    auto params = NewNode(Ast::Params);
    if (!Is(Tok::CloseParen)) {
        do {
            if (!Is(Tok::Name)) return Error("expected a parameter name");
            const auto p = Take();
            if (!CheckName(p, "parameter")) return nullptr;
            if (!Is(Tok::Colon)) return Error(p, std::format("parameter '{}' needs a type, e.g. '{}: int'", p.Text(), p.Text()));
            Take();
            auto type = Type();
            if (!type) return nullptr;
            auto param = NewNode(Ast::Param, p);
            param->Add(type);
            if (Is(Tok::Ellipsis)) param->Add(NewNode(Take()));  // `xs: Ts...`, a pack parameter
            params->Add(param);
        } while (Accept(Tok::Comma));
    }
    if (!Require(Tok::CloseParen, "')' after the parameters")) return nullptr;
    fun->Add(params);

    AstNodePtr result = NewNode(Ast::None);
    if (Accept(Tok::Arrow)) {
        result = Type();
        if (!result) return nullptr;
    }
    fun->Add(result);

    ++functionDepth_;
    auto body = Block();
    --functionDepth_;
    if (!body) return nullptr;
    fun->Add(body);
    if (typeParams) fun->Add(typeParams);
    return fun;
}

SigmaParser::AstNodePtr SigmaParser::IfStatement() {
    auto node = NewNode(Ast::If, Take());
    auto cond = Expression();
    if (!cond) return nullptr;
    auto then = Block();
    if (!then) return nullptr;
    node->Add(cond);
    node->Add(then);

    if (Is(Tok::Else)) {
        Take();
        auto other = Is(Tok::If) ? IfStatement() : Block();
        if (!other) return nullptr;
        node->Add(other);
    }
    return node;
}

SigmaParser::AstNodePtr SigmaParser::WhileStatement() {
    auto node = NewNode(Ast::While, Take());
    auto cond = Expression();
    if (!cond) return nullptr;
    auto body = Block();
    if (!body) return nullptr;
    node->Add(cond);
    node->Add(body);
    return node;
}

SigmaParser::AstNodePtr SigmaParser::DoWhileStatement() {
    auto node = NewNode(Ast::DoWhile, Take());
    auto body = Block();
    if (!body) return nullptr;
    if (!Require(Tok::While, "'while' after the do block")) return nullptr;
    auto cond = Expression();
    if (!cond || !EndOfStatement()) return nullptr;
    node->Add(body);
    node->Add(cond);
    return node;
}

SigmaParser::AstNodePtr SigmaParser::ForStatement() {
    const auto forTok = Take();

    if (Is(Tok::Name) && Peek(1).type == Tok::In) {
        const auto var = Take();
        if (!CheckName(var, "loop variable")) return nullptr;
        Take();  // in
        auto iterable = Expression();
        if (!iterable) return nullptr;
        auto body = Block();
        if (!body) return nullptr;
        auto node = NewNode(Ast::ForEach, var);
        node->Add(iterable);
        node->Add(body);
        return node;
    }

    auto node = NewNode(Ast::For, forTok);
    AstNodePtr init = Is(Tok::Semi) ? NewNode(Ast::None) : SimpleStatement();
    if (!init || !Require(Tok::Semi, "';' after the for-loop initialiser")) return nullptr;
    AstNodePtr cond = Is(Tok::Semi) ? NewNode(Ast::None) : Expression();
    if (!cond || !Require(Tok::Semi, "';' after the for-loop condition")) return nullptr;
    AstNodePtr step = Is(Tok::NewLine) ? NewNode(Ast::None) : SimpleStatement();
    if (!step) return nullptr;
    auto body = Block();
    if (!body) return nullptr;
    node->Add(init);
    node->Add(cond);
    node->Add(step);
    node->Add(body);
    return node;
}

SigmaParser::AstNodePtr SigmaParser::ReturnStatement() {
    auto node = NewNode(Ast::Return, Take());
    if (!Is(Tok::NewLine) && !Is(Tok::Semi) && !Is(Tok::End) && !Is(Tok::Dedent)) {
        auto value = Expression();
        if (!value) return nullptr;
        node->Add(value);
    }
    return node;
}

SigmaParser::AstNodePtr SigmaParser::SimpleStatement() {
    // name: Type = value
    if (Is(Tok::Name) && Peek(1).type == Tok::Colon) {
        const auto name = Take();
        if (!CheckName(name, "variable")) return nullptr;
        Take();  // :
        auto type = Type();
        if (!type) return nullptr;
        if (!Is(Tok::Assign)) return Error("a declaration needs an initial value: 'name: Type = value'");
        Take();
        auto value = Expression();
        if (!value) return nullptr;
        auto node = NewNode(Ast::Declaration, name);
        node->Add(type);
        node->Add(value);
        return node;
    }

    auto expr = Expression();
    if (!expr) return nullptr;

    switch (Peek().type) {
        case Tok::Assign:
        case Tok::PlusAssign:
        case Tok::MinusAssign:
        case Tok::MulAssign:
        case Tok::DivAssign:
        case Tok::ModAssign: {
            const auto op = Take();
            const bool isName = expr->GetType() == Ast::TokenType && expr->GetToken().type == Tok::Name;
            if (!isName && expr->GetType() != Ast::Index && expr->GetType() != Ast::Member)
                return Error(op, "the left side of an assignment must be a name, an index or a member");
            if (isName && !CheckName(expr->GetToken(), "variable")) return nullptr;
            if (!isName && op.type != Tok::Assign)
                return Error(op, std::format("'{}' needs a plain name on the left", op.Text()));
            auto value = Expression();
            if (!value) return nullptr;
            auto node = NewNode(Ast::Assignment, op);
            node->Add(expr);
            node->Add(value);
            return node;
        }
        default: {
            auto node = NewNode(Ast::ExprStatement);
            node->Add(expr);
            return node;
        }
    }
}

SigmaParser::AstNodePtr SigmaParser::Type() {
    if (Is(Tok::Fun)) {
        auto fun = NewNode(Ast::Type, Take());
        if (!Require(Tok::OpenParen, "'(' in a function type")) return nullptr;
        auto params = NewNode(Ast::None);
        if (!Is(Tok::CloseParen)) {
            do {
                auto p = Type();
                if (!p) return nullptr;
                params->Add(p);
            } while (Accept(Tok::Comma));
        }
        if (!Require(Tok::CloseParen, "')' in a function type")) return nullptr;
        fun->Add(params);
        if (Accept(Tok::Arrow)) {
            auto result = Type();
            if (!result) return nullptr;
            fun->Add(result);
        } else {
            fun->Add(NewNode(Ast::None));
        }
        return fun;
    }

    if (!Is(Tok::Name)) return Error("expected a type");
    auto type = NewNode(Ast::Type, Take());
    if (Accept(Tok::OpenSquare)) {
        do {
            auto arg = Type();
            if (!arg) return nullptr;
            type->Add(arg);
        } while (Accept(Tok::Comma));
        if (!Require(Tok::CloseSquare, "']' after the type arguments")) return nullptr;
    }
    if (Is(Tok::Question)) return Error("nullable types (T?) are not supported in Sigma yet");
    return type;
}

SigmaParser::AstNodePtr SigmaParser::Expression() {
    auto cond = Or();
    if (!cond || !Is(Tok::Question)) return cond;
    auto node = NewNode(Ast::Ternary, Take());
    auto then = Expression();
    if (!then || !Require(Tok::Colon, "':' in a conditional expression")) return nullptr;
    auto other = Expression();
    if (!other) return nullptr;
    node->Add(cond);
    node->Add(then);
    node->Add(other);
    return node;
}

SigmaParser::AstNodePtr SigmaParser::BinaryLevel(Level next, std::initializer_list<Tok::Enum> ops) {
    auto left = (this->*next)();
    while (left) {
        bool matched = false;
        for (auto op : ops) matched = matched || Is(op);
        if (!matched) break;
        auto node = NewNode(Ast::Binary, Take());
        auto right = (this->*next)();
        if (!right) return nullptr;
        node->Add(left);
        node->Add(right);
        left = node;
    }
    return left;
}

SigmaParser::AstNodePtr SigmaParser::Or() { return BinaryLevel(&SigmaParser::And, {Tok::Or}); }
SigmaParser::AstNodePtr SigmaParser::And() { return BinaryLevel(&SigmaParser::BitOr, {Tok::And}); }
SigmaParser::AstNodePtr SigmaParser::BitOr() { return BinaryLevel(&SigmaParser::BitXor, {Tok::BitOr}); }
SigmaParser::AstNodePtr SigmaParser::BitXor() { return BinaryLevel(&SigmaParser::BitAnd, {Tok::BitXor}); }
SigmaParser::AstNodePtr SigmaParser::BitAnd() { return BinaryLevel(&SigmaParser::Equality, {Tok::BitAnd}); }
SigmaParser::AstNodePtr SigmaParser::Equality() {
    return BinaryLevel(&SigmaParser::Relational, {Tok::Equiv, Tok::NotEquiv});
}
SigmaParser::AstNodePtr SigmaParser::Relational() {
    return BinaryLevel(&SigmaParser::Shift, {Tok::Less, Tok::Greater, Tok::LessEquiv, Tok::GreaterEquiv});
}
SigmaParser::AstNodePtr SigmaParser::Shift() {
    return BinaryLevel(&SigmaParser::Additive, {Tok::LeftShift, Tok::RightShift});
}
SigmaParser::AstNodePtr SigmaParser::Additive() { return BinaryLevel(&SigmaParser::Term, {Tok::Plus, Tok::Minus}); }
SigmaParser::AstNodePtr SigmaParser::Term() {
    return BinaryLevel(&SigmaParser::Unary, {Tok::Mul, Tok::Divide, Tok::Mod});
}

SigmaParser::AstNodePtr SigmaParser::Unary() {
    if (Is(Tok::Minus) || Is(Tok::Plus) || Is(Tok::Not) || Is(Tok::BitNot)) {
        auto node = NewNode(Ast::Unary, Take());
        auto operand = Unary();
        if (!operand) return nullptr;
        node->Add(operand);
        return node;
    }
    return Postfix();
}

SigmaParser::AstNodePtr SigmaParser::Postfix() {
    auto expr = Primary();
    if (!expr) return nullptr;

    const bool isName = expr->GetType() == Ast::TokenType && expr->GetToken().type == Tok::Name;
    if (!isName) {
        // Rho only accepts '.', '(' and '[' after a name; anything else would
        // be silently parsed as two expressions.
        if (Is(Tok::Dot) || Is(Tok::OpenParen) || Is(Tok::OpenSquare))
            return Error("'.', calls and indexing must follow a name in Sigma; assign the value to a variable first");
        return expr;
    }

    while (true) {
        if (Is(Tok::Dot)) {
            Take();
            if (!Is(Tok::Name)) return Error("expected a member name after '.'");
            auto member = NewNode(Ast::Member, Take());
            member->Add(expr);
            expr = member;
        } else if (Is(Tok::OpenParen)) {
            auto call = NewNode(Ast::Call, Take());
            auto args = NewNode(Ast::Args);
            if (!Is(Tok::CloseParen)) {
                do {
                    auto arg = Expansion();
                    if (!arg) return nullptr;
                    args->Add(arg);
                } while (Accept(Tok::Comma));
            }
            const TokenNode close = Peek();
            if (!Require(Tok::CloseParen, "')' after the arguments")) return nullptr;
            if (Is(Tok::Ellipsis))
                return Error("'...' (resume) is not supported in Sigma: in Rho it leaves for the top level without calling the function");
            call->Add(expr);
            call->Add(args);
            // Continuation operator, as in Rho: `f(x)&` (suspend, an ordinary
            // call written out) or `f(x)!` (replace: a tail call). It must be
            // written directly after the ')'. With a space between them,
            // `f(x) & mask` is a bitwise and, and `f(x) !` is not a call
            // operator at all.
            if ((Is(Tok::Not) || Is(Tok::BitAnd)) && Adjacent(close, Peek())) call->Add(NewNode(Take()));
            expr = call;
        } else if (Is(Tok::OpenSquare)) {
            auto index = NewNode(Ast::Index, Take());
            auto at = Expression();
            if (!at || !Require(Tok::CloseSquare, "']'")) return nullptr;
            index->Add(expr);
            index->Add(at);
            expr = index;
        } else {
            return expr;
        }
    }
}

bool SigmaParser::Adjacent(const TokenNode &a, const TokenNode &b) {
    return a.lineNumber == b.lineNumber && a.slice.End == b.slice.Start;
}

SigmaParser::AstNodePtr SigmaParser::Primary() {
    switch (Peek().type) {
        case Tok::Int:
        case Tok::Float:
        case Tok::String:
        case Tok::True:
        case Tok::False:
        case Tok::Name: return NewNode(Take());

        case Tok::PiBlock: return NewNode(Ast::PiBlock, Take());

        case Tok::OpenParen: {
            if (FoldAhead()) return Fold();
            Take();
            auto expr = Expression();
            if (!expr || !Require(Tok::CloseParen, "')'")) return nullptr;
            return expr;
        }

        case Tok::OpenSquare: return ListLiteral();
        case Tok::OpenBrace: return MapLiteral();

        case Tok::Fun: return Error("anonymous functions are not supported in Sigma yet");
        case Tok::Self: return Error("'self' is not supported in Sigma");
        default: return Error("expected an expression");
    }
}

// An argument or list element: an expression, or a pack expansion `xs...`.
SigmaParser::AstNodePtr SigmaParser::Expansion() {
    auto expr = Expression();
    if (!expr || !Is(Tok::Ellipsis)) return expr;
    auto expand = NewNode(Ast::Expand, Take());
    expand->Add(expr);
    return expand;
}

namespace {
bool IsFoldOperator(SigmaTokenEnumType::Enum t) {
    using T = SigmaTokenEnumType;
    switch (t) {
        case T::Plus:
        case T::Minus:
        case T::Mul:
        case T::Divide:
        case T::Mod:
        case T::And:
        case T::Or:
        case T::BitAnd:
        case T::BitOr:
        case T::BitXor: return true;
        default: return false;
    }
}
}  // namespace

// True if the parenthesised expression starting here is a fold: a '...' at
// its own bracket depth, before the matching ')'.
bool SigmaParser::FoldAhead() const {
    int depth = 0;
    for (size_t n = 0;; ++n) {
        const auto t = Peek(n).type;
        if (t == Tok::End) return false;
        if (t == Tok::OpenParen || t == Tok::OpenSquare || t == Tok::OpenBrace) ++depth;
        if (t == Tok::CloseParen || t == Tok::CloseSquare || t == Tok::CloseBrace) {
            if (--depth == 0) return false;
        }
        // `xs...` (an expansion) follows a name; a fold's '...' follows an operator or '('.
        if (t == Tok::Ellipsis && depth == 1 && Peek(n - 1).type != Tok::Name) return true;
    }
}

// Fold expressions, as in C++17:
//   (xs op ...)   (... op xs)   (xs op ... op init)   (init op ... op xs)
SigmaParser::AstNodePtr SigmaParser::Fold() {
    Take();  // (
    AstNodePtr left = NewNode(Ast::None);
    if (!Is(Tok::Ellipsis)) {
        left = Unary();
        if (!left) return nullptr;
    }
    auto opBefore = Peek();
    if (Is(Tok::Ellipsis)) {
        // (... op xs)
        Take();
        if (!IsFoldOperator(Peek().type)) return Error("expected an operator after '...' in a fold expression");
        auto fold = NewNode(Ast::Fold, Take());
        auto right = Unary();
        if (!right) return nullptr;
        fold->Add(left);
        fold->Add(right);
        if (!Require(Tok::CloseParen, "')' to close the fold expression")) return nullptr;
        return fold;
    }
    if (!IsFoldOperator(opBefore.type)) return Error("expected an operator before '...' in a fold expression");
    auto fold = NewNode(Ast::Fold, Take());
    if (!Require(Tok::Ellipsis, "'...' in a fold expression")) return nullptr;
    AstNodePtr right = NewNode(Ast::None);
    if (!Is(Tok::CloseParen)) {
        if (Peek().type != opBefore.type)
            return Error(Peek(), std::format("both operators of a fold expression must be '{}'", opBefore.Text()));
        Take();
        right = Unary();
        if (!right) return nullptr;
    }
    fold->Add(left);
    fold->Add(right);
    if (!Require(Tok::CloseParen, "')' to close the fold expression")) return nullptr;
    return fold;
}

SigmaParser::AstNodePtr SigmaParser::ListLiteral() {
    auto list = NewNode(Ast::List, Take());
    while (!Is(Tok::CloseSquare)) {
        auto element = Expansion();
        if (!element) return nullptr;
        list->Add(element);
        if (!Accept(Tok::Comma)) break;
    }
    if (!Require(Tok::CloseSquare, "',' or ']' in the list")) return nullptr;
    return list;
}

SigmaParser::AstNodePtr SigmaParser::MapLiteral() {
    auto map = NewNode(Ast::Map, Take());
    while (!Is(Tok::CloseBrace)) {
        if (!Is(Tok::String)) return Error("map keys must be string literals");
        auto entry = NewNode(Ast::MapEntry, Take());
        if (!Require(Tok::Colon, "':' after the map key")) return nullptr;
        auto value = Expression();
        if (!value) return nullptr;
        entry->Add(value);
        map->Add(entry);
        if (!Accept(Tok::Comma)) break;
    }
    if (!Require(Tok::CloseBrace, "',' or '}' in the map")) return nullptr;
    return map;
}

KAI_END
