#include <KAI/Language/Rho/RhoTranslator.h>
#include <KAI/Language/Sigma/SigmaTranslator.h>

#include <algorithm>
#include <optional>
#include <utility>

KAI_BEGIN

namespace {

using Ast = SigmaAstNodeEnumType;
using Tok = SigmaTokenEnumType;
using NodePtr = SigmaAstNodePtr;

/// Lowers a checked Sigma AST to Rho source.
///
/// Every operator expression is parenthesised, so the result does not depend
/// on Rho's precedence rules (Rho parses `-1 + 5` as `-1`, for example), and
/// compound assignments are expanded because Rho's `+=` family is not
/// implemented at runtime.
//
/// A variadic template becomes one Rho function per pack length, `name__N`,
/// whose pack parameter `xs` is spelled out as `xs__0, ..., xs__N-1`.
/// Expansions, folds and `xs.size()` are written out for that length, and a
/// condition on a pack's length keeps only the branch it selects.
class RhoEmitter {
   public:
    explicit RhoEmitter(const SigmaChecker &checker)
        : checker_(checker), widened_(&checker.GetWidened()), variadics_(checker.GetVariadics()) {}

    std::string Program(const NodePtr &root) {
        std::string out;
        // Instances of variadic templates defined by earlier session inputs.
        for (auto const &inst : checker_.GetVariadicInstances()) {
            const auto &top = root->GetChildren();
            if (std::find(top.begin(), top.end(), inst.fun) == top.end()) Instance(out, inst, 0);
        }
        for (auto const &st : root->GetChildren()) Statement(out, st, 0);
        return out;
    }

   private:
    const SigmaChecker &checker_;
    const std::unordered_set<const SigmaAstNode *> *widened_;
    std::unordered_map<std::string, int> variadics_;  // name -> parameters before the pack
    std::unordered_map<std::string, int> packs_;      // pack parameter -> its length, in an instance

    bool Widened(const NodePtr &node) const { return widened_->contains(node.get()); }

    static std::string Mangle(const std::string &name, int arity) { return name + "__" + std::to_string(arity); }

    static bool IsVariadic(const NodePtr &fun) {
        const auto &ch = fun->GetChildren();
        return ch.size() > 3 && !ch[3]->GetChildren().empty() && ch[3]->GetChildren().back()->GetType() == Ast::Expand;
    }

    void Instance(std::string &out, const SigmaChecker::VariadicInstance &inst, int depth) {
        const auto &ch = inst.fun->GetChildren();
        std::string params;
        const auto &ps = ch[0]->GetChildren();
        for (size_t n = 0; n < ps.size(); ++n) {
            if (n + 1 == ps.size()) {
                for (int e = 0; e < inst.arity; ++e) params += (params.empty() ? "" : ", ") + Mangle(ps[n]->Text(), e);
                packs_[ps[n]->Text()] = inst.arity;
            } else {
                params += (params.empty() ? "" : ", ") + ps[n]->Text();
            }
        }
        auto outer = std::exchange(widened_, &inst.widened);
        out += Indent(depth) + "fun " + Mangle(inst.fun->Text(), inst.arity) + "(" + params + ")\n";
        Block(out, ch[2], depth + 1);
        widened_ = outer;
        packs_.clear();
    }

    std::optional<bool> Static(const NodePtr &condition) const {
        return SigmaChecker::StaticCondition(condition, [this](const std::string &name) -> std::optional<int> {
            auto found = packs_.find(name);
            return found == packs_.end() ? std::nullopt : std::optional<int>(found->second);
        });
    }

    // Arguments or list elements, with `xs...` spelled out.
    std::string Items(const std::vector<NodePtr> &nodes, int *count = nullptr) {
        std::string items;
        int n = 0;
        for (auto const &a : nodes) {
            if (a->GetType() == Ast::Expand) {
                const std::string pack = a->GetChild(0)->Text();
                const int length = packs_.at(pack);
                for (int e = 0; e < length; ++e, ++n) items += (items.empty() ? "" : ", ") + Mangle(pack, e);
            } else {
                items += (items.empty() ? "" : ", ") + Expr(a);
                ++n;
            }
        }
        if (count) *count = n;
        return items;
    }

    // (xs op ...) is x0 op (x1 op (... op init)); (... op xs) is ((init op x0) op x1) ...
    std::string FoldOut(const NodePtr &node) {
        const auto &left = node->GetChild(0);
        const auto &right = node->GetChild(1);
        const bool fromRight = left->GetType() != Ast::None && packs_.contains(left->Text()) &&
                               left->GetType() == Ast::TokenType;
        const NodePtr &packNode = fromRight ? left : right;
        const NodePtr &init = fromRight ? right : left;
        const std::string pack = packNode->Text();
        const int length = packs_.at(pack);
        const std::string op = node->Text();
        std::vector<std::string> terms;
        for (int e = 0; e < length; ++e) terms.push_back(Mangle(pack, e));
        if (init->GetType() != Ast::None) {
            if (fromRight)
                terms.push_back(Expr(init));
            else
                terms.insert(terms.begin(), Expr(init));
        }
        if (terms.empty()) return node->GetToken().type == Tok::And ? "true" : "false";
        if (fromRight) {
            std::string acc = terms.back();
            for (size_t n = terms.size() - 1; n-- > 0;) acc = "(" + terms[n] + " " + op + " " + acc + ")";
            return acc;
        }
        std::string acc = terms.front();
        for (size_t n = 1; n < terms.size(); ++n) acc = "(" + acc + " " + op + " " + terms[n] + ")";
        return acc;
    }

    static std::string Indent(int depth) { return std::string(static_cast<size_t>(depth) * 4, ' '); }

    void Block(std::string &out, const NodePtr &block, int depth) {
        for (auto const &st : block->GetChildren()) Statement(out, st, depth);
    }

    void If(std::string &out, const NodePtr &node, int depth, const std::string &prefix) {
        out += Indent(depth) + prefix + "if " + Expr(node->GetChild(0)) + "\n";
        Block(out, node->GetChild(1), depth + 1);
        if (node->GetChildren().size() < 3) return;
        const auto &other = node->GetChild(2);
        if (other->GetType() == Ast::If && Static(other->GetChild(0))) {
            out += Indent(depth) + "else\n";
            Statement(out, other, depth + 1);
        } else if (other->GetType() == Ast::If) {
            If(out, other, depth, "else ");
        } else {
            out += Indent(depth) + "else\n";
            Block(out, other, depth + 1);
        }
    }

    void Statement(std::string &out, const NodePtr &node, int depth) {
        const auto &ch = node->GetChildren();
        switch (node->GetType()) {
            case Ast::Function: {
                if (IsVariadic(node)) {
                    for (auto const &inst : checker_.GetVariadicInstances())
                        if (inst.fun == node) Instance(out, inst, depth);
                    return;
                }
                std::string params;
                for (auto const &p : ch[0]->GetChildren()) params += (params.empty() ? "" : ", ") + p->Text();
                out += Indent(depth) + "fun " + node->Text() + "(" + params + ")\n";
                Block(out, ch[2], depth + 1);
                return;
            }
            case Ast::If:
                // Decided by a pack's length: keep only the branch it selects.
                if (auto taken = Static(ch[0])) {
                    if (*taken)
                        Block(out, ch[1], depth);
                    else if (ch.size() > 2)
                        Statement(out, ch[2], depth);
                    return;
                }
                If(out, node, depth, "");
                return;
            case Ast::While:
                out += Indent(depth) + "while " + Expr(ch[0]) + "\n";
                Block(out, ch[1], depth + 1);
                return;
            case Ast::DoWhile:
                out += Indent(depth) + "do\n";
                Block(out, ch[0], depth + 1);
                out += Indent(depth) + "while " + Expr(ch[1]) + "\n";
                return;
            case Ast::For:
                out += Indent(depth) + "for " + Simple(ch[0]) + "; " + Optional(ch[1]) + "; " + Simple(ch[2]) + "\n";
                Block(out, ch[3], depth + 1);
                return;
            case Ast::ForEach:
                out += Indent(depth) + "for " + node->Text() + " in " + Expr(ch[0]) + "\n";
                Block(out, ch[1], depth + 1);
                return;
            case Ast::Block: Block(out, node, depth); return;
            default: out += Indent(depth) + Simple(node) + "\n"; return;
        }
    }

    std::string Optional(const NodePtr &node) { return node->GetType() == Ast::None ? "" : Expr(node); }

    // A statement that fits on one line (also used in for-loop headers).
    std::string Simple(const NodePtr &node) {
        const auto &ch = node->GetChildren();
        switch (node->GetType()) {
            case Ast::None: return "";
            case Ast::Declaration: return node->Text() + " = " + Expr(ch[1]);
            case Ast::Assignment: {
                const auto op = node->GetToken().type;
                const std::string target = Raw(ch[0]);
                if (op == Tok::Assign) return target + " = " + Expr(ch[1]);
                std::string text = node->Text();  // "+=", "-=", ...
                text.pop_back();
                std::string value = "(" + target + " " + text + " " + Expr(ch[1]) + ")";
                if (Widened(node)) value = "(" + value + " + 0.0)";
                return target + " = " + value;
            }
            case Ast::Return: return ch.empty() ? "return" : "return " + Expr(ch[0]);
            case Ast::Break: return "break";
            case Ast::Continue: return "continue";
            case Ast::Assert: return "assert(" + Expr(ch[0]) + ")";
            case Ast::ExprStatement: return Expr(ch[0]);
            default: return Expr(node);
        }
    }

    std::string Expr(const NodePtr &node) {
        std::string text = Raw(node);
        return Widened(node) ? "(" + text + " + 0.0)" : text;
    }

    static std::string StringLiteral(const NodePtr &node) {
        const auto &tok = node->GetToken();
        const std::string text = tok.Text();
        const auto &line = tok.lexer->GetLine(tok.lineNumber);
        const bool single = tok.slice.Start > 0 && line[tok.slice.Start - 1] == '\'';
        if (!single) return "\"" + text + "\"";

        std::string out = "\"";
        for (size_t n = 0; n < text.size(); ++n) {
            if (text[n] == '\\' && n + 1 < text.size() && text[n + 1] == '\'') {
                out += '\'';
                ++n;
            } else if (text[n] == '"') {
                out += "\\\"";
            } else {
                out += text[n];
            }
        }
        return out + "\"";
    }

    std::string Raw(const NodePtr &node) {
        const auto &ch = node->GetChildren();
        switch (node->GetType()) {
            case Ast::TokenType:
                return node->GetToken().type == Tok::String ? StringLiteral(node) : node->Text();
            case Ast::PiBlock: return node->Text();
            case Ast::Binary: {
                std::string left = Expr(ch[0]);
                // Rho reads '&' after a call's ')' as the suspend operator
                // whatever the spacing, so `f(x) & m` needs the call wrapped.
                if (node->GetToken().type == Tok::BitAnd && ch[0]->GetType() == Ast::Call) left = "(" + left + ")";
                return "(" + left + " " + node->Text() + " " + Expr(ch[1]) + ")";
            }
            case Ast::Unary: return "(" + node->Text() + Expr(ch[0]) + ")";
            case Ast::Ternary:
                if (auto taken = Static(ch[0])) return Expr(*taken ? ch[1] : ch[2]);
                return "(" + Expr(ch[0]) + " ? " + Expr(ch[1]) + " : " + Expr(ch[2]) + ")";
            case Ast::Fold: return FoldOut(node);
            case Ast::Member: return Raw(ch[0]) + "." + node->Text();
            case Ast::Index: return Raw(ch[0]) + "[" + Expr(ch[1]) + "]";
            case Ast::Call: {
                // xs.size() is the pack's length, a constant in each instance.
                if (ch[0]->GetType() == Ast::Member && ch[0]->Text() == "size" &&
                    ch[0]->GetChild(0)->GetType() == Ast::TokenType && packs_.contains(ch[0]->GetChild(0)->Text()))
                    return std::to_string(packs_.at(ch[0]->GetChild(0)->Text()));
                int count = 0;
                const std::string args = Items(ch[1]->GetChildren(), &count);
                std::string callee = Raw(ch[0]);
                if (ch[0]->GetType() == Ast::TokenType)
                    if (auto v = variadics_.find(callee); v != variadics_.end()) callee = Mangle(callee, count - v->second);
                // A continuation operator stays glued to the ')', as Rho expects.
                return callee + "(" + args + ")" + (ch.size() > 2 ? ch[2]->Text() : std::string());
            }
            case Ast::List: return "[" + Items(ch) + "]";
            case Ast::Map: {
                std::string items;
                for (auto const &e : ch)
                    items += (items.empty() ? "" : ", ") + StringLiteral(e) + ": " + Expr(e->GetChild(0));
                return "{" + items + "}";
            }
            default: return "";
        }
    }
};

}  // namespace

bool SigmaTranslator::Compile(const char *text) {
    rho_.clear();
    errors_.clear();
    failed = false;
    error.clear();
    if (text == nullptr || text[0] == 0) return true;

    auto lex = std::make_shared<SigmaLexer>(text, *reg_);
    lex->Process();
    if (lex->failed) {
        errors_.push_back(lex->error);
        return false;
    }

    SigmaParser parser(*reg_);
    parser.Process(lex, Structure::Program);
    if (parser.failed) {
        errors_.push_back(parser.GetError());
        return false;
    }

    SigmaChecker checker(reg_);
    checker.SetSession(session_);
    checker.SetSource(lex);
    if (!checker.Check(parser.GetRoot())) {
        auto diagnostics = checker.GetDiagnostics();
        std::stable_sort(diagnostics.begin(), diagnostics.end(), [](auto const &a, auto const &b) {
            return a.line != b.line ? a.line < b.line : a.column < b.column;
        });
        for (auto const &d : diagnostics) errors_.push_back(d.ToString());
        return false;
    }

    rho_ = RhoEmitter(checker).Program(parser.GetRoot());
    session_ = checker.GetGlobals();
    return true;
}

Pointer<Continuation> SigmaTranslator::Translate(const char *text, Structure st) {
    if (!Compile(text)) {
        std::string report;
        for (auto const &e : errors_) report += e + "\n";
        Fail(report);
        return Object();
    }
    if (rho_.empty()) return Object();

    // A Sigma input is always a sequence of statements, whatever structure
    // the caller guessed (the console guesses Expression for most lines).
    (void)st;
    RhoTranslator rho(*reg_);
    rho.trace = trace;
    auto cont = rho.Translate(rho_.c_str(), Structure::Program);
    if (rho.failed) {
        errors_.push_back("internal error: the generated Rho did not translate: " + rho.error);
        Fail(errors_.back());
        return Object();
    }
    return cont;
}

KAI_END
