#include "kaish/platform.hpp"
#include "kaish/shell.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

namespace kai::kaish {

    namespace fs = std::filesystem;
    using platform::from_path;
    using platform::to_path;

    namespace {

        // ---------------------------------------------------------------- helpers

        struct Opts {
            std::string flags;
            std::map<char, std::string> values;
            std::vector<std::string> operands;
            bool has(char c) const { return flags.find(c) != std::string::npos; }
        };

        // getopt-style: "n:" means -n takes a value. "--" ends options; "-" is an operand.
        bool parse_opts(const Args& args, const std::string& spec, Opts& o, std::ostream& err) {
            bool only_operands = false;
            for (size_t i = 1; i < args.size(); ++i) {
                const std::string& a = args[i];
                if (only_operands || a.size() < 2 || a[0] != '-') { o.operands.push_back(a); continue; }
                if (a == "--") { only_operands = true; continue; }
                for (size_t k = 1; k < a.size(); ++k) {
                    const char c = a[k];
                    const size_t p = spec.find(c);
                    if (c == ':' || p == std::string::npos) {
                        err << args[0] << ": invalid option -- '" << c << "'\n";
                        return false;
                    }
                    o.flags += c;
                    if (p + 1 < spec.size() && spec[p + 1] == ':') {
                        if (k + 1 < a.size()) o.values[c] = a.substr(k + 1);
                        else if (i + 1 < args.size()) o.values[c] = args[++i];
                        else {
                            err << args[0] << ": option requires an argument -- '" << c << "'\n";
                            return false;
                        }
                        break;
                    }
                }
            }
            return true;
        }

        // head -5  ->  head -n 5
        Args numeric_shorthand(const Args& a) {
            Args r;
            for (size_t i = 0; i < a.size(); ++i) {
                const auto& s = a[i];
                if (i > 0 && s.size() > 1 && s[0] == '-' &&
                    std::all_of(s.begin() + 1, s.end(), [](char c) { return std::isdigit(static_cast<unsigned char>(c)); })) {
                    r.push_back("-n");
                    r.push_back(s.substr(1));
                } else {
                    r.push_back(s);
                }
            }
            return r;
        }

        std::optional<long> to_long(const std::string& s) {
            if (s.empty()) return std::nullopt;
            char* end = nullptr;
            const long v = std::strtol(s.c_str(), &end, 10);
            if (*end) return std::nullopt;
            return v;
        }

        std::string describe(const std::error_code& ec) {
            if (ec == std::errc::no_such_file_or_directory) return "No such file or directory";
            if (ec == std::errc::permission_denied) return "Permission denied";
            if (ec == std::errc::file_exists) return "File exists";
            if (ec == std::errc::directory_not_empty) return "Directory not empty";
            if (ec == std::errc::not_a_directory) return "Not a directory";
            if (ec == std::errc::is_a_directory) return "Is a directory";
            return ec.message();
        }

        std::string lower(std::string s) {
            for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return s;
        }

        size_t utf8_width(std::string_view s) {
            size_t n = 0;
            for (unsigned char c : s) {
                if ((c & 0xC0) != 0x80) ++n;
            }
            return n;
        }

        bool getline_nocr(std::istream& in, std::string& line) {
            if (!std::getline(in, line)) return false;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            return true;
        }

        // Runs fn over each named file, or stdin when there are none ("-" also means stdin).
        int with_inputs(const std::vector<std::string>& files, CommandIo& io, const std::string& cmd,
                        const std::function<void(std::istream&, const std::string&)>& fn) {
            if (files.empty()) {
                fn(io.in, "-");
                io.in.clear();
                return 0;
            }
            int rc = 0;
            for (const auto& f : files) {
                if (f == "-") {
                    fn(io.in, "-");
                    io.in.clear();
                    continue;
                }
                std::error_code ec;
                if (fs::is_directory(to_path(f), ec)) {
                    io.err << cmd << ": " << f << ": Is a directory\n";
                    rc = 1;
                    continue;
                }
                std::ifstream in(to_path(f), std::ios::binary);
                if (!in) {
                    io.err << cmd << ": " << f << ": No such file or directory\n";
                    rc = 1;
                    continue;
                }
                fn(in, f);
            }
            return rc;
        }

        fs::path leaf(const fs::path& p) {
            fs::path n = p;
            if (!n.has_filename() && n.has_parent_path()) n = n.parent_path();
            return n.filename();
        }

        void make_writable(const fs::path& p) {
            std::error_code ec;
            fs::permissions(p, fs::perms::owner_write, fs::perm_options::add, ec);
            if (!fs::is_directory(fs::symlink_status(p, ec))) return;
            for (fs::recursive_directory_iterator it(p, ec), end; !ec && it != end; it.increment(ec)) {
                std::error_code ignore;
                fs::permissions(it->path(), fs::perms::owner_write, fs::perm_options::add, ignore);
            }
        }

        // -------------------------------------------------------------------- ls

        struct Entry {
            std::string name;
            fs::path path;
            bool dir = false;
            bool link = false;
            bool exec = false;
            std::uintmax_t size = 0;
            std::time_t mtime = 0;
            fs::perms perms = fs::perms::none;
        };

        std::time_t to_time_t(fs::file_time_type ft) {
            using namespace std::chrono;
            const auto sys = time_point_cast<system_clock::duration>(ft - fs::file_time_type::clock::now() + system_clock::now());
            return system_clock::to_time_t(sys);
        }

        Entry make_entry(const fs::path& p, std::string name) {
            Entry e;
            e.name = std::move(name);
            e.path = p;
            std::error_code ec;
            e.link = fs::is_symlink(fs::symlink_status(p, ec));
            const auto st = fs::status(p, ec);
            e.dir = fs::is_directory(st);
            e.perms = st.permissions();
            if (!e.dir) {
                const auto sz = fs::file_size(p, ec);
                if (!ec) e.size = sz;
            }
            ec.clear();
            const auto t = fs::last_write_time(p, ec);
            if (!ec) e.mtime = to_time_t(t);
            e.exec = !e.dir && platform::is_executable(p);
            return e;
        }

        std::string perm_string(const Entry& e) {
            std::string s = e.link ? "l" : e.dir ? "d" : "-";
#ifdef _WIN32
            // Windows only has a read-only bit; derive the rest.
            const bool w = (e.perms & fs::perms::owner_write) != fs::perms::none;
            const bool x = e.dir || e.exec;
            for (int i = 0; i < 3; ++i) {
                s += 'r';
                s += w ? 'w' : '-';
                s += x ? 'x' : '-';
            }
#else
            using P = fs::perms;
            const P bits[] = {P::owner_read, P::owner_write, P::owner_exec, P::group_read, P::group_write,
                              P::group_exec, P::others_read, P::others_write, P::others_exec};
            const char* chars = "rwxrwxrwx";
            for (int i = 0; i < 9; ++i) s += (e.perms & bits[i]) != P::none ? chars[i] : '-';
#endif
            return s;
        }

        std::string human(std::uintmax_t n) {
            const char* units = "BKMGTP";
            double v = static_cast<double>(n);
            int i = 0;
            while (v >= 1024 && i < 5) { v /= 1024; ++i; }
            if (i == 0) return std::to_string(n);
            char buf[32];
            std::snprintf(buf, sizeof buf, v < 10 ? "%.1f%c" : "%.0f%c", v, units[i]);
            return buf;
        }

        std::string date_string(std::time_t t) {
            std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm, &t);
#else
            localtime_r(&t, &tm);
#endif
            const std::time_t now = std::time(nullptr);
            const bool recent = now - t < 60L * 60 * 24 * 182 && t <= now + 3600;
            char buf[64];
            std::strftime(buf, sizeof buf, recent ? "%b %e %H:%M" : "%b %e  %Y", &tm);
            return buf;
        }

        std::string suffix(const Entry& e, bool classify) {
            if (!classify) return "";
            if (e.link) return "@";
            if (e.dir) return "/";
            if (e.exec) return "*";
            return "";
        }

        std::string colored(const Entry& e, bool color) {
            if (!color) return e.name;
            if (e.link) return "\x1b[1;36m" + e.name + "\x1b[0m";
            if (e.dir) return "\x1b[1;34m" + e.name + "\x1b[0m";
            if (e.exec) return "\x1b[1;32m" + e.name + "\x1b[0m";
            return e.name;
        }

        // Fills columns top-to-bottom like GNU ls.
        void print_columns(std::ostream& out, const std::vector<std::string>& plain, const std::vector<std::string>& shown, int width) {
            const size_t n = plain.size();
            if (n == 0) return;
            std::vector<size_t> w(n);
            for (size_t i = 0; i < n; ++i) w[i] = utf8_width(plain[i]);
            size_t rows = n;
            std::vector<size_t> col_w;
            for (size_t cols = std::min(n, static_cast<size_t>(std::max(1, width / 3))); cols >= 1; --cols) {
                const size_t r = (n + cols - 1) / cols;
                const size_t real = (n + r - 1) / r;
                std::vector<size_t> cw(real, 0);
                for (size_t i = 0; i < n; ++i) cw[i / r] = std::max(cw[i / r], w[i]);
                size_t total = 0;
                for (size_t c = 0; c < real; ++c) total += cw[c] + (c + 1 < real ? 2 : 0);
                if (total <= static_cast<size_t>(width) || cols == 1) {
                    rows = r;
                    col_w = std::move(cw);
                    break;
                }
            }
            for (size_t r = 0; r < rows; ++r) {
                for (size_t c = 0; c < col_w.size(); ++c) {
                    const size_t i = c * rows + r;
                    if (i >= n) break;
                    out << shown[i];
                    if (i + rows < n) out << std::string(col_w[c] - w[i] + 2, ' ');
                }
                out << '\n';
            }
        }

        int b_ls(Shell&, const Args& a, CommandIo& io) {
            Opts o;
            if (!parse_opts(a, "aAlh1rtSdFC", o, io.err)) return 2;
            const bool all = o.has('a');
            const bool almost = o.has('A');
            const bool lng = o.has('l');
            const bool one = o.has('1') || (!io.out_is_terminal && !o.has('C'));
            const bool color = io.out_is_terminal;
            const bool classify = o.has('F');
            if (o.operands.empty()) o.operands.push_back(".");

            int rc = 0;
            std::vector<Entry> files;
            std::vector<std::pair<std::string, fs::path>> dirs;
            for (const auto& op : o.operands) {
                const fs::path p = to_path(op);
                std::error_code ec;
                const auto st = fs::status(p, ec);
                if (!fs::exists(st)) {
                    io.err << "ls: cannot access '" << op << "': No such file or directory\n";
                    rc = 2;
                    continue;
                }
                if (fs::is_directory(st) && !o.has('d')) dirs.emplace_back(op, p);
                else files.push_back(make_entry(p, op));
            }

            auto print = [&](std::vector<Entry>& v, bool total) {
                std::sort(v.begin(), v.end(), [&](const Entry& x, const Entry& y) {
                    if (o.has('t') && x.mtime != y.mtime) return x.mtime > y.mtime;
                    if (o.has('S') && x.size != y.size) return x.size > y.size;
                    const auto lx = lower(x.name), ly = lower(y.name);
                    return lx != ly ? lx < ly : x.name < y.name;
                });
                if (o.has('r')) std::reverse(v.begin(), v.end());

                std::vector<std::string> long_lines;
                if (lng) {
                    std::vector<std::string> sizes;
                    size_t sw = 0;
                    for (const auto& e : v) {
                        sizes.push_back(o.has('h') ? human(e.size) : std::to_string(e.size));
                        sw = std::max(sw, sizes.back().size());
                    }
                    for (size_t i = 0; i < v.size(); ++i) {
                        const Entry& e = v[i];
                        std::ostringstream line;
                        line << perm_string(e) << ' ' << std::setw(static_cast<int>(sw)) << sizes[i] << ' '
                             << date_string(e.mtime) << ' ' << colored(e, color) << suffix(e, classify);
                        if (e.link) {
                            std::error_code ec;
                            const auto target = fs::read_symlink(e.path, ec);
                            if (!ec) line << " -> " << from_path(target);
                        }
                        long_lines.push_back(line.str());
                    }
                }

                if (io.stack) {   // one stack entry per directory entry
                    if (lng) {
                        for (const auto& l : long_lines) io.stack->push_text(l);
                    } else {
                        for (const auto& e : v) io.stack->push_text(e.name + suffix(e, classify));
                    }
                    return;
                }

                if (lng) {
                    if (total) {
                        std::uintmax_t blocks = 0;
                        for (const auto& e : v) blocks += (e.size + 1023) / 1024;
                        io.out << "total " << blocks << '\n';
                    }
                    for (const auto& l : long_lines) io.out << l << '\n';
                } else if (one) {
                    for (const auto& e : v) io.out << colored(e, color) << suffix(e, classify) << '\n';
                } else {
                    std::vector<std::string> plain, shown;
                    for (const auto& e : v) {
                        plain.push_back(e.name + suffix(e, classify));
                        shown.push_back(colored(e, color) + suffix(e, classify));
                    }
                    print_columns(io.out, plain, shown, platform::terminal_width());
                }
            };

            if (!files.empty()) print(files, false);
            for (size_t i = 0; i < dirs.size(); ++i) {
                const auto& [label, p] = dirs[i];
                std::vector<Entry> v;
                if (all) {
                    v.push_back(make_entry(p, "."));
                    v.push_back(make_entry(p / "..", ".."));
                }
                std::error_code ec;
                fs::directory_iterator it(p, fs::directory_options::skip_permission_denied, ec);
                if (ec) {
                    io.err << "ls: cannot open directory '" << label << "': " << describe(ec) << '\n';
                    rc = 2;
                    continue;
                }
                for (fs::directory_iterator end; it != end; it.increment(ec)) {
                    if (ec) break;
                    const std::string name = from_path(it->path().filename());
                    if (!all && !almost && platform::is_hidden(it->path(), name)) continue;
                    v.push_back(make_entry(it->path(), name));
                }
                if (!io.stack && (dirs.size() > 1 || !files.empty())) {
                    if (i > 0 || !files.empty()) io.out << '\n';
                    io.out << label << ":\n";
                }
                print(v, true);
            }
            return rc;
        }

        // ------------------------------------------------------- navigation, env

        int b_cd(Shell& sh, const Args& a, CommandIo& io) {
            if (a.size() > 2) {
                io.err << "kaish: cd: too many arguments\n";
                return 1;
            }
            std::string target;
            if (a.size() < 2) {
                target = from_path(platform::home_dir());
            } else if (a[1] == "-") {
                const auto old = sh.get_var("OLDPWD");
                if (!old) {
                    io.err << "kaish: cd: OLDPWD not set\n";
                    return 1;
                }
                target = *old;
                io.out << target << '\n';
            } else {
                target = a[1];
            }
            std::error_code ec;
            const auto old = fs::current_path(ec);
            fs::current_path(to_path(target), ec);
            if (ec) {
                io.err << "kaish: cd: " << target << ": " << describe(ec) << '\n';
                return 1;
            }
            sh.export_var("OLDPWD", from_path(old));
            sh.export_var("PWD", from_path(fs::current_path(ec)));
            return 0;
        }

        int b_pwd(Shell&, const Args&, CommandIo& io) {
            io.out << from_path(fs::current_path()) << '\n';
            return 0;
        }

        std::string c_escapes(const std::string& s, bool& stop) {
            std::string r;
            for (size_t i = 0; i < s.size(); ++i) {
                if (s[i] != '\\' || i + 1 >= s.size()) { r += s[i]; continue; }
                switch (s[++i]) {
                    case 'n': r += '\n'; break;
                    case 't': r += '\t'; break;
                    case 'r': r += '\r'; break;
                    case 'a': r += '\a'; break;
                    case 'b': r += '\b'; break;
                    case 'e': r += '\x1b'; break;
                    case '\\': r += '\\'; break;
                    case 'c': stop = true; return r;
                    default: r += '\\'; r += s[i]; break;
                }
            }
            return r;
        }

        int b_echo(Shell&, const Args& a, CommandIo& io) {
            bool newline = true, escapes = false;
            size_t i = 1;
            for (; i < a.size(); ++i) {
                const auto& s = a[i];
                if (s.size() < 2 || s[0] != '-' || s.find_first_not_of("neE", 1) != std::string::npos) break;
                for (char c : s.substr(1)) {
                    if (c == 'n') newline = false;
                    else escapes = c == 'e';
                }
            }
            std::string text;
            bool stop = false;
            for (size_t k = i; k < a.size() && !stop; ++k) {
                if (k > i) text += ' ';
                text += escapes ? c_escapes(a[k], stop) : a[k];
            }
            io.out << text;
            if (newline && !stop) io.out << '\n';
            return 0;
        }

        int b_export(Shell& sh, const Args& a, CommandIo& io) {
            if (a.size() == 1 || a[1] == "-p") {
                auto env = platform::environment();
                std::sort(env.begin(), env.end());
                for (const auto& [k, v] : env) io.out << "declare -x " << k << "=\"" << v << "\"\n";
                return 0;
            }
            int rc = 0;
            for (size_t i = 1; i < a.size(); ++i) {
                const auto eq = a[i].find('=');
                const std::string name = a[i].substr(0, eq);
                if (!is_valid_name(name)) {
                    io.err << "kaish: export: `" << a[i] << "': not a valid identifier\n";
                    rc = 1;
                    continue;
                }
                if (eq != std::string::npos) sh.export_var(name, a[i].substr(eq + 1));
                else if (auto it = sh.vars.find(name); it != sh.vars.end()) sh.export_var(name, it->second);
            }
            return rc;
        }

        int b_unset(Shell& sh, const Args& a, CommandIo&) {
            for (size_t i = 1; i < a.size(); ++i) sh.unset_var(a[i]);
            return 0;
        }

        int b_env(Shell&, const Args&, CommandIo& io) {
            auto env = platform::environment();
            std::sort(env.begin(), env.end());
            for (const auto& [k, v] : env) io.out << k << '=' << v << '\n';
            return 0;
        }

        int b_set(Shell& sh, const Args&, CommandIo& io) {
            for (const auto& [k, v] : sh.vars) io.out << k << '=' << v << '\n';
            return 0;
        }

        int b_alias(Shell& sh, const Args& a, CommandIo& io) {
            if (a.size() == 1) {
                for (const auto& [k, v] : sh.aliases) io.out << "alias " << k << "='" << v << "'\n";
                return 0;
            }
            int rc = 0;
            for (size_t i = 1; i < a.size(); ++i) {
                const auto eq = a[i].find('=');
                if (eq != std::string::npos) {
                    sh.aliases[a[i].substr(0, eq)] = a[i].substr(eq + 1);
                } else if (auto it = sh.aliases.find(a[i]); it != sh.aliases.end()) {
                    io.out << "alias " << it->first << "='" << it->second << "'\n";
                } else {
                    io.err << "kaish: alias: " << a[i] << ": not found\n";
                    rc = 1;
                }
            }
            return rc;
        }

        int b_unalias(Shell& sh, const Args& a, CommandIo& io) {
            if (a.size() > 1 && a[1] == "-a") {
                sh.aliases.clear();
                return 0;
            }
            int rc = 0;
            for (size_t i = 1; i < a.size(); ++i) {
                if (sh.aliases.erase(a[i]) == 0) {
                    io.err << "kaish: unalias: " << a[i] << ": not found\n";
                    rc = 1;
                }
            }
            return rc;
        }

        int b_history(Shell& sh, const Args& a, CommandIo& io) {
            if (a.size() > 1 && a[1] == "-c") {
                sh.history.clear();
                std::ofstream truncate(to_path(sh.history_file()), std::ios::trunc);
                return 0;
            }
            size_t start = 0;
            if (a.size() > 1) {
                const auto n = to_long(a[1]);
                if (!n || *n < 0) {
                    io.err << "kaish: history: " << a[1] << ": numeric argument required\n";
                    return 1;
                }
                if (static_cast<size_t>(*n) < sh.history.size()) start = sh.history.size() - static_cast<size_t>(*n);
            }
            for (size_t i = start; i < sh.history.size(); ++i) io.out << std::setw(5) << i + 1 << "  " << sh.history[i] << '\n';
            return 0;
        }

        int b_type(Shell& sh, const Args& a, CommandIo& io) {
            int rc = 0;
            for (size_t i = 1; i < a.size(); ++i) {
                const auto& n = a[i];
                if (auto it = sh.aliases.find(n); it != sh.aliases.end()) io.out << n << " is aliased to `" << it->second << "'\n";
                else if (sh.builtins.count(n)) io.out << n << " is a shell builtin\n";
                else if (auto p = platform::find_executable(n)) io.out << n << " is " << from_path(*p) << '\n';
                else {
                    io.err << "kaish: type: " << n << ": not found\n";
                    rc = 1;
                }
            }
            return rc;
        }

        int b_which(Shell& sh, const Args& a, CommandIo& io) {
            int rc = 0;
            for (size_t i = 1; i < a.size(); ++i) {
                if (auto p = platform::find_executable(a[i])) io.out << from_path(*p) << '\n';
                else if (sh.builtins.count(a[i])) io.out << a[i] << ": shell builtin\n";
                else rc = 1;
            }
            return rc;
        }

        int b_source(Shell& sh, const Args& a, CommandIo& io) {
            if (a.size() < 2) {
                io.err << "kaish: " << a[0] << ": filename argument required\n";
                return 2;
            }
            return sh.run_file(a[1], Args(a.begin() + 2, a.end()));
        }

        int b_exit(Shell& sh, const Args& a, CommandIo& io) {
            int code = sh.status;
            if (a.size() > 1) {
                const auto n = to_long(a[1]);
                if (!n) {
                    io.err << "kaish: exit: " << a[1] << ": numeric argument required\n";
                    code = 2;
                } else {
                    code = static_cast<int>(*n);
                }
            }
            sh.exiting = true;
            sh.exit_code = code;
            return code;
        }

        int b_clear(Shell&, const Args&, CommandIo& io) {
            io.out << "\x1b[2J\x1b[3J\x1b[H";
            return 0;
        }

        // ------------------------------------------------------------- stack

        int b_drop(Shell& sh, const Args& a, CommandIo& io) {
            long n = 1;
            if (a.size() > 1) {
                const auto v = to_long(a[1]);
                if (!v || *v < 0) {
                    io.err << "drop: " << a[1] << ": numeric argument required\n";
                    return 1;
                }
                n = *v;
            }
            for (long i = 0; i < n; ++i) sh.stack().drop();
            return 0;
        }

        int b_dup(Shell& sh, const Args&, CommandIo&) { sh.stack().dup(); return 0; }
        int b_swap(Shell& sh, const Args&, CommandIo&) { sh.stack().swap(); return 0; }
        int b_clr(Shell& sh, const Args&, CommandIo&) { sh.stack().clear(); return 0; }

        int b_help(Shell& sh, const Args&, CommandIo& io) {
            io.out << "kaish - KAI Object Shell\n\n"
                      "Modes (all share one value stack, shown after every command):\n"
                      "  ps                              this shell; results go onto the stack\n"
                      "  pi                              KAI Pi (RPN)\n"
                      "  rho                             KAI Rho (infix)\n"
                      "  pi 1 2 +    rho 1 + 2           run one line in another mode\n"
                      "  $ ls                            run a ps line from pi or rho\n"
                      "  ps aux                          'ps' with arguments is the program, in any mode\n"
                      "  kaish file.pi / source f.rho    run a Pi or Rho script ('$' lines are ps)\n\n"
                      "Stack:  @1 @2 ...  level N as an argument, e.g. cp @2 @1 ('@1' is literal)\n"
                      "        drop [n]  dup  swap  clr       ~/.kaish.json sets stack_levels, mode,\n"
                      "        term CMD  (output to screen)   show_stack and passthrough programs\n\n"
                      "  cmd args | cmd > file 2>&1      pipes and redirection (<, >, >>, 2>, 2>&1, &>)\n"
                      "  a ; b    a && b    a || b       sequencing\n"
                      "  $VAR ${VAR} $? $$ $1 $#         variables; NAME=value, export, unset\n"
                      "  $(cmd) `cmd`                    command substitution\n"
                      "  ~  *  ?  [a-z]                  home and globbing\n"
                      "  'literal'  \"with $vars\"  \\x    quoting\n"
                      "  !! !n                           history\n"
                      "\n"
                      "Programs on PATH are run directly (.bat/.cmd via cmd.exe, .ps1 via pwsh).\n"
                      "'command NAME' skips builtins and aliases. ~/.kaishrc is sourced at startup.\n\n"
                      "Builtins:\n";
            std::vector<std::string> names;
            for (const auto& [k, v] : sh.builtins) names.push_back(k);
            print_columns(io.out, names, names, 72);
            return 0;
        }

        // ---------------------------------------------------------- file builtins

        int b_cat(Shell&, const Args& a, CommandIo& io) {
            Opts o;
            if (!parse_opts(a, "n", o, io.err)) return 1;
            size_t lineno = 1;
            return with_inputs(o.operands, io, "cat", [&](std::istream& in, const std::string&) {
                if (o.has('n')) {
                    std::string line;
                    while (std::getline(in, line)) io.out << std::setw(6) << lineno++ << '\t' << line << '\n';
                    return;
                }
                char buf[65536];
                while (in.read(buf, sizeof buf) || in.gcount() > 0) io.out.write(buf, in.gcount());
            });
        }

        int b_mkdir(Shell&, const Args& a, CommandIo& io) {
            Opts o;
            if (!parse_opts(a, "pv", o, io.err)) return 1;
            if (o.operands.empty()) {
                io.err << "mkdir: missing operand\n";
                return 1;
            }
            int rc = 0;
            for (const auto& op : o.operands) {
                std::error_code ec;
                if (o.has('p')) {
                    fs::create_directories(to_path(op), ec);
                } else if (!fs::create_directory(to_path(op), ec) && !ec) {
                    ec = std::make_error_code(std::errc::file_exists);
                }
                if (ec) {
                    io.err << "mkdir: cannot create directory '" << op << "': " << describe(ec) << '\n';
                    rc = 1;
                } else if (o.has('v')) {
                    io.out << "mkdir: created directory '" << op << "'\n";
                }
            }
            return rc;
        }

        int b_rmdir(Shell&, const Args& a, CommandIo& io) {
            int rc = 0;
            for (size_t i = 1; i < a.size(); ++i) {
                const fs::path p = to_path(a[i]);
                std::error_code ec;
                if (!fs::is_directory(p, ec)) ec = std::make_error_code(fs::exists(p, ec) ? std::errc::not_a_directory : std::errc::no_such_file_or_directory);
                else fs::remove(p, ec);
                if (ec) {
                    io.err << "rmdir: failed to remove '" << a[i] << "': " << describe(ec) << '\n';
                    rc = 1;
                }
            }
            return rc;
        }

        int b_rm(Shell&, const Args& a, CommandIo& io) {
            Opts o;
            if (!parse_opts(a, "rRfv", o, io.err)) return 1;
            const bool recursive = o.has('r') || o.has('R');
            const bool force = o.has('f');
            if (o.operands.empty()) {
                if (force) return 0;
                io.err << "rm: missing operand\n";
                return 1;
            }
            int rc = 0;
            for (const auto& op : o.operands) {
                const fs::path p = to_path(op);
                std::error_code ec;
                const auto st = fs::symlink_status(p, ec);
                if (!fs::exists(st)) {
                    if (!force) {
                        io.err << "rm: cannot remove '" << op << "': No such file or directory\n";
                        rc = 1;
                    }
                    continue;
                }
                if (fs::is_directory(st)) {
                    if (!recursive) {
                        io.err << "rm: cannot remove '" << op << "': Is a directory\n";
                        rc = 1;
                        continue;
                    }
                    const fs::path abs = fs::absolute(p, ec).lexically_normal();
                    const fs::path rel = abs.relative_path();
                    if (rel.empty() || rel == "." || abs == fs::absolute(platform::home_dir(), ec).lexically_normal()) {
                        io.err << "rm: refusing to remove '" << op << "'\n";
                        rc = 1;
                        continue;
                    }
                    fs::remove_all(p, ec);
                    if (ec && force) { // read-only files (e.g. .git objects on Windows)
                        ec.clear();
                        make_writable(p);
                        fs::remove_all(p, ec);
                    }
                } else {
                    fs::remove(p, ec);
                    if (ec && force) {
                        ec.clear();
                        make_writable(p);
                        fs::remove(p, ec);
                    }
                }
                if (ec) {
                    io.err << "rm: cannot remove '" << op << "': " << describe(ec) << '\n';
                    rc = 1;
                } else if (o.has('v')) {
                    io.out << "removed '" << op << "'\n";
                }
            }
            return rc;
        }

        int copy_or_move(const Args& a, CommandIo& io, bool move) {
            const char* name = move ? "mv" : "cp";
            Opts o;
            if (!parse_opts(a, "rRfv", o, io.err)) return 1;
            if (o.operands.size() < 2) {
                io.err << name << ": missing destination file operand\n";
                return 1;
            }
            const fs::path dest = to_path(o.operands.back());
            std::error_code ec;
            const bool dest_dir = fs::is_directory(dest, ec);
            if (o.operands.size() > 2 && !dest_dir) {
                io.err << name << ": target '" << o.operands.back() << "' is not a directory\n";
                return 1;
            }
            int rc = 0;
            for (size_t i = 0; i + 1 < o.operands.size(); ++i) {
                const auto& op = o.operands[i];
                const fs::path src = to_path(op);
                ec.clear();
                const auto st = fs::status(src, ec);
                if (!fs::exists(st)) {
                    io.err << name << ": cannot stat '" << op << "': No such file or directory\n";
                    rc = 1;
                    continue;
                }
                const fs::path target = dest_dir ? dest / leaf(src) : dest;
                ec.clear();
                if (move) {
                    fs::rename(src, target, ec);
                    if (ec == std::errc::cross_device_link) {
                        ec.clear();
                        fs::copy(src, target, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
                        if (!ec) fs::remove_all(src, ec);
                    }
                } else if (fs::is_directory(st)) {
                    if (!o.has('r') && !o.has('R')) {
                        io.err << "cp: -r not specified; omitting directory '" << op << "'\n";
                        rc = 1;
                        continue;
                    }
                    fs::copy(src, target, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
                } else {
                    fs::copy_file(src, target, fs::copy_options::overwrite_existing, ec);
                }
                if (ec) {
                    io.err << name << ": cannot " << (move ? "move" : "copy") << " '" << op << "': " << describe(ec) << '\n';
                    rc = 1;
                } else if (o.has('v')) {
                    io.out << "'" << op << "' -> '" << from_path(target) << "'\n";
                }
            }
            return rc;
        }

        int b_cp(Shell&, const Args& a, CommandIo& io) { return copy_or_move(a, io, false); }
        int b_mv(Shell&, const Args& a, CommandIo& io) { return copy_or_move(a, io, true); }

        int b_touch(Shell&, const Args& a, CommandIo& io) {
            int rc = 0;
            for (size_t i = 1; i < a.size(); ++i) {
                const fs::path p = to_path(a[i]);
                std::error_code ec;
                if (fs::exists(p, ec)) {
                    fs::last_write_time(p, fs::file_time_type::clock::now(), ec);
                } else {
                    std::ofstream f(p, std::ios::app);
                    if (!f) ec = std::make_error_code(std::errc::no_such_file_or_directory);
                }
                if (ec) {
                    io.err << "touch: cannot touch '" << a[i] << "': " << describe(ec) << '\n';
                    rc = 1;
                }
            }
            return rc;
        }

        // --------------------------------------------------------- text builtins

        int head_or_tail(const Args& a0, CommandIo& io, bool tail) {
            const char* name = tail ? "tail" : "head";
            Opts o;
            if (!parse_opts(numeric_shorthand(a0), "n:", o, io.err)) return 1;
            long n = 10;
            if (o.values.count('n')) {
                const auto v = to_long(o.values['n']);
                if (!v || *v < 0) {
                    io.err << name << ": invalid number of lines: '" << o.values['n'] << "'\n";
                    return 1;
                }
                n = *v;
            }
            const bool multi = o.operands.size() > 1;
            bool first = true;
            return with_inputs(o.operands, io, name, [&](std::istream& in, const std::string& label) {
                if (multi) {
                    if (!first) io.out << '\n';
                    io.out << "==> " << label << " <==\n";
                }
                first = false;
                std::string line;
                if (!tail) {
                    for (long i = 0; i < n && getline_nocr(in, line); ++i) io.out << line << '\n';
                    return;
                }
                std::deque<std::string> q;
                while (getline_nocr(in, line)) {
                    q.push_back(line);
                    if (static_cast<long>(q.size()) > n) q.pop_front();
                }
                for (const auto& l : q) io.out << l << '\n';
            });
        }

        int b_head(Shell&, const Args& a, CommandIo& io) { return head_or_tail(a, io, false); }
        int b_tail(Shell&, const Args& a, CommandIo& io) { return head_or_tail(a, io, true); }

        int b_wc(Shell&, const Args& a, CommandIo& io) {
            Opts o;
            if (!parse_opts(a, "lwc", o, io.err)) return 1;
            bool l = o.has('l'), w = o.has('w'), c = o.has('c');
            if (!l && !w && !c) l = w = c = true;
            const int fields = l + w + c;
            const bool pad = fields > 1 || o.operands.size() > 1;
            size_t tl = 0, tw = 0, tc = 0;
            int inputs = 0;
            auto print = [&](size_t L, size_t W, size_t C, const std::string& label) {
                bool first = true;
                auto put = [&](size_t v) {
                    if (!first) io.out << ' ';
                    first = false;
                    if (pad) io.out << std::setw(7);
                    io.out << v;
                };
                if (l) put(L);
                if (w) put(W);
                if (c) put(C);
                if (label != "-") io.out << ' ' << label;
                io.out << '\n';
            };
            const int rc = with_inputs(o.operands, io, "wc", [&](std::istream& in, const std::string& label) {
                size_t L = 0, W = 0, C = 0;
                bool in_word = false;
                char buf[65536];
                while (in.read(buf, sizeof buf) || in.gcount() > 0) {
                    const auto n = in.gcount();
                    for (std::streamsize k = 0; k < n; ++k) {
                        const char ch = buf[k];
                        ++C;
                        if (ch == '\n') ++L;
                        const bool space = std::isspace(static_cast<unsigned char>(ch)) != 0;
                        if (!space && !in_word) ++W;
                        in_word = !space;
                    }
                }
                print(L, W, C, label);
                tl += L;
                tw += W;
                tc += C;
                ++inputs;
            });
            if (inputs > 1) print(tl, tw, tc, "total");
            return rc;
        }

        int b_grep(Shell&, const Args& a, CommandIo& io) {
            Opts o;
            if (!parse_opts(a, "ivnclEFHh", o, io.err)) return 2;
            if (o.operands.empty()) {
                io.err << "usage: grep [-ivnclFHh] PATTERN [FILE]...\n";
                return 2;
            }
            std::string pattern = o.operands[0];
            if (o.has('F')) {
                std::string escaped;
                for (char ch : pattern) {
                    if (std::string_view("\\^$.|?*+()[]{}").find(ch) != std::string_view::npos) escaped += '\\';
                    escaped += ch;
                }
                pattern = escaped;
            }
            std::regex re;
            try {
                auto flags = std::regex::ECMAScript;
                if (o.has('i')) flags |= std::regex::icase;
                re = std::regex(pattern, flags);
            } catch (const std::regex_error& e) {
                io.err << "grep: invalid pattern: " << e.what() << '\n';
                return 2;
            }
            const std::vector<std::string> files(o.operands.begin() + 1, o.operands.end());
            const bool label_lines = !o.has('h') && (o.has('H') || files.size() > 1);
            const bool invert = o.has('v');
            const bool highlight = io.out_is_terminal && !invert;
            bool any = false;
            const int rc = with_inputs(files, io, "grep", [&](std::istream& in, const std::string& label) {
                std::string line;
                size_t lineno = 0, count = 0;
                const std::string shown_label = label == "-" ? "(standard input)" : label;
                while (getline_nocr(in, line)) {
                    ++lineno;
                    if (std::regex_search(line, re) == invert) continue;
                    ++count;
                    any = true;
                    if (o.has('c') || o.has('l')) continue;
                    if (label_lines) io.out << (highlight ? "\x1b[35m" + shown_label + "\x1b[36m:\x1b[0m" : shown_label + ":");
                    if (o.has('n')) io.out << (highlight ? "\x1b[32m" + std::to_string(lineno) + "\x1b[36m:\x1b[0m" : std::to_string(lineno) + ":");
                    if (highlight) {
                        size_t last = 0;
                        for (std::sregex_iterator it(line.begin(), line.end(), re), end; it != end; ++it) {
                            if (it->length() == 0) continue;
                            const size_t pos = static_cast<size_t>(it->position());
                            io.out << line.substr(last, pos - last) << "\x1b[1;31m" << it->str() << "\x1b[0m";
                            last = pos + static_cast<size_t>(it->length());
                        }
                        io.out << line.substr(last) << '\n';
                    } else {
                        io.out << line << '\n';
                    }
                }
                if (o.has('c')) {
                    if (label_lines) io.out << shown_label << ':';
                    io.out << count << '\n';
                }
                if (o.has('l') && count) io.out << shown_label << '\n';
            });
            if (rc) return 2;
            return any ? 0 : 1;
        }

        int b_sort(Shell&, const Args& a, CommandIo& io) {
            Opts o;
            if (!parse_opts(a, "rnuf", o, io.err)) return 2;
            std::vector<std::string> lines;
            const int rc = with_inputs(o.operands, io, "sort", [&](std::istream& in, const std::string&) {
                std::string line;
                while (getline_nocr(in, line)) lines.push_back(line);
            });
            if (o.has('n')) {
                std::stable_sort(lines.begin(), lines.end(), [](const std::string& x, const std::string& y) {
                    return std::strtod(x.c_str(), nullptr) < std::strtod(y.c_str(), nullptr);
                });
            } else if (o.has('f')) {
                std::stable_sort(lines.begin(), lines.end(), [](const std::string& x, const std::string& y) { return lower(x) < lower(y); });
            } else {
                std::sort(lines.begin(), lines.end());
            }
            if (o.has('r')) std::reverse(lines.begin(), lines.end());
            if (o.has('u')) lines.erase(std::unique(lines.begin(), lines.end()), lines.end());
            for (const auto& l : lines) io.out << l << '\n';
            return rc;
        }

    }

    void register_builtins(Shell& sh) {
        auto& b = sh.builtins;
        b["."] = b_source;
        b[":"] = [](Shell&, const Args&, CommandIo&) { return 0; };
        b["alias"] = b_alias;
        b["cat"] = b_cat;
        b["cd"] = b_cd;
        b["clear"] = b_clear;
        b["command"] = [](Shell&, const Args&, CommandIo&) { return 0; };
        b["clr"] = b_clr;
        b["cp"] = b_cp;
        b["drop"] = b_drop;
        b["dup"] = b_dup;
        b["echo"] = b_echo;
        b["env"] = b_env;
        b["exit"] = b_exit;
        b["export"] = b_export;
        b["false"] = [](Shell&, const Args&, CommandIo&) { return 1; };
        b["grep"] = b_grep;
        b["head"] = b_head;
        b["help"] = b_help;
        b["history"] = b_history;
        b["ls"] = b_ls;
        b["mkdir"] = b_mkdir;
        b["mv"] = b_mv;
        b["pwd"] = b_pwd;
        b["rm"] = b_rm;
        b["rmdir"] = b_rmdir;
        b["set"] = b_set;
        b["sort"] = b_sort;
        b["swap"] = b_swap;
        b["source"] = b_source;
        b["tail"] = b_tail;
        b["term"] = [](Shell&, const Args&, CommandIo&) { return 0; };
        b["touch"] = b_touch;
        b["true"] = [](Shell&, const Args&, CommandIo&) { return 0; };
        b["type"] = b_type;
        b["unalias"] = b_unalias;
        b["unset"] = b_unset;
        b["wc"] = b_wc;
        b["which"] = b_which;
    }

}
