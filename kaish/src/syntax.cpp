#include "kaish/syntax.hpp"

namespace kai::kaish {

    bool Word::is_plain() const {
        for (const auto& p : parts) {
            if (p.quote != Segment::Quote::None) return false;
        }
        return true;
    }

    std::string Word::raw() const {
        std::string s;
        for (const auto& p : parts) s += p.text;
        return s;
    }

    namespace {

        using Quote = Segment::Quote;

        struct Token {
            bool is_op = false;
            Word word;
            std::string op;
        };

        // Unquoted backslash only escapes shell metacharacters, so Windows paths
        // like C:\Users\chris and src\*.cpp keep working.
        bool is_escapable(char c) {
            switch (c) {
                case ' ': case '\t': case '\'': case '"': case '$': case '`': case '|':
                case '&': case ';': case '<': case '>': case '(': case ')': case '#': case '!':
                    return true;
                default:
                    return false;
            }
        }

        bool is_special(char c) {
            switch (c) {
                case ' ': case '\t': case '\r': case '\n': case '\\': case '\'': case '"': case '`':
                case '$': case '|': case '&': case ';': case '<': case '>': case '(': case ')':
                    return true;
                default:
                    return false;
            }
        }

        class Lexer {
        public:
            explicit Lexer(std::string_view src) : src_(src) {}

            std::vector<Token> run() {
                while (i_ < src_.size()) {
                    const char c = src_[i_];
                    if (c == ' ' || c == '\t' || c == '\r') { flush(); ++i_; continue; }
                    if (c == '\n') { op("\n"); ++i_; continue; }
                    if (c == '#' && !in_word_) {
                        while (i_ < src_.size() && src_[i_] != '\n') ++i_;
                        continue;
                    }
                    if (c == '\\') { escape(); continue; }
                    if (c == '\'') { single(); continue; }
                    if (c == '"') { dquote(); continue; }
                    if (c == '`') { ++i_; add(Quote::None, "$(" + backtick_body() + ")"); continue; }
                    if (c == '$') {
                        if (peek(1) == '(') { i_ += 2; add(Quote::None, "$(" + paren_body() + ")"); }
                        else { add(Quote::None, "$"); ++i_; }
                        continue;
                    }
                    if ((c == '0' || c == '1' || c == '2') && !in_word_ && (peek(1) == '>' || peek(1) == '<')) {
                        ++i_;
                        redirect(c - '0');
                        continue;
                    }
                    if (c == '>') { redirect(1); continue; }
                    if (c == '<') { redirect(0); continue; }
                    if (c == '&') {
                        if (peek(1) == '&') { op("&&"); i_ += 2; }
                        else if (peek(1) == '>') { op("&>"); i_ += 2; }
                        else { op("&"); ++i_; }
                        continue;
                    }
                    if (c == '|') {
                        if (peek(1) == '|') { op("||"); i_ += 2; }
                        else { op("|"); ++i_; }
                        continue;
                    }
                    if (c == ';') { op(";"); ++i_; continue; }
                    if (c == '(' || c == ')') {
                        throw SyntaxError(std::string("syntax error near unexpected token `") + c + "'");
                    }
                    const size_t start = i_;
                    while (i_ < src_.size() && !is_special(src_[i_])) ++i_;
                    add(Quote::None, src_.substr(start, i_ - start));
                }
                flush();
                return std::move(out_);
            }

        private:
            std::string_view src_;
            size_t i_ = 0;
            std::vector<Token> out_;
            Word cur_;
            bool in_word_ = false;

            char peek(size_t k) const { return i_ + k < src_.size() ? src_[i_ + k] : '\0'; }

            void add(Quote q, std::string_view text) {
                in_word_ = true;
                if (!cur_.parts.empty() && cur_.parts.back().quote == q) cur_.parts.back().text += text;
                else cur_.parts.push_back(Segment{std::string(text), q});
            }

            void flush() {
                if (!in_word_) return;
                Token t;
                t.word = std::move(cur_);
                out_.push_back(std::move(t));
                cur_ = Word{};
                in_word_ = false;
            }

            void op(std::string o) {
                flush();
                Token t;
                t.is_op = true;
                t.op = std::move(o);
                out_.push_back(std::move(t));
            }

            void escape() {
                if (i_ + 1 >= src_.size()) {
                    // "cd C:\" is a path, not a line continuation
                    if (i_ > 0 && src_[i_ - 1] == ':') { add(Quote::None, "\\"); ++i_; return; }
                    throw SyntaxError("unexpected end of input", true);
                }
                const char n = src_[i_ + 1];
                if (n == '\n') { i_ += 2; return; }
                if (n == '\r' && peek(2) == '\n') { i_ += 3; return; }
                if (is_escapable(n)) { add(Quote::Single, std::string(1, n)); i_ += 2; return; }
                add(Quote::None, "\\");
                ++i_;
            }

            void single() {
                const size_t end = src_.find('\'', i_ + 1);
                if (end == std::string_view::npos) {
                    throw SyntaxError("unexpected EOF while looking for matching `''", true);
                }
                add(Quote::Single, src_.substr(i_ + 1, end - i_ - 1));
                i_ = end + 1;
            }

            void dquote() {
                ++i_;
                std::string buf;
                for (;;) {
                    if (i_ >= src_.size()) {
                        throw SyntaxError("unexpected EOF while looking for matching `\"'", true);
                    }
                    const char c = src_[i_];
                    if (c == '"') { ++i_; break; }
                    if (c == '\\' && i_ + 1 < src_.size()) {
                        const char n = src_[i_ + 1];
                        if (n == '\n') { i_ += 2; continue; }
                        if (n == '"' || n == '$' || n == '`' || n == '\\') {
                            add(Quote::Double, buf);
                            buf.clear();
                            add(Quote::Single, std::string(1, n));
                            i_ += 2;
                            continue;
                        }
                    }
                    if (c == '$' && peek(1) == '(') { i_ += 2; buf += "$(" + paren_body() + ")"; continue; }
                    if (c == '`') { ++i_; buf += "$(" + backtick_body() + ")"; continue; }
                    buf += c;
                    ++i_;
                }
                add(Quote::Double, buf);
            }

            // Called just after "$(": returns the body and consumes the closing ')'.
            std::string paren_body() {
                const size_t start = i_;
                int depth = 1;
                while (i_ < src_.size()) {
                    const char c = src_[i_];
                    if (c == '\\') { i_ += 2; continue; }
                    if (c == '\'') {
                        const size_t e = src_.find('\'', i_ + 1);
                        if (e == std::string_view::npos) break;
                        i_ = e + 1;
                        continue;
                    }
                    if (c == '"') {
                        ++i_;
                        while (i_ < src_.size() && src_[i_] != '"') {
                            if (src_[i_] == '\\') ++i_;
                            ++i_;
                        }
                        if (i_ >= src_.size()) break;
                        ++i_;
                        continue;
                    }
                    if (c == '(') ++depth;
                    else if (c == ')' && --depth == 0) {
                        std::string body(src_.substr(start, i_ - start));
                        ++i_;
                        return body;
                    }
                    ++i_;
                }
                throw SyntaxError("unexpected EOF while looking for matching `)'", true);
            }

            // Called just after the opening backtick.
            std::string backtick_body() {
                std::string body;
                while (i_ < src_.size()) {
                    const char c = src_[i_];
                    if (c == '\\' && i_ + 1 < src_.size() && src_[i_ + 1] == '`') { body += '`'; i_ += 2; continue; }
                    if (c == '`') { ++i_; return body; }
                    body += c;
                    ++i_;
                }
                throw SyntaxError("unexpected EOF while looking for matching ``'", true);
            }

            // At '<' or '>' (after any fd digit).
            void redirect(int fd) {
                const char c = src_[i_++];
                if (c == '<') { op(std::to_string(fd) + "<"); return; }
                if (peek(0) == '>') { ++i_; op(std::to_string(fd) + ">>"); return; }
                if (peek(0) == '&' && (peek(1) == '1' || peek(1) == '2')) {
                    std::string o = std::to_string(fd) + ">&" + peek(1);
                    i_ += 2;
                    op(o);
                    return;
                }
                op(std::to_string(fd) + ">");
            }
        };

        SyntaxError unexpected(const std::string& tok) {
            return SyntaxError("syntax error near unexpected token `" + (tok == "\n" ? std::string("newline") : tok) + "'");
        }

    }

    CommandList parse_command_line(std::string_view source) {
        auto tokens = Lexer(source).run();
        CommandList list;
        Pipeline pipe;
        SimpleCommand cmd;
        auto cmd_empty = [&] { return cmd.words.empty() && cmd.redirects.empty(); };

        for (size_t i = 0; i < tokens.size(); ++i) {
            Token& t = tokens[i];
            if (!t.is_op) {
                cmd.words.push_back(std::move(t.word));
                continue;
            }
            const std::string o = t.op;
            if (o == "|") {
                if (cmd_empty()) throw unexpected(o);
                pipe.commands.push_back(std::move(cmd));
                cmd = SimpleCommand{};
                continue;
            }
            if (o == "\n" || o == ";" || o == "&&" || o == "||") {
                if (cmd_empty()) {
                    if (o == "\n") continue;                     // blank line, or newline after | && ||
                    if (o == ";" && pipe.commands.empty()) continue;
                    throw unexpected(o);
                }
                pipe.commands.push_back(std::move(cmd));
                cmd = SimpleCommand{};
                const Connector c = o == "&&" ? Connector::And : o == "||" ? Connector::Or : Connector::Seq;
                list.push_back(ListItem{std::move(pipe), c});
                pipe = Pipeline{};
                continue;
            }
            if (o == "&") throw SyntaxError("background jobs (&) are not supported");

            Redirect r;
            if (o == "&>") {
                r.kind = Redirect::Kind::OutErr;
            } else {
                r.fd = o[0] - '0';
                const std::string rest = o.substr(1);
                if (rest == "<") r.kind = Redirect::Kind::In;
                else if (rest == ">") r.kind = Redirect::Kind::Out;
                else if (rest == ">>") r.kind = Redirect::Kind::Append;
                else { r.kind = Redirect::Kind::Dup; r.dup_fd = rest[2] - '0'; }
            }
            if (r.kind != Redirect::Kind::Dup) {
                if (i + 1 >= tokens.size()) throw unexpected("\n");
                if (tokens[i + 1].is_op) throw unexpected(tokens[i + 1].op);
                r.target = std::move(tokens[++i].word);
            }
            cmd.redirects.push_back(std::move(r));
        }

        if (!cmd_empty()) pipe.commands.push_back(std::move(cmd));
        else if (!pipe.commands.empty()) throw SyntaxError("unexpected end of input", true);

        if (!pipe.commands.empty()) list.push_back(ListItem{std::move(pipe), Connector::Seq});
        else if (!list.empty() && list.back().next != Connector::Seq) throw SyntaxError("unexpected end of input", true);
        return list;
    }

}
