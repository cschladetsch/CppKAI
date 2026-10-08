#include <KAI/Core/Object/ClassBase.h>
#include <KAI/Core/Object/MethodBase.h>
#include <KAI/Core/Object/PropertyBase.h>
#include <KAI/Core/Registry.h>
#include <KAI/Language/Sigma/SigmaChecker.h>

#include <format>

KAI_BEGIN

// ---------------------------------------------------------------------------
// SigmaType
// ---------------------------------------------------------------------------

SigmaTypePtr SigmaType::Of(Kind k) {
    auto t = std::make_shared<SigmaType>();
    t->kind = k;
    return t;
}

SigmaTypePtr SigmaType::ListOf(SigmaTypePtr element) {
    auto t = std::make_shared<SigmaType>();
    t->kind = Kind::List;
    t->args = {std::move(element)};
    return t;
}

SigmaTypePtr SigmaType::MapOf(SigmaTypePtr key, SigmaTypePtr value) {
    auto t = std::make_shared<SigmaType>();
    t->kind = Kind::Map;
    t->args = {std::move(key), std::move(value)};
    return t;
}

SigmaTypePtr SigmaType::FunOf(std::vector<SigmaTypePtr> params, SigmaTypePtr result) {
    auto t = std::make_shared<SigmaType>();
    t->kind = Kind::Fun;
    t->args = std::move(params);
    t->result = std::move(result);
    return t;
}

SigmaTypePtr SigmaType::NativeOf(int typeNumber, std::string name) {
    auto t = std::make_shared<SigmaType>();
    t->kind = Kind::Native;
    t->native = typeNumber;
    t->name = std::move(name);
    return t;
}

std::string SigmaType::ToString() const {
    switch (kind) {
        case Kind::Any: return "any";
        case Kind::Void: return "void";
        case Kind::Bool: return "bool";
        case Kind::Int: return "int";
        case Kind::Float: return "float";
        case Kind::Str: return "str";
        case Kind::List: return "List[" + args[0]->ToString() + "]";
        case Kind::Map: return "Map[" + args[0]->ToString() + ", " + args[1]->ToString() + "]";
        case Kind::Native: return name.empty() ? std::format("#{}", native) : name;
        case Kind::Fun: {
            std::string s = "fun(";
            for (size_t n = 0; n < args.size(); ++n) s += (n ? ", " : "") + args[n]->ToString();
            return s + ") -> " + result->ToString();
        }
    }
    return "?";
}

bool SigmaType::Same(const SigmaTypePtr &a, const SigmaTypePtr &b) {
    if (a->kind != b->kind || a->args.size() != b->args.size()) return false;
    if (a->Is(Kind::Native) && a->native != b->native) return false;
    for (size_t n = 0; n < a->args.size(); ++n)
        if (!Same(a->args[n], b->args[n])) return false;
    if (a->Is(Kind::Fun)) return Same(a->result, b->result);
    return true;
}

bool SigmaType::Assignable(const SigmaTypePtr &from, const SigmaTypePtr &to) {
    if (from->Is(Kind::Any) || to->Is(Kind::Any)) return !from->Is(Kind::Void) && !to->Is(Kind::Void);
    if (from->Is(Kind::Int) && to->Is(Kind::Float)) return true;
    return Same(from, to);
}

std::string SigmaDiagnostic::ToString() const { return std::format("{}:{}: {}", line + 1, column + 1, message); }

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

namespace {
using Kind = SigmaType::Kind;
using Tok = SigmaTokenEnumType;

// The type of an expression that already produced a diagnostic. It behaves
// like `any` so that one mistake does not cascade into many.
SigmaTypePtr Bad() {
    auto t = std::make_shared<SigmaType>();
    t->kind = Kind::Any;
    t->name = "!";
    return t;
}
bool IsBad(const SigmaTypePtr &t) { return t->Is(Kind::Any) && t->name == "!"; }
bool IsAny(const SigmaTypePtr &t) { return t->Is(Kind::Any); }

SigmaTypePtr T(Kind k) { return SigmaType::Of(k); }

bool IsName(const SigmaAstNodePtr &n) {
    return n->GetType() == SigmaAstNodeEnumType::TokenType && n->GetToken().type == Tok::Name;
}

Tok::Enum CompoundToBinary(Tok::Enum op) {
    switch (op) {
        case Tok::PlusAssign: return Tok::Plus;
        case Tok::MinusAssign: return Tok::Minus;
        case Tok::MulAssign: return Tok::Mul;
        case Tok::DivAssign: return Tok::Divide;
        case Tok::ModAssign: return Tok::Mod;
        default: return Tok::None;
    }
}
}  // namespace

void SigmaChecker::Report(const SigmaToken &at, const std::string &message) {
    // A string token's slice excludes its opening quote; point at the quote.
    const int column = at.type == Tok::String && at.slice.Start > 0 ? at.slice.Start - 1 : at.slice.Start;
    diagnostics_.push_back({at.lineNumber, column, message});
}

void SigmaChecker::Report(const NodePtr &at, const std::string &message) {
    // Use the first node in the subtree that carries a real token.
    std::vector<NodePtr> work{at};
    for (size_t n = 0; n < work.size(); ++n) {
        if (!work[n]) continue;
        if (work[n]->GetToken().lexer != nullptr) return Report(work[n]->GetToken(), message);
        for (auto const &ch : work[n]->GetChildren()) work.push_back(ch);
    }
    diagnostics_.push_back({0, 0, message});
}

SigmaChecker::Binding *SigmaChecker::Lookup(const std::string &name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return &found->second;
    }
    return nullptr;
}

void SigmaChecker::Bind(const std::string &name, SigmaTypePtr type, bool function) {
    scopes_.back()[name] = Binding{std::move(type), function};
}

bool SigmaChecker::Declarable(const NodePtr &at, const std::string &name, const SigmaTypePtr &type,
                              bool function) {
    Binding *b = Lookup(name);
    if (!b) return true;
    if (b->session && b->function == function && SigmaType::Same(b->type, type)) return true;
    if (b->session) {
        Report(at, std::format("'{}' was declared earlier in this session as {}", name, b->type->ToString()));
    } else {
        // Rho would assign to the existing binding rather than shadow it.
        Report(at, std::format("'{}' is already declared", name));
    }
    return false;
}

SigmaChecker::Globals SigmaChecker::GetGlobals() const {
    Globals globals;
    if (!scopes_.empty())
        for (auto const &[name, b] : scopes_.front()) globals[name] = Global{b.type, b.function};
    return globals;
}

SigmaTypePtr SigmaChecker::GetGlobalType(const std::string &name) const {
    if (scopes_.empty()) return nullptr;
    auto found = scopes_.front().find(name);
    return found == scopes_.front().end() ? nullptr : found->second.type;
}

bool SigmaChecker::Convert(const NodePtr &node, const SigmaTypePtr &from, const SigmaTypePtr &to,
                           const std::string &context) {
    if (IsBad(from) || IsBad(to)) return true;
    if (!SigmaType::Assignable(from, to)) {
        Report(node, std::format("{}: expected {}, got {}", context, to->ToString(), from->ToString()));
        return false;
    }
    if (from->Is(Kind::Int) && to->Is(Kind::Float)) widened_.insert(node.get());
    return true;
}

// ---------------------------------------------------------------------------
// types
// ---------------------------------------------------------------------------

SigmaTypePtr SigmaChecker::FromTypeNumber(Type::Number n) const
{
    using N = Type::Number;
    switch (n) {
        case N::Void:
            return T(Kind::Void);
        case N::Bool:
            return T(Kind::Bool);
        case N::Signed32:
            return T(Kind::Int); // other int widths: intentionally native?
        case N::Single:
        case N::Double:
            return T(Kind::Float);
        case N::String:
            return T(Kind::Str);
        case N::Object:
            return T(Kind::Any);
        case N::None:
            return T(Kind::Void); // or a dedicated None kind
        case N::Undefined:
            return T(Kind::Unknown);
        default:
            break; // user/native classes
    }

    std::string name;
    if (reg_) {
        if (auto klass = reg_->FindClass(n))
            name = klass->GetName().ToString().StdString();
    }
    if (name.empty())
        name = "<native #" + std::to_string(static_cast<int>(n)) + ">";
    return SigmaType::NativeOf(static_cast<int>(n), std::move(name));
}
SigmaTypePtr SigmaChecker::Resolve(const NodePtr& node, bool allowVoid)
{
    if (!node || node->GetType() == Ast::None) return T(Kind::Void);
    const auto &tok = node->GetToken();
    const auto &args = node->GetChildren();

    if (tok.type == Tok::Fun) {
        std::vector<SigmaTypePtr> params;
        for (auto const &p : args[0]->GetChildren()) params.push_back(Resolve(p));
        return SigmaType::FunOf(std::move(params), Resolve(args[1], true));
    }

    const std::string name = tok.Text();
    auto arity = [&](size_t n) {
        if (args.size() == n) return true;
        Report(node, std::format("'{}' takes {} type argument{}", name, n, n == 1 ? "" : "s"));
        return false;
    };

    if (name == "void") {
        if (!allowVoid) Report(node, "'void' is only allowed as a function result");
        return T(Kind::Void);
    }
    if (name == "any") return T(Kind::Any);
    if (name == "bool") return T(Kind::Bool);
    if (name == "int") return T(Kind::Int);
    if (name == "float") return T(Kind::Float);
    if (name == "str" || name == "string") return T(Kind::Str);
    if (name == "List") return arity(1) ? SigmaType::ListOf(Resolve(args[0])) : Bad();
    if (name == "Map") {
        if (!arity(2)) return Bad();
        auto key = Resolve(args[0]);
        if (!key->Is(Kind::Str) && !IsBad(key)) Report(args[0], "map keys must be str");
        return SigmaType::MapOf(T(Kind::Str), Resolve(args[1]));
    }

    if (reg_ != nullptr) {
        if (auto klass = reg_->GetClass(Label(name.c_str()))) {
            if (!args.empty()) Report(node, std::format("'{}' is not generic", name));
            return FromTypeNumber(klass->GetTypeNumber().ToInt());
        }
    }

    Report(node, std::format("unknown type '{}'", name));
    return Bad();
}

SigmaTypePtr SigmaChecker::Signature(const NodePtr &fun) {
    std::vector<SigmaTypePtr> params;
    for (auto const &p : fun->GetChild(0)->GetChildren()) params.push_back(Resolve(p->GetChild(0)));
    return SigmaType::FunOf(std::move(params), Resolve(fun->GetChild(1), true));
}

// ---------------------------------------------------------------------------
// statements
// ---------------------------------------------------------------------------

bool SigmaChecker::Check(const NodePtr &program) {
    diagnostics_.clear();
    widened_.clear();
    functions_.clear();
    scopes_.assign(1, Scope{});
    for (auto const &[name, g] : session_) scopes_.front()[name] = Binding{g.type, g.function, true};
    loops_ = 0;
    function_ = nullptr;

    Statements(program, true);

    // Function bodies are checked after the top level, so they may use any
    // global the program declares (as Rho resolves names when called).
    for (auto const &fun : functions_) FunctionBody(fun);

    return diagnostics_.empty();
}

void SigmaChecker::Statements(const NodePtr &block, bool topLevel) {
    if (topLevel) {
        // Functions are visible from the start of the program, so calls may
        // precede definitions and functions may be mutually recursive.
        for (auto const &st : block->GetChildren()) {
            if (st->GetType() != Ast::Function) continue;
            const std::string name = st->GetToken().Text();
            auto sig = Signature(st);
            if (!Declarable(st, name, sig, true)) continue;
            Bind(name, sig, true);
            functions_.push_back(st);
        }
    }
    for (auto const &st : block->GetChildren()) Statement(st, topLevel);
}

void SigmaChecker::Condition(const NodePtr &node, const char *what) {
    auto t = Expr(node, T(Kind::Bool));
    if (!t->Is(Kind::Bool) && !IsAny(t)) Report(node, std::format("{} must be bool, got {}", what, t->ToString()));
}

void SigmaChecker::Statement(const NodePtr &node, bool topLevel) {
    const auto &ch = node->GetChildren();
    switch (node->GetType()) {
        case Ast::Function:
            if (!topLevel) Report(node, "functions must be defined at the top level");
            return;

        case Ast::Declaration: return Declaration(node);
        case Ast::Assignment: return Assignment(node);
        case Ast::ForEach: return ForEach(node);
        case Ast::Return: return Return(node);
        case Ast::ExprStatement: Expr(ch[0]); return;
        case Ast::Assert: Condition(ch[0], "an assert condition"); return;

        case Ast::If:
            Condition(ch[0], "an if condition");
            Statements(ch[1], false);
            if (ch.size() > 2) Statement(ch[2], false);
            return;
        case Ast::Block: Statements(node, false); return;

        case Ast::While:
            Condition(ch[0], "a while condition");
            ++loops_;
            Statements(ch[1], false);
            --loops_;
            return;
        case Ast::DoWhile:
            ++loops_;
            Statements(ch[0], false);
            --loops_;
            Condition(ch[1], "a do-while condition");
            return;
        case Ast::For:
            if (ch[0]->GetType() != Ast::None) Statement(ch[0], false);
            if (ch[1]->GetType() != Ast::None) Condition(ch[1], "a for condition");
            ++loops_;
            Statements(ch[3], false);
            --loops_;
            if (ch[2]->GetType() != Ast::None) Statement(ch[2], false);
            return;

        case Ast::Break:
        case Ast::Continue:
            if (loops_ == 0)
                Report(node, std::format("'{}' outside a loop", node->GetType() == Ast::Break ? "break" : "continue"));
            return;

        default: Report(node, "unsupported statement"); return;
    }
}

void SigmaChecker::Declaration(const NodePtr &node) {
    const std::string name = node->GetToken().Text();
    auto type = Resolve(node->GetChild(0));
    auto value = Expr(node->GetChild(1), type);
    Convert(node->GetChild(1), value, type, std::format("cannot initialise '{}'", name));
    if (Declarable(node, name, type, false)) Bind(name, type);
}

void SigmaChecker::Assignment(const NodePtr &node) {
    const auto op = node->GetToken().type;
    const auto &target = node->GetChild(0);
    const auto &valueNode = node->GetChild(1);

    if (IsName(target)) {
        const std::string name = target->GetToken().Text();
        Binding *b = Lookup(name);

        if (op != Tok::Assign) {
            if (!b) {
                Report(target, std::format("'{}' is not declared", name));
                Expr(valueNode);
                return;
            }
            auto value = Expr(valueNode);
            auto result = BinaryResult(node, CompoundToBinary(op), b->type, value);
            // The whole `x = x op v` is what gets stored; widen it if needed.
            if (!IsBad(result) && !SigmaType::Assignable(result, b->type)) {
                Report(node, std::format("cannot assign {} to '{}' of type {}", result->ToString(), name,
                                         b->type->ToString()));
            } else if (result->Is(Kind::Int) && b->type->Is(Kind::Float)) {
                widened_.insert(node.get());
            }
            return;
        }

        if (b) {
            if (b->function) Report(target, std::format("cannot assign to function '{}'", name));
            auto value = Expr(valueNode, b->type);
            Convert(valueNode, value, b->type, std::format("cannot assign to '{}'", name));
            return;
        }

        // First assignment declares the variable with the value's type.
        auto value = Expr(valueNode);
        if (value->Is(Kind::Void)) {
            Report(valueNode, std::format("cannot initialise '{}' with a void value", name));
            value = Bad();
        } else if (IsAny(value) && !IsBad(value)) {
            Report(valueNode,
                   std::format("cannot infer a type for '{}' from 'any'; declare it: '{}: T = ...'", name, name));
            value = Bad();
        }
        Bind(name, value);
        return;
    }

    // container[index] = value, object.member = value
    auto slot = target->GetType() == Ast::Index ? Index(target) : Member(target);
    auto value = Expr(valueNode, slot);
    Convert(valueNode, value, slot, "cannot store value");
}

void SigmaChecker::ForEach(const NodePtr &node) {
    const std::string name = node->GetToken().Text();
    auto iterable = Expr(node->GetChild(0));
    SigmaTypePtr element = Bad();
    if (iterable->Is(Kind::List)) {
        element = iterable->args[0];
    } else if (IsAny(iterable)) {
        element = iterable;
    } else {
        Report(node->GetChild(0), std::format("can only iterate over a List, not {}", iterable->ToString()));
    }

    if (Binding *b = Lookup(name)) {
        // Rho's loop variable is an ordinary assignment in the enclosing scope.
        if (!IsBad(element) && !SigmaType::Same(element, b->type) && !IsAny(b->type))
            Report(node, std::format("loop variable '{}' is {}, but the elements are {}", name, b->type->ToString(),
                                     element->ToString()));
    } else {
        Bind(name, element);
    }

    ++loops_;
    Statements(node->GetChild(1), false);
    --loops_;
}

void SigmaChecker::Return(const NodePtr &node) {
    if (!function_) {
        Report(node, "'return' outside a function");
        return;
    }
    const std::string fname = function_->GetToken().Text();
    if (node->GetChildren().empty()) {
        if (!result_->Is(Kind::Void))
            Report(node, std::format("'{}' must return {}", fname, result_->ToString()));
        return;
    }
    if (result_->Is(Kind::Void)) {
        Report(node, std::format("'{}' does not return a value; declare '-> T' to return one", fname));
        Expr(node->GetChild(0));
        return;
    }
    auto value = Expr(node->GetChild(0), result_);
    Convert(node->GetChild(0), value, result_, std::format("'{}' returns {}", fname, result_->ToString()));
}

bool SigmaChecker::AlwaysReturns(const NodePtr &node) {
    switch (node->GetType()) {
        case Ast::Return: return true;
        case Ast::Block:
            for (auto const &st : node->GetChildren())
                if (AlwaysReturns(st)) return true;
            return false;
        case Ast::If:
            return node->GetChildren().size() == 3 && AlwaysReturns(node->GetChild(1)) &&
                   AlwaysReturns(node->GetChild(2));
        default: return false;
    }
}

void SigmaChecker::FunctionBody(const NodePtr &fun) {
    // Copy the signature: pushing a scope below may reallocate scopes_, and
    // a pointer into the old global scope would then dangle (MSVC copies
    // unordered_maps when a vector grows, because their move isn't noexcept).
    const Binding *found = Lookup(fun->GetToken().Text());
    if (!found || !found->type->Is(Kind::Fun)) return;
    const SigmaTypePtr sig = found->type;

    function_ = fun.get();
    result_ = sig->result;
    scopes_.emplace_back();

    const auto &params = fun->GetChild(0)->GetChildren();
    for (size_t n = 0; n < params.size(); ++n) {
        const std::string name = params[n]->GetToken().Text();
        if (scopes_.back().contains(name)) Report(params[n], std::format("duplicate parameter '{}'", name));
        Bind(name, sig->args[n]);
    }

    const auto &body = fun->GetChild(2);
    tailCall_ = TailCallIn(body);
    Statements(body, false);
    if (!result_->Is(Kind::Void) && !AlwaysReturns(body))
        Report(fun, std::format("'{}' does not return a value on every path", fun->GetToken().Text()));

    scopes_.pop_back();
    function_ = nullptr;
    result_ = nullptr;
    tailCall_ = nullptr;
}

// The call carrying '&' or '!' (Rho's suspend and replace), or None.
static SigmaTokenEnumType::Enum Continuation(const SigmaChecker::NodePtr &call) {
    if (call->GetType() != SigmaAstNodeEnumType::Call || call->GetChildren().size() < 3)
        return SigmaTokenEnumType::None;
    return call->GetChild(2)->GetToken().type;
}

// `f(...)!` replaces the running function with f, so the function never
// resumes. Rho only does that correctly when the call is the last statement
// of the function body itself (inside an if or a loop it corrupts the stack
// or hangs), so that is the only place Sigma allows it:
//     return f(...)!      or, in a void function,      f(...)!
const SigmaAstNode *SigmaChecker::TailCallIn(const NodePtr &body) {
    const auto &statements = body->GetChildren();
    if (statements.empty()) return nullptr;
    const auto &last = statements.back();
    if ((last->GetType() == Ast::Return || last->GetType() == Ast::ExprStatement) && !last->GetChildren().empty()) {
        const auto &call = last->GetChild(0);
        if (Continuation(call) == Tok::Not) return call.get();
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// expressions
// ---------------------------------------------------------------------------

SigmaTypePtr SigmaChecker::Expr(const NodePtr &node, const SigmaTypePtr &expected) {
    switch (node->GetType()) {
        case Ast::TokenType:
            switch (node->GetToken().type) {
                case Tok::Int: return T(Kind::Int);
                case Tok::Float: return T(Kind::Float);
                case Tok::String: return T(Kind::Str);
                case Tok::True:
                case Tok::False: return T(Kind::Bool);
                case Tok::Name: return Name(node);
                default: break;
            }
            break;
        case Ast::PiBlock: return T(Kind::Any);
        case Ast::Binary: return Binary(node);
        case Ast::Unary: return Unary(node);
        case Ast::Ternary: return Ternary(node, expected);
        case Ast::Call: return Call(node);
        case Ast::Member: return Member(node);
        case Ast::Index: return Index(node);
        case Ast::List: return List(node, expected);
        case Ast::Map: return Map(node, expected);
        default: break;
    }
    Report(node, "unsupported expression");
    return Bad();
}

SigmaTypePtr SigmaChecker::Name(const NodePtr &node) {
    const std::string name = node->GetToken().Text();
    if (auto b = Lookup(name)) return b->type;
    Report(node, std::format("undefined name '{}'", name));
    return Bad();
}

SigmaTypePtr SigmaChecker::BinaryResult(const NodePtr &at, Tok::Enum op, const SigmaTypePtr &a,
                                        const SigmaTypePtr &b) {
    if (IsBad(a) || IsBad(b)) return Bad();
    const bool any = IsAny(a) || IsAny(b);
    auto numeric = [&] { return a->Is(Kind::Int) && b->Is(Kind::Int) ? T(Kind::Int) : T(Kind::Float); };
    auto reject = [&](SigmaTypePtr r) {
        if (any) return r;
        Report(at, std::format("operator '{}' cannot be applied to {} and {}", at->GetToken().Text(),
                               a->ToString(), b->ToString()));
        return Bad();
    };

    switch (op) {
        case Tok::Plus:
            if (a->IsNumeric() && b->IsNumeric()) return numeric();
            if (a->Is(Kind::Str) && b->Is(Kind::Str)) return T(Kind::Str);
            if (a->Is(Kind::List) && SigmaType::Same(a, b)) return a;
            return reject(T(Kind::Any));
        case Tok::Minus:
        case Tok::Mul:
        case Tok::Divide:
        case Tok::Mod:
            if (a->IsNumeric() && b->IsNumeric()) return numeric();
            return reject(T(Kind::Any));
        case Tok::Less:
        case Tok::Greater:
        case Tok::LessEquiv:
        case Tok::GreaterEquiv:
            if ((a->IsNumeric() && b->IsNumeric()) || (a->Is(Kind::Str) && b->Is(Kind::Str))) return T(Kind::Bool);
            return reject(T(Kind::Bool));
        case Tok::Equiv:
        case Tok::NotEquiv:
            if ((a->IsNumeric() && b->IsNumeric()) || SigmaType::Same(a, b)) return T(Kind::Bool);
            return reject(T(Kind::Bool));
        case Tok::And:
        case Tok::Or:
            if (a->Is(Kind::Bool) && b->Is(Kind::Bool)) return T(Kind::Bool);
            return reject(T(Kind::Bool));
        case Tok::BitAnd:
        case Tok::BitOr:
        case Tok::BitXor:
        case Tok::LeftShift:
        case Tok::RightShift:
            if (a->Is(Kind::Int) && b->Is(Kind::Int)) return T(Kind::Int);
            return reject(T(Kind::Int));
        default: return reject(T(Kind::Any));
    }
}

SigmaTypePtr SigmaChecker::Binary(const NodePtr &node) {
    auto a = Expr(node->GetChild(0));
    auto b = Expr(node->GetChild(1));
    return BinaryResult(node, node->GetToken().type, a, b);
}

SigmaTypePtr SigmaChecker::Unary(const NodePtr &node) {
    const auto op = node->GetToken().type;
    auto t = Expr(node->GetChild(0));
    if (IsAny(t)) return op == Tok::Not ? T(Kind::Bool) : t;

    const bool ok = (op == Tok::Not && t->Is(Kind::Bool)) || (op == Tok::BitNot && t->Is(Kind::Int)) ||
                    ((op == Tok::Minus || op == Tok::Plus) && t->IsNumeric());
    if (ok) return t;
    Report(node, std::format("operator '{}' cannot be applied to {}", node->GetToken().Text(), t->ToString()));
    return Bad();
}

// The common type of several expressions: identical types, or int and float
// mixed (the ints are widened). Anything else is an error.
SigmaTypePtr SigmaChecker::Unify(const std::vector<NodePtr> &nodes, const std::vector<SigmaTypePtr> &types,
                                 const NodePtr &at, const char *what) {
    SigmaTypePtr common;
    for (auto const &t : types) {
        if (IsBad(t)) return Bad();
        if (!common) {
            common = t;
        } else if (common->IsNumeric() && t->IsNumeric()) {
            if (t->Is(Kind::Float)) common = t;
        } else if (!SigmaType::Same(common, t)) {
            Report(at, std::format("{} have different types: {} and {}", what, common->ToString(), t->ToString()));
            return Bad();
        }
    }
    if (common && common->Is(Kind::Float))
        for (size_t n = 0; n < nodes.size(); ++n)
            if (types[n]->Is(Kind::Int)) widened_.insert(nodes[n].get());
    return common;
}

SigmaTypePtr SigmaChecker::Ternary(const NodePtr &node, const SigmaTypePtr &expected) {
    Condition(node->GetChild(0), "a conditional expression's condition");
    const auto &a = node->GetChild(1);
    const auto &b = node->GetChild(2);
    if (expected && !IsAny(expected)) {
        Convert(a, Expr(a, expected), expected, "conditional branch");
        Convert(b, Expr(b, expected), expected, "conditional branch");
        return expected;
    }
    return Unify({a, b}, {Expr(a), Expr(b)}, node, "the branches of a conditional expression");
}

SigmaTypePtr SigmaChecker::List(const NodePtr &node, const SigmaTypePtr &expected) {
    const auto &elements = node->GetChildren();
    if (expected && expected->Is(Kind::List)) {
        for (auto const &e : elements) Convert(e, Expr(e, expected->args[0]), expected->args[0], "list element");
        return expected;
    }
    if (elements.empty()) {
        Report(node, "cannot infer the element type of an empty list; declare it, e.g. 'xs: List[int] = []'");
        return Bad();
    }
    std::vector<SigmaTypePtr> types;
    for (auto const &e : elements) types.push_back(Expr(e));
    auto element = Unify(elements, types, node, "list elements");
    return IsBad(element) ? element : SigmaType::ListOf(element);
}

SigmaTypePtr SigmaChecker::Map(const NodePtr &node, const SigmaTypePtr &expected) {
    const auto &entries = node->GetChildren();
    if (expected && expected->Is(Kind::Map)) {
        for (auto const &e : entries)
            Convert(e->GetChild(0), Expr(e->GetChild(0), expected->args[1]), expected->args[1], "map value");
        return expected;
    }
    if (entries.empty()) {
        Report(node, "cannot infer the value type of an empty map; declare it, e.g. 'm: Map[str, int] = {}'");
        return Bad();
    }
    std::vector<NodePtr> values;
    std::vector<SigmaTypePtr> types;
    for (auto const &e : entries) {
        values.push_back(e->GetChild(0));
        types.push_back(Expr(e->GetChild(0)));
    }
    auto value = Unify(values, types, node, "map values");
    return IsBad(value) ? value : SigmaType::MapOf(T(Kind::Str), value);
}

SigmaTypePtr SigmaChecker::Index(const NodePtr &node) {
    auto container = Expr(node->GetChild(0));
    const auto &at = node->GetChild(1);
    if (container->Is(Kind::List)) {
        Convert(at, Expr(at, T(Kind::Int)), T(Kind::Int), "list index");
        return container->args[0];
    }
    if (container->Is(Kind::Map)) {
        Convert(at, Expr(at, T(Kind::Str)), T(Kind::Str), "map key");
        return container->args[1];
    }
    Expr(at);
    if (IsAny(container)) return container;
    Report(node, std::format("{} cannot be indexed", container->ToString()));
    return Bad();
}

SigmaTypePtr SigmaChecker::Member(const NodePtr &node) {
    auto object = Expr(node->GetChild(0));
    const std::string name = node->GetToken().Text();
    if (IsAny(object)) return object;

    if (object->Is(Kind::Native) && reg_ != nullptr) {
        const ClassBase *klass = nullptr;
        try {
            klass = reg_->GetClass(Type::Number(object->native));
        } catch (...) {
        }
        if (klass && klass->HasProperty(Label(name.c_str())))
            return FromTypeNumber(klass->GetProperty(Label(name.c_str())).GetFieldTypeNumber().ToInt());
        if (klass && klass->GetMethod(Label(name.c_str()))) {
            Report(node, std::format("'{}' is a method of {}; call it", name, object->ToString()));
            return Bad();
        }
    }
    Report(node, std::format("{} has no member '{}'", object->ToString(), name));
    return Bad();
}

SigmaTypePtr SigmaChecker::Arguments(const NodePtr &at, const std::string &what,
                                     const std::vector<SigmaTypePtr> &params, const NodePtr &args) {
    const auto &given = args->GetChildren();
    if (given.size() != params.size()) {
        Report(at, std::format("{} expects {} argument{}, got {}", what, params.size(), params.size() == 1 ? "" : "s",
                               given.size()));
    }
    for (size_t n = 0; n < given.size(); ++n) {
        if (n >= params.size()) {
            Expr(given[n]);
            continue;
        }
        Convert(given[n], Expr(given[n], params[n]), params[n], std::format("argument {} of {}", n + 1, what));
    }
    return nullptr;
}

SigmaTypePtr SigmaChecker::Call(const NodePtr &node) {
    const auto &callee = node->GetChild(0);
    const auto &args = node->GetChild(1);
    const auto control = Continuation(node);

    if (control != Tok::None) {
        auto type = PlainCall(node);
        if (callee->GetType() == Ast::Member || (IsName(callee) && callee->GetToken().Text() == "print" && !Lookup("print"))) {
            Report(node->GetChild(2), std::format("'{}' applies to calls of Sigma functions, not built-ins or methods",
                                                  node->GetChild(2)->GetToken().Text()));
            return type;
        }
        if (control == Tok::Not) TailCall(node, type);
        return type;
    }
    return PlainCall(node);
}

void SigmaChecker::TailCall(const NodePtr &node, const SigmaTypePtr &type) {
    const auto &op = node->GetChild(2);
    if (!function_) {
        Report(op, "'!' (tail call) can only be used inside a function");
        return;
    }
    if (node.get() != tailCall_) {
        Report(op, "'!' (tail call) must be the last statement of a function body: 'return f(...)!', or 'f(...)!' in a void function");
        return;
    }
    if (IsBad(type) || IsAny(type)) return;
    const std::string fname = function_->GetToken().Text();
    if (result_->Is(Kind::Void)) {
        if (!type->Is(Kind::Void))
            Report(node, std::format("tail call in void '{}' must call a void function, got {}", fname, type->ToString()));
        return;
    }
    // No int-to-float widening: the callee's result becomes this function's
    // result directly, with no code of ours left to run.
    if (!SigmaType::Same(type, result_))
        Report(node, std::format("tail call: '{}' returns {}, so the called function must return exactly {}, got {}",
                                 fname, result_->ToString(), result_->ToString(), type->ToString()));
}

SigmaTypePtr SigmaChecker::PlainCall(const NodePtr &node) {
    const auto &callee = node->GetChild(0);
    const auto &args = node->GetChild(1);

    if (callee->GetType() == Ast::Member) return MethodCall(node, callee, args);

    // print(...) is built into Rho and accepts anything.
    if (IsName(callee) && callee->GetToken().Text() == "print" && !Lookup("print")) {
        for (auto const &a : args->GetChildren()) {
            if (Expr(a)->Is(Kind::Void)) Report(a, "cannot print a void value");
        }
        return T(Kind::Void);
    }

    auto fun = Expr(callee);
    if (IsAny(fun)) {
        for (auto const &a : args->GetChildren()) Expr(a);
        return fun;
    }
    const std::string what = IsName(callee) ? std::format("'{}'", callee->GetToken().Text()) : "the call";
    if (!fun->Is(Kind::Fun)) {
        Report(node, std::format("{} of type {} is not a function", what, fun->ToString()));
        for (auto const &a : args->GetChildren()) Expr(a);
        return Bad();
    }
    Arguments(node, what, fun->args, args);
    return fun->result;
}

SigmaTypePtr SigmaChecker::MethodCall(const NodePtr &call, const NodePtr &member, const NodePtr &args) {
    auto receiver = Expr(member->GetChild(0));
    const std::string name = member->GetToken().Text();
    const std::string what = std::format("'{}'", name);

    if (IsAny(receiver)) {
        for (auto const &a : args->GetChildren()) Expr(a);
        return receiver;
    }

    // Container methods that Rho lowers to Pi operations.
    if (receiver->Is(Kind::List)) {
        const auto &element = receiver->args[0];
        if (name == "size") return Arguments(call, what, {}, args), T(Kind::Int);
        if (name == "push") return Arguments(call, what, {element}, args), T(Kind::Void);
        if (name == "slice") return Arguments(call, what, {T(Kind::Int), T(Kind::Int)}, args), receiver;
    } else if (receiver->Is(Kind::Map)) {
        if (name == "size") return Arguments(call, what, {}, args), T(Kind::Int);
        if (name == "keys") return Arguments(call, what, {}, args), SigmaType::ListOf(receiver->args[0]);
    } else if (receiver->Is(Kind::Str)) {
        if (name == "size") return Arguments(call, what, {}, args), T(Kind::Int);
    } else if (receiver->Is(Kind::Native) && reg_ != nullptr) {
        // Reflected C++ methods, using the signatures ClassBuilder records.
        const ClassBase *klass = nullptr;
        try {
            klass = reg_->GetClass(Type::Number(receiver->native));
        } catch (...) {
        }
        if (auto method = klass ? klass->GetMethod(Label(name.c_str())) : nullptr) {
            std::vector<SigmaTypePtr> params;
            for (auto const &p : method->GetArgumentTypes()) params.push_back(FromTypeNumber(p.ToInt()));
            Arguments(call, std::format("'{}.{}'", receiver->ToString(), name), params, args);
            return FromTypeNumber(method->GetReturnType().ToInt());
        }
    }

    if (!IsBad(receiver)) Report(member, std::format("{} has no method '{}'", receiver->ToString(), name));
    for (auto const &a : args->GetChildren()) Expr(a);
    return Bad();
}

KAI_END
