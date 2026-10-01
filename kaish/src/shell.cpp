#include "kaish/shell.hpp"
#include "kaish/platform.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace kai::kaish {

    namespace fs = std::filesystem;
    using platform::from_path;
    using platform::to_path;

    struct Shell::StageIo {
        std::string in;
        std::string out;
        std::string err;
        bool out_append = false;
        bool err_append = false;
        bool err_to_out = false;
        bool out_to_err = false;
        bool to_stack = false;
    };

    bool is_valid_name(std::string_view s) {
        if (s.empty() || !(std::isalpha(static_cast<unsigned char>(s[0])) || s[0] == '_')) return false;
        return std::all_of(s.begin(), s.end(), [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; });
    }

    namespace {

        // Marks the next character of a glob pattern as literal (it came from quotes).
        constexpr char kEsc = '\x01';

#ifdef _WIN32
        constexpr bool kFoldCase = true;
#else
        constexpr bool kFoldCase = false;
#endif

        char fold(char c) { return kFoldCase ? static_cast<char>(std::tolower(static_cast<unsigned char>(c))) : c; }
        bool is_sep(char c) { return c == '/' || c == '\\'; }

        bool match(const char* p, const char* s) {
            while (*p) {
                if (*p == kEsc && p[1]) {
                    if (!*s || fold(*s) != fold(p[1])) return false;
                    p += 2;
                    ++s;
                    continue;
                }
                if (*p == '*') {
                    while (*p == '*') ++p;
                    if (!*p) return true;
                    for (; *s; ++s) {
                        if (match(p, s)) return true;
                    }
                    return match(p, s);
                }
                if (!*s) return false;
                if (*p == '?') { ++p; ++s; continue; }
                if (*p == '[') {
                    const char* q = p + 1;
                    bool negate = false;
                    if (*q == '!' || *q == '^') { negate = true; ++q; }
                    const char* first = q;
                    bool hit = false;
                    while (*q && (*q != ']' || q == first)) {
                        if (q[1] == '-' && q[2] && q[2] != ']') {
                            if (fold(*s) >= fold(*q) && fold(*s) <= fold(q[2])) hit = true;
                            q += 3;
                        } else {
                            if (fold(*s) == fold(*q)) hit = true;
                            ++q;
                        }
                    }
                    if (*q != ']') { // unterminated: literal '['
                        if (*s != '[') return false;
                        ++p;
                        ++s;
                        continue;
                    }
                    if (hit == negate) return false;
                    p = q + 1;
                    ++s;
                    continue;
                }
                if (fold(*p) != fold(*s)) return false;
                ++p;
                ++s;
            }
            return *s == 0;
        }

        bool has_wild(const std::string& pat) {
            for (size_t i = 0; i < pat.size(); ++i) {
                if (pat[i] == kEsc) { ++i; continue; }
                if (pat[i] == '*' || pat[i] == '?' || pat[i] == '[') return true;
            }
            return false;
        }

        std::string unescape(const std::string& pat) {
            std::string r;
            for (size_t i = 0; i < pat.size(); ++i) {
                if (pat[i] == kEsc && i + 1 < pat.size()) r += pat[++i];
                else r += pat[i];
            }
            return r;
        }

        std::vector<std::string> glob(const std::string& pattern) {
            size_t pos = 0;
            std::string root;
            if (pattern.size() >= 2 && std::isalpha(static_cast<unsigned char>(pattern[0])) && pattern[1] == ':') {
                root = pattern.substr(0, 2);
                pos = 2;
            }
            while (pos < pattern.size() && is_sep(pattern[pos])) root += pattern[pos++];

            std::vector<std::pair<std::string, std::string>> parts; // component, separator after it
            while (pos < pattern.size()) {
                size_t e = pos;
                while (e < pattern.size() && !is_sep(pattern[e])) e += (pattern[e] == kEsc) ? 2 : 1;
                e = std::min(e, pattern.size());
                size_t s = e;
                while (s < pattern.size() && is_sep(pattern[s])) ++s;
                parts.emplace_back(pattern.substr(pos, e - pos), pattern.substr(e, s - e));
                pos = s;
            }

            std::vector<std::string> bases{root};
            for (const auto& [comp, sep] : parts) {
                std::vector<std::string> next;
                if (!has_wild(comp)) {
                    for (const auto& b : bases) next.push_back(b + unescape(comp) + sep);
                } else {
                    const bool want_dot = comp[0] == '.' || (comp[0] == kEsc && comp.size() > 1 && comp[1] == '.');
                    for (const auto& b : bases) {
                        std::error_code ec;
                        const fs::path dir = b.empty() ? fs::path(".") : to_path(b);
                        std::vector<std::string> names;
                        for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec)) {
                            const std::string name = from_path(it->path().filename());
                            if (name.empty() || (name[0] == '.' && !want_dot)) continue;
                            if (match(comp.c_str(), name.c_str())) names.push_back(name);
                        }
                        std::sort(names.begin(), names.end());
                        for (const auto& n : names) next.push_back(b + n + sep);
                    }
                }
                bases = std::move(next);
                if (bases.empty()) break;
            }

            std::vector<std::string> out;
            for (const auto& b : bases) {
                std::error_code ec;
                if (fs::exists(to_path(b), ec)) out.push_back(b);
            }
            if (out.empty()) return {unescape(pattern)}; // bash: no match leaves the word as-is
            return out;
        }

        size_t matching_paren(const std::string& t, size_t open) {
            int depth = 0;
            for (size_t i = open; i < t.size(); ++i) {
                const char c = t[i];
                if (c == '\\') { ++i; continue; }
                if (c == '\'') {
                    const size_t e = t.find('\'', i + 1);
                    if (e == std::string::npos) return std::string::npos;
                    i = e;
                    continue;
                }
                if (c == '"') {
                    ++i;
                    while (i < t.size() && t[i] != '"') {
                        if (t[i] == '\\') ++i;
                        ++i;
                    }
                    continue;
                }
                if (c == '(') ++depth;
                else if (c == ')' && --depth == 0) return i;
            }
            return std::string::npos;
        }

        std::string device_path(const std::string& p) {
#ifdef _WIN32
            if (p == "/dev/null") return "NUL";
#endif
            return p;
        }

        std::string strip_bom(std::string s) {
            if (s.size() >= 3 && s.compare(0, 3, "\xEF\xBB\xBF") == 0) s.erase(0, 3);
            return s;
        }

        // Pushes command output as one stack entry, without the trailing newline.
        void push_output(ValueStack& stack, std::string text) {
            while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) text.pop_back();
            if (!text.empty()) stack.push_text(text);
        }

        // These print for the user rather than produce a value.
        bool is_display_builtin(const std::string& name) {
            return name == "help" || name == "history" || name == "clear";
        }

        std::string trim(const std::string& s) {
            const size_t a = s.find_first_not_of(" \t\r\n");
            if (a == std::string::npos) return "";
            const size_t b = s.find_last_not_of(" \t\r\n");
            return s.substr(a, b - a + 1);
        }

        size_t utf8_width(std::string_view s) {
            size_t n = 0;
            for (unsigned char c : s) {
                if ((c & 0xC0) != 0x80) ++n;
            }
            return n;
        }

        std::string utf8_prefix(const std::string& s, size_t chars) {
            size_t n = 0;
            for (size_t i = 0; i < s.size(); ++i) {
                if ((static_cast<unsigned char>(s[i]) & 0xC0) != 0x80) {
                    if (n == chars) return s.substr(0, i);
                    ++n;
                }
            }
            return s;
        }

        std::optional<Mode> mode_word(const std::string& w) {
            if (w == "ps") return Mode::Ps;
            if (w == "pi") return Mode::Pi;
            if (w == "rho") return Mode::Rho;
            return std::nullopt;
        }

        bool starts_with_path(const std::string& s, const std::string& prefix) {
            if (prefix.empty() || s.size() < prefix.size()) return false;
            for (size_t i = 0; i < prefix.size(); ++i) {
                if (fold(s[i]) != fold(prefix[i]) && !(is_sep(s[i]) && is_sep(prefix[i]))) return false;
            }
            return s.size() == prefix.size() || is_sep(s[prefix.size()]);
        }

    }

    Shell::Shell(std::unique_ptr<Backend> b, Config c)
        : backend(std::move(b)), config(std::move(c)), mode(config.mode) {
        if (!backend) backend = make_text_backend();
        if (!backend->has_languages()) mode = Mode::Ps;
        register_builtins(*this);
        aliases["ll"] = "ls -alF";
        aliases["la"] = "ls -A";
        aliases["l"] = "ls -CF";
        std::error_code ec;
        platform::set_env("PWD", from_path(fs::current_path(ec)));
    }

    std::optional<std::string> Shell::get_var(const std::string& name) const {
        if (auto it = vars.find(name); it != vars.end()) return it->second;
        return platform::get_env(name);
    }

    void Shell::set_var(const std::string& name, const std::string& value) {
        if (vars.count(name) == 0 && platform::get_env(name)) platform::set_env(name, value);
        else vars[name] = value;
    }

    void Shell::export_var(const std::string& name, const std::string& value) {
        vars.erase(name);
        platform::set_env(name, value);
    }

    void Shell::unset_var(const std::string& name) {
        vars.erase(name);
        platform::unset_env(name);
    }

    std::string Shell::temp_path() {
        std::error_code ec;
        fs::path dir = fs::temp_directory_path(ec);
        if (ec) dir = fs::current_path();
        const std::string name = "kaish-" + std::to_string(platform::process_id()) + "-" + std::to_string(++temp_counter_) + ".tmp";
        return from_path(dir / to_path(name));
    }

    std::string Shell::history_file() const {
        return from_path(platform::home_dir() / ".kaish_history");
    }

    int Shell::execute(std::string_view source, bool to_stack) {
        CommandList list;
        try {
            list = parse_command_line(source);
        } catch (const SyntaxError& e) {
            std::cerr << "kaish: " << e.what() << "\n";
            return status = 2;
        }
        StageIo io;
        io.to_stack = to_stack;
        return run_list(list, io);
    }

    std::string Shell::capture(std::string_view source) {
        StageIo io;
        io.out = temp_path();
        const bool was_exiting = exiting;
        try {
            run_list(parse_command_line(source), io);
        } catch (const SyntaxError& e) {
            std::cerr << "kaish: " << e.what() << "\n";
            status = 2;
        }
        exiting = was_exiting;
        std::string text;
        {
            std::ifstream f(to_path(io.out), std::ios::binary);
            if (f) {
                std::ostringstream ss;
                ss << f.rdbuf();
                text = ss.str();
            }
        }
        std::error_code ec;
        fs::remove(to_path(io.out), ec);
        return text;
    }

    int Shell::run_list(const CommandList& list, const StageIo& io) {
        Connector prev = Connector::Seq;
        for (const auto& item : list) {
            if (exiting) break;
            const bool run = prev == Connector::Seq || (prev == Connector::And && status == 0) ||
                             (prev == Connector::Or && status != 0);
            if (run) status = run_pipeline(item.pipeline, io);
            prev = item.next;
        }
        return status;
    }

    // Pipeline stages run in sequence and hand data through temp files. Not streaming,
    // but it behaves the same for builtins and external programs on every OS.
    int Shell::run_pipeline(const Pipeline& p, const StageIo& io) {
        std::vector<std::string> temps;
        std::string prev_out;
        int rc = 0;
        for (size_t i = 0; i < p.commands.size(); ++i) {
            StageIo s = io;
            if (i > 0) s.in = prev_out;
            if (i + 1 < p.commands.size()) {
                prev_out = temp_path();
                temps.push_back(prev_out);
                s.out = prev_out;
                s.out_append = false;
                s.out_to_err = false;
                s.to_stack = false;
            }
            try {
                rc = run_command(p.commands[i], s);
            } catch (const std::exception& e) {
                std::cerr << "kaish: " << e.what() << "\n";
                rc = 1;
            }
            if (exiting) break;
        }
        for (const auto& t : temps) {
            std::error_code ec;
            fs::remove(to_path(t), ec);
        }
        return rc;
    }

    std::vector<std::string> Shell::expand(const Word& w) {
        std::string result; // glob pattern; literal wildcard chars are kEsc-escaped
        bool wild = false;
        bool any_quoted = false;
        auto lit = [&](const std::string& s) {
            for (char c : s) {
                if (c == '*' || c == '?' || c == '[' || c == kEsc) result += kEsc;
                result += c;
            }
        };

        for (size_t pi = 0; pi < w.parts.size(); ++pi) {
            const Segment& seg = w.parts[pi];
            if (seg.quote != Segment::Quote::None) any_quoted = true;
            if (seg.quote == Segment::Quote::Single) { lit(seg.text); continue; }

            const std::string& t = seg.text;
            const bool unquoted = seg.quote == Segment::Quote::None;
            size_t i = 0;
            if (unquoted && pi == 0 && !t.empty() && t[0] == '~' && (t.size() == 1 || is_sep(t[1]))) {
                lit(from_path(platform::home_dir()));
                i = 1;
            }
            // @N at the start of an unquoted word is stack level N (not consumed).
            // Quote it ('@1') for a literal; mid-word @ (user@host, pkg@4) is untouched.
            if (unquoted && pi == 0 && t.size() > 1 && t[0] == '@' && std::isdigit(static_cast<unsigned char>(t[1]))) {
                size_t j = 1;
                while (j < t.size() && std::isdigit(static_cast<unsigned char>(t[j]))) ++j;
                const std::string ref = t.substr(0, j);
                const size_t level = std::stoul(t.substr(1, j - 1));
                if (level == 0 || level > stack().depth()) throw std::runtime_error(ref + ": Too few arguments");
                lit(stack().text(level));
                i = j;
            }
            while (i < t.size()) {
                const char c = t[i];
                if (c == '$' && i + 1 < t.size()) {
                    const char n = t[i + 1];
                    if (n == '(') {
                        const size_t close = matching_paren(t, i + 1);
                        if (close != std::string::npos) {
                            std::string out = capture(t.substr(i + 2, close - i - 2));
                            while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
                            lit(out);
                            i = close + 1;
                            continue;
                        }
                    }
                    if (n == '{') {
                        const size_t e = t.find('}', i + 2);
                        if (e != std::string::npos) {
                            lit(get_var(t.substr(i + 2, e - i - 2)).value_or(""));
                            i = e + 1;
                            continue;
                        }
                    }
                    if (n == '?') { lit(std::to_string(status)); i += 2; continue; }
                    if (n == '$') { lit(std::to_string(platform::process_id())); i += 2; continue; }
                    if (n == '#') { lit(std::to_string(positional.empty() ? 0 : positional.size() - 1)); i += 2; continue; }
                    if (std::isdigit(static_cast<unsigned char>(n))) {
                        const size_t k = static_cast<size_t>(n - '0');
                        lit(k < positional.size() ? positional[k] : "");
                        i += 2;
                        continue;
                    }
                    if (std::isalpha(static_cast<unsigned char>(n)) || n == '_') {
                        size_t j = i + 1;
                        while (j < t.size() && (std::isalnum(static_cast<unsigned char>(t[j])) || t[j] == '_')) ++j;
                        lit(get_var(t.substr(i + 1, j - i - 1)).value_or(""));
                        i = j;
                        continue;
                    }
                }
                if (unquoted && (c == '*' || c == '?' || c == '[')) {
                    wild = true;
                    result += c;
                    ++i;
                    continue;
                }
                lit(std::string(1, c));
                ++i;
            }
        }

        if (!any_quoted && result.empty()) return {}; // unquoted empty expansion vanishes
        if (wild) return glob(result);
        return {unescape(result)};
    }

    int Shell::run_command(const SimpleCommand& cmd, StageIo io) {
        // Leading NAME=value words
        std::vector<std::pair<std::string, std::string>> assigns;
        size_t wi = 0;
        for (; wi < cmd.words.size(); ++wi) {
            const Word& w = cmd.words[wi];
            if (w.parts.empty() || w.parts[0].quote != Segment::Quote::None) break;
            const size_t eq = w.parts[0].text.find('=');
            if (eq == std::string::npos || !is_valid_name(w.parts[0].text.substr(0, eq))) break;
            Word value = w;
            value.parts[0].text = value.parts[0].text.substr(eq + 1);
            std::string joined;
            for (const auto& s : expand(value)) joined += (joined.empty() ? "" : " ") + s;
            assigns.emplace_back(w.parts[0].text.substr(0, eq), joined);
        }

        Args argv;
        for (size_t k = wi; k < cmd.words.size(); ++k) {
            const Word& w = cmd.words[k];
            if (k == wi && w.is_plain()) {
                if (auto it = aliases.find(w.raw()); it != aliases.end()) {
                    try {
                        auto alias = parse_command_line(it->second);
                        if (alias.size() == 1 && alias[0].pipeline.commands.size() == 1) {
                            for (const auto& aw : alias[0].pipeline.commands[0].words) {
                                for (auto& s : expand(aw)) argv.push_back(std::move(s));
                            }
                            continue;
                        }
                    } catch (const SyntaxError&) {
                    }
                }
            }
            for (auto& s : expand(w)) argv.push_back(std::move(s));
        }

        for (const auto& r : cmd.redirects) {
            std::string target;
            if (r.kind != Redirect::Kind::Dup) {
                auto t = expand(r.target);
                if (t.size() != 1) {
                    std::cerr << "kaish: " << r.target.raw() << ": ambiguous redirect\n";
                    return 1;
                }
                target = device_path(t[0]);
            }
            switch (r.kind) {
                case Redirect::Kind::In:
                    io.in = target;
                    break;
                case Redirect::Kind::Out:
                case Redirect::Kind::Append:
                    if (r.fd == 2) {
                        io.err = target;
                        io.err_append = r.kind == Redirect::Kind::Append;
                        io.err_to_out = false;
                    } else {
                        io.out = target;
                        io.out_append = r.kind == Redirect::Kind::Append;
                        io.out_to_err = false;
                    }
                    break;
                case Redirect::Kind::OutErr:
                    io.out = target;
                    io.out_append = false;
                    io.out_to_err = false;
                    io.err_to_out = true;
                    break;
                case Redirect::Kind::Dup:
                    if (r.fd == 2 && r.dup_fd == 1) io.err_to_out = true;
                    else if (r.fd == 1 && r.dup_fd == 2) io.out_to_err = true;
                    break;
            }
        }

        if (argv.empty()) {
            for (const auto& [n, v] : assigns) set_var(n, v);
            if (!io.out.empty()) {
                std::ofstream touch(to_path(io.out), std::ios::binary | (io.out_append ? std::ios::app : std::ios::trunc));
            }
            return 0;
        }

        // `command x` skips builtins and aliases; `term x` gives x the terminal
        // instead of capturing its output onto the stack.
        bool skip_builtins = false;
        bool on_terminal = false;
        while (argv.size() > 1 && (argv[0] == "command" || argv[0] == "term")) {
            (argv[0] == "command" ? skip_builtins : on_terminal) = true;
            argv.erase(argv.begin());
        }
        const bool collect = io.to_stack && !on_terminal && io.out.empty() && !io.out_to_err;

        // FOO=bar cmd: set for this command only
        std::vector<std::pair<std::string, std::optional<std::string>>> saved;
        for (const auto& [n, v] : assigns) {
            saved.emplace_back(n, platform::get_env(n));
            platform::set_env(n, v);
        }
        struct Restore {
            std::vector<std::pair<std::string, std::optional<std::string>>>& saved;
            ~Restore() {
                for (auto it = saved.rbegin(); it != saved.rend(); ++it) {
                    if (it->second) platform::set_env(it->first, *it->second);
                    else platform::unset_env(it->first);
                }
            }
        } restore{saved};

        auto builtin = skip_builtins ? builtins.end() : builtins.find(argv[0]);
        if (builtin != builtins.end()) {
            std::ifstream fin;
            std::ofstream fout;
            std::ofstream ferr;
            std::istream* in = &std::cin;
            std::ostream* out = &std::cout;
            std::ostream* err = &std::cerr;
            if (!io.in.empty()) {
                fin.open(to_path(io.in), std::ios::binary);
                if (!fin) {
                    std::cerr << "kaish: " << io.in << ": No such file or directory\n";
                    return 1;
                }
                in = &fin;
            }
            if (!io.out.empty()) {
                fout.open(to_path(io.out), std::ios::binary | (io.out_append ? std::ios::app : std::ios::trunc));
                if (!fout) {
                    std::cerr << "kaish: " << io.out << ": cannot open for writing\n";
                    return 1;
                }
                out = &fout;
            }
            if (!io.err.empty() && !io.err_to_out) {
                ferr.open(to_path(io.err), std::ios::binary | (io.err_append ? std::ios::app : std::ios::trunc));
                if (!ferr) {
                    std::cerr << "kaish: " << io.err << ": cannot open for writing\n";
                    return 1;
                }
                err = &ferr;
            }
            std::ostringstream collected;
            const bool collect_here = collect && !is_display_builtin(argv[0]);
            if (collect_here) out = &collected;
            if (io.err_to_out) err = out;
            if (io.out_to_err) out = err;

            CommandIo cio{*in, *out, *err, out == &std::cout && platform::stdout_is_console(),
                          collect_here ? &stack() : nullptr};
            int rc = 1;
            try {
                rc = builtin->second(*this, argv, cio);
            } catch (const std::exception& e) {
                std::cerr << argv[0] << ": " << e.what() << "\n";
            } catch (...) {
                std::cerr << argv[0] << ": failed\n";
            }
            out->flush();
            err->flush();
            std::cin.clear();
            if (collect_here) push_output(stack(), collected.str());
            return rc;
        }

        const auto exe = platform::find_executable(argv[0]);
        if (!exe) {
            std::cerr << "kaish: " << argv[0] << ": command not found\n";
            return 127;
        }
        platform::StdioSpec spec;
        spec.in_path = io.in;
        spec.out_path = io.out;
        spec.out_append = io.out_append;
        spec.err_path = io.err;
        spec.err_append = io.err_append;
        spec.err_to_out = io.err_to_out;
        spec.out_to_err = io.out_to_err;
        std::string captured;
        if (collect && !is_passthrough(argv[0])) {
            captured = temp_path();
            spec.out_path = captured;
            spec.out_append = false;
        }
        std::cout.flush();
        std::cerr.flush();
        int rc = 1;
        try {
            rc = platform::run_process(*exe, argv, spec);
            if (rc < 0) {
                std::cerr << "kaish: " << argv[0] << ": cannot execute\n";
                rc = 126;
            }
        } catch (const std::exception& e) {
            std::cerr << "kaish: " << e.what() << "\n";
            rc = 1;
        }
        if (!captured.empty()) {
            {
                std::ifstream f(to_path(captured), std::ios::binary);
                std::ostringstream ss;
                if (f) ss << f.rdbuf();
                push_output(stack(), ss.str());
            }
            std::error_code ec;
            fs::remove(to_path(captured), ec);
        }
        return rc;
    }

    bool Shell::is_passthrough(const std::string& program) const {
        std::string name = from_path(to_path(program).stem());
        if (kFoldCase) {
            for (auto& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        for (std::string p : config.passthrough) {
            if (kFoldCase) {
                for (auto& c : p) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            if (p == name) return true;
        }
        return false;
    }

    std::string Shell::prompt() const {
        std::error_code ec;
        std::string cwd = from_path(fs::current_path(ec));
        const std::string home = from_path(platform::home_dir());
        if (starts_with_path(cwd, home)) cwd = "~" + cwd.substr(home.size());
        std::replace(cwd.begin(), cwd.end(), '\\', '/');
        const std::string tag = mode_name(mode);
        const std::string sym = mode == Mode::Ps ? "$ " : " \xCE\xBB ";  // λ for pi and rho
        if (!platform::stdout_is_console()) return tag + " " + cwd + sym;
        const char* color = mode == Mode::Ps ? "\x1b[1;32m" : mode == Mode::Pi ? "\x1b[1;35m" : "\x1b[1;36m";
        return std::string(color) + tag + "\x1b[0m \x1b[1;34m" + cwd + "\x1b[0m" + sym;
    }

    // HP48 style: level 1 sits just above the prompt.
    void Shell::render_stack(std::ostream& out) {
        size_t depth = 0;
        try {
            depth = stack().depth();
        } catch (...) {
            return;
        }
        if (depth == 0) return;
        const size_t levels = std::min(depth, static_cast<size_t>(std::max(1, config.stack_levels)));
        const bool color = platform::stdout_is_console();
        const size_t width = static_cast<size_t>(std::max(20, platform::terminal_width()));
        const size_t num_w = std::to_string(levels).size();
        const char* dim = color ? "\x1b[2m" : "";
        const char* reset = color ? "\x1b[0m" : "";
        if (depth > levels) {
            out << dim << std::string(num_w, ' ') << "  \xE2\x8B\xAE " << depth - levels << " more" << reset << '\n';
        }
        for (size_t level = levels; level >= 1; --level) {
            std::string value;
            bool text = false;
            try {
                value = stack().show(level);
                text = stack().is_text(level);
            } catch (const std::exception& e) {
                value = std::string("<") + e.what() + ">";
            }
            std::string note;
            if (const size_t nl = value.find('\n'); nl != std::string::npos) {
                const size_t lines = 1 + static_cast<size_t>(std::count(value.begin() + static_cast<std::ptrdiff_t>(nl), value.end(), '\n'));
                value = value.substr(0, nl) + "\xE2\x80\xA6";
                note = " (" + std::to_string(lines) + " lines)";
            }
            const size_t room = width > num_w + 3 + utf8_width(note) ? width - num_w - 3 - utf8_width(note) : 1;
            if (utf8_width(value) > room) value = utf8_prefix(value, room > 1 ? room - 1 : 0) + "\xE2\x80\xA6";
            out << (color ? "\x1b[33m" : "");
            out << std::string(num_w - std::to_string(level).size(), ' ') << level << ':' << reset << ' ';
            out << (color && text ? "\x1b[32m" : "") << value << reset << dim << note << reset << '\n';
        }
    }

    bool Shell::needs_more_input(const std::string& text) {
        if (mode != Mode::Ps) return backend->incomplete(mode, text);
        try {
            parse_command_line(text);
            return false;
        } catch (const SyntaxError& e) {
            return e.incomplete;
        }
    }

    void Shell::dispatch(const std::string& line) {
        const std::string trimmed = trim(line);
        if (trimmed.empty()) return;
        const size_t sp = trimmed.find_first_of(" \t");
        const std::string head = trimmed.substr(0, sp);
        const std::string rest = sp == std::string::npos ? "" : trim(trimmed.substr(sp));
        if (head == "ps" && !rest.empty()) {   // `ps aux` is the program, from any mode
            run_in(Mode::Ps, trimmed);
            return;
        }
        if (const auto m = mode_word(head)) {
            if (*m != Mode::Ps && !backend->has_languages()) {
                std::cerr << "kaish: " << head << ": not available (kaish was built without KAI)\n";
                status = 1;
                return;
            }
            if (rest.empty()) mode = *m;   // `pi` switches mode
            else run_in(*m, rest);         // `pi 1 2 +` runs once
            return;
        }
        run_in(mode, trimmed);
    }

    void Shell::run_in(Mode m, const std::string& code) {
        if (m == Mode::Ps) {
            execute(code, true);
            return;
        }
        if (code == "exit" || code == "quit") {
            exiting = true;
            exit_code = 0;
            return;
        }
        if (code == "clr") { stack().clear(); return; }
        if (code == "cls") { execute("clear"); return; }
        if (code == "clear" || code == "history") { execute(code); return; }
        if (code[0] == '$') {            // shell escape from pi/rho: `$ ls`
            execute(trim(code.substr(1)), true);
            return;
        }
        const std::string err = backend->eval(m, code);
        status = err.empty() ? 0 : 1;
        if (!err.empty()) {
            std::cerr << err;
            if (err.back() != '\n') std::cerr << '\n';
        }
    }

    std::optional<std::string> Shell::expand_history(const std::string& line) const {
        std::string out;
        for (size_t i = 0; i < line.size(); ++i) {
            const char c = line[i];
            if (c == '\'') {
                size_t e = line.find('\'', i + 1);
                if (e == std::string::npos) e = line.size() - 1;
                out += line.substr(i, e - i + 1);
                i = e;
                continue;
            }
            if (c == '!' && i + 1 < line.size()) {
                if (line[i + 1] == '!') {
                    if (history.empty()) {
                        std::cerr << "kaish: !!: event not found\n";
                        return std::nullopt;
                    }
                    out += history.back();
                    ++i;
                    continue;
                }
                if (std::isdigit(static_cast<unsigned char>(line[i + 1]))) {
                    size_t j = i + 1;
                    while (j < line.size() && std::isdigit(static_cast<unsigned char>(line[j]))) ++j;
                    const std::string num = line.substr(i + 1, j - i - 1);
                    const size_t n = std::stoul(num);
                    if (n == 0 || n > history.size()) {
                        std::cerr << "kaish: !" << num << ": event not found\n";
                        return std::nullopt;
                    }
                    out += history[n - 1];
                    i = j - 1;
                    continue;
                }
            }
            out += c;
        }
        return out;
    }

    int Shell::run_interactive() {
        platform::ignore_interrupts();
        {
            std::ifstream h(to_path(history_file()));
            std::string line;
            while (std::getline(h, line)) {
                if (!line.empty()) history.push_back(line);
            }
            if (history.size() > 1000) history.erase(history.begin(), history.end() - 1000);
        }
        const fs::path rc = platform::home_dir() / ".kaishrc";
        std::error_code ec;
        if (fs::exists(rc, ec)) run_file(from_path(rc), {});

        std::cout << "kaish (KAI Object Shell) v0.3 - 'pi', 'rho' or 'ps' to switch, 'help' for more\n";
        std::string line;
        while (!exiting) {
            if (config.show_stack) render_stack(std::cout);
            std::cout << prompt() << std::flush;
            if (!platform::read_line(line)) {
                std::cout << "exit\n";
                break;
            }
            while (needs_more_input(line)) {
                std::cout << "> " << std::flush;
                std::string more;
                if (!platform::read_line(more)) break;
                line += "\n" + more;
            }
            if (line.find_first_not_of(" \t") == std::string::npos) continue;

            if (mode == Mode::Ps) {  // pi uses '!' itself, so history expansion is ps only
                auto expanded = expand_history(line);
                if (!expanded) continue;
                if (*expanded != line) {
                    std::cout << *expanded << "\n";
                    line = *expanded;
                }
            }
            if (history.empty() || history.back() != line) {
                history.push_back(line);
                std::ofstream h(to_path(history_file()), std::ios::app | std::ios::binary);
                h << line << "\n";
            }
            dispatch(line);
        }
        return exiting ? exit_code : status;
    }

    std::optional<Mode> script_language(const std::string& path) {
        std::string ext = from_path(to_path(path).extension());
        for (auto& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (ext == ".pi") return Mode::Pi;
        if (ext == ".rho") return Mode::Rho;
        return std::nullopt;
    }

    // Like Console::ExecuteFile: '#' lines are comments, '$' lines are ps commands
    // (results onto the stack), and the code between them is evaluated as one block.
    int Shell::run_kai_file(Mode lang, const std::string& path) {
        if (!backend->has_languages()) {
            std::cerr << "kaish: " << path << ": " << mode_name(lang) << " needs kaish built inside CppKAI\n";
            return status = 1;
        }
        std::ifstream f(to_path(path), std::ios::binary);
        if (!f) {
            std::cerr << "kaish: " << path << ": No such file or directory\n";
            return status = 127;
        }
        bool failed = false;
        std::string block;
        auto flush = [&] {
            if (block.empty()) return;
            const std::string err = backend->eval(lang, block);
            block.clear();
            if (!err.empty()) {
                std::cerr << path << ": " << err;
                if (err.back() != '\n') std::cerr << '\n';
                failed = true;
            }
        };
        std::string line;
        bool first = true;
        while (std::getline(f, line) && !exiting) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (first && line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
            first = false;
            if (line.empty() || line[0] == '#') continue;
            if (line[0] == '$') {
                flush();
                if (execute(trim(line.substr(1)), true) != 0) failed = true;
            } else {
                block += line + "\n";
            }
        }
        flush();
        status = failed ? 1 : 0;
        return exiting ? exit_code : status;
    }

    int Shell::run_file(const std::string& path, const Args& args) {
        if (const auto lang = script_language(path)) return run_kai_file(*lang, path);
        std::ifstream f(to_path(path), std::ios::binary);
        if (!f) {
            std::cerr << "kaish: " << path << ": No such file or directory\n";
            return 127;
        }
        std::ostringstream ss;
        ss << f.rdbuf();
        Args saved = positional;
        positional = {path};
        positional.insert(positional.end(), args.begin(), args.end());
        execute(strip_bom(ss.str()));
        positional = std::move(saved);
        return exiting ? exit_code : status;
    }

}
