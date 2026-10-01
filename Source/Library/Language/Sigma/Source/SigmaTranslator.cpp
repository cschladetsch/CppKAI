#include <KAI/Language/Rho/RhoTranslator.h>
#include <KAI/Language/Sigma/SigmaTranslator.h>

#include <algorithm>

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
class RhoEmitter {
   public:
    explicit RhoEmitter(const std::unordered_set<const SigmaAstNode *> &widened) : widened_(widened) {}

    std::string Program(const NodePtr &root) {
        std::string out;
        for (auto const &st : root->GetChildren()) Statement(out, st, 0);
        return out;
    }

   private:
    const std::unordered_set<const SigmaAstNode *> &widened_;

    static std::string Indent(int depth) { return std::string(static_cast<size_t>(depth) * 4, ' '); }

    void Block(std::string &out, const NodePtr &block, int depth) {
        for (auto const &st : block->GetChildren()) Statement(out, st, depth);
    }

    void If(std::string &out, const NodePtr &node, int depth, const std::string &prefix) {
        out += Indent(depth) + prefix + "if " + Expr(node->GetChild(0)) + "\n";
        Block(out, node->GetChild(1), depth + 1);
        if (node->GetChildren().size() < 3) return;
        const auto &other = node->GetChild(2);
        if (other->GetType() == Ast::If) {
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
                std::string params;
                for (auto const &p : ch[0]->GetChildren()) params += (params.empty() ? "" : ", ") + p->Text();
                out += Indent(depth) + "fun " + node->Text() + "(" + params + ")\n";
                Block(out, ch[2], depth + 1);
                return;
            }
            case Ast::If: If(out, node, depth, ""); return;
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
                if (widened_.contains(node.get())) value = "(" + value + " + 0.0)";
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
        return widened_.contains(node.get()) ? "(" + text + " + 0.0)" : text;
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
            case Ast::Binary: return "(" + Expr(ch[0]) + " " + node->Text() + " " + Expr(ch[1]) + ")";
            case Ast::Unary: return "(" + node->Text() + Expr(ch[0]) + ")";
            case Ast::Ternary: return "(" + Expr(ch[0]) + " ? " + Expr(ch[1]) + " : " + Expr(ch[2]) + ")";
            case Ast::Member: return Raw(ch[0]) + "." + node->Text();
            case Ast::Index: return Raw(ch[0]) + "[" + Expr(ch[1]) + "]";
            case Ast::Call: {
                std::string args;
                for (auto const &a : ch[1]->GetChildren()) args += (args.empty() ? "" : ", ") + Expr(a);
                return Raw(ch[0]) + "(" + args + ")";
            }
            case Ast::List: {
                std::string items;
                for (auto const &e : ch) items += (items.empty() ? "" : ", ") + Expr(e);
                return "[" + items + "]";
            }
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
    if (!checker.Check(parser.GetRoot())) {
        auto diagnostics = checker.GetDiagnostics();
        std::stable_sort(diagnostics.begin(), diagnostics.end(), [](auto const &a, auto const &b) {
            return a.line != b.line ? a.line < b.line : a.column < b.column;
        });
        for (auto const &d : diagnostics) errors_.push_back(d.ToString());
        return false;
    }

    rho_ = RhoEmitter(checker.GetWidened()).Program(parser.GetRoot());
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
