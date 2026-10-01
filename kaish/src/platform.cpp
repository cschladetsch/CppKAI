#include "kaish/platform.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <cwctype>
#else
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

namespace kai::kaish::platform {

#ifdef _WIN32

    namespace {

        std::wstring widen(std::string_view s) {
            if (s.empty()) return {};
            const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
            std::wstring w(static_cast<size_t>(n), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), w.data(), n);
            return w;
        }

        std::string narrow(std::wstring_view w) {
            if (w.empty()) return {};
            const int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
            std::string s(static_cast<size_t>(n), '\0');
            WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), s.data(), n, nullptr, nullptr);
            return s;
        }

        std::wstring lower(std::wstring s) {
            for (auto& c : s) c = static_cast<wchar_t>(std::towlower(c));
            return s;
        }

        std::vector<std::wstring> executable_extensions() {
            const std::wstring raw = widen(get_env("PATHEXT").value_or(".COM;.EXE;.BAT;.CMD"));
            std::vector<std::wstring> exts;
            size_t start = 0;
            while (start <= raw.size()) {
                size_t end = raw.find(L';', start);
                if (end == std::wstring::npos) end = raw.size();
                if (end > start) exts.push_back(lower(raw.substr(start, end - start)));
                start = end + 1;
            }
            if (std::find(exts.begin(), exts.end(), L".ps1") == exts.end()) exts.push_back(L".ps1");
            return exts;
        }

        bool is_file(const fs::path& p) {
            const DWORD a = GetFileAttributesW(p.c_str());
            return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
        }

        // Standard MSVC CRT argv quoting.
        std::wstring quote_arg(const std::wstring& a) {
            if (!a.empty() && a.find_first_of(L" \t\n\v\"") == std::wstring::npos) return a;
            std::wstring r = L"\"";
            for (auto it = a.begin();; ++it) {
                size_t backslashes = 0;
                while (it != a.end() && *it == L'\\') { ++it; ++backslashes; }
                if (it == a.end()) { r.append(backslashes * 2, L'\\'); break; }
                if (*it == L'"') { r.append(backslashes * 2 + 1, L'\\'); r.push_back(L'"'); }
                else { r.append(backslashes, L'\\'); r.push_back(*it); }
            }
            r.push_back(L'"');
            return r;
        }

        BOOL WINAPI ctrl_handler(DWORD type) {
            return type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT;
        }

    }

    fs::path to_path(const std::string& s) { return fs::path(widen(s)); }
    std::string from_path(const fs::path& p) { return narrow(p.wstring()); }

    std::vector<std::string> command_line_args(int argc, char** argv) {
        int n = 0;
        LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &n);
        if (!wargv) return std::vector<std::string>(argv, argv + argc);
        std::vector<std::string> args;
        for (int i = 0; i < n; ++i) args.push_back(narrow(wargv[i]));
        LocalFree(wargv);
        return args;
    }

    std::optional<std::string> get_env(const std::string& name) {
        const std::wstring w = widen(name);
        SetLastError(0);
        DWORD n = GetEnvironmentVariableW(w.c_str(), nullptr, 0);
        if (n == 0) return std::nullopt;
        std::wstring v(n, L'\0');
        n = GetEnvironmentVariableW(w.c_str(), v.data(), n);
        v.resize(n);
        return narrow(v);
    }

    void set_env(const std::string& name, const std::string& value) {
        SetEnvironmentVariableW(widen(name).c_str(), widen(value).c_str());
    }

    void unset_env(const std::string& name) {
        SetEnvironmentVariableW(widen(name).c_str(), nullptr);
    }

    std::vector<std::pair<std::string, std::string>> environment() {
        std::vector<std::pair<std::string, std::string>> env;
        LPWCH block = GetEnvironmentStringsW();
        if (!block) return env;
        for (const wchar_t* p = block; *p; p += wcslen(p) + 1) {
            std::wstring_view e(p);
            if (e.empty() || e[0] == L'=') continue; // per-drive cwd entries like =C:
            const size_t eq = e.find(L'=');
            if (eq == std::wstring_view::npos) continue;
            env.emplace_back(narrow(e.substr(0, eq)), narrow(e.substr(eq + 1)));
        }
        FreeEnvironmentStringsW(block);
        return env;
    }

    void init_console() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        for (DWORD which : {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE}) {
            HANDLE h = GetStdHandle(which);
            DWORD mode = 0;
            if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }

    void ignore_interrupts() { SetConsoleCtrlHandler(ctrl_handler, TRUE); }

    bool stdin_is_console() {
        DWORD mode = 0;
        return GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode) != 0;
    }

    bool stdout_is_console() {
        DWORD mode = 0;
        return GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode) != 0;
    }

    int terminal_width() {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
            return info.srWindow.Right - info.srWindow.Left + 1;
        }
        return 80;
    }

    bool read_line(std::string& line) {
        HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode = 0;
        if (!GetConsoleMode(h, &mode)) {
            if (!std::getline(std::cin, line)) return false;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            return true;
        }
        // ReadConsoleW gives proper Unicode input plus the console's own line editing
        // and history (arrow keys, F7).
        std::wstring buf;
        wchar_t chunk[1024];
        for (;;) {
            DWORD n = 0;
            if (!ReadConsoleW(h, chunk, 1024, &n, nullptr)) {
                if (GetLastError() == ERROR_OPERATION_ABORTED) { line.clear(); std::cout << "\n"; return true; }
                return false;
            }
            if (n == 0) { line.clear(); std::cout << "\n"; return true; } // Ctrl-C
            buf.append(chunk, n);
            if (buf.back() == L'\n') break;
        }
        while (!buf.empty() && (buf.back() == L'\n' || buf.back() == L'\r')) buf.pop_back();
        if (!buf.empty() && (buf[0] == 0x1a || buf[0] == 0x04)) return false; // Ctrl-Z / Ctrl-D
        line = narrow(buf);
        return true;
    }

    std::string user_name() { return get_env("USERNAME").value_or("user"); }
    std::string host_name() { return get_env("COMPUTERNAME").value_or("localhost"); }

    fs::path home_dir() {
        if (auto p = get_env("USERPROFILE")) return to_path(*p);
        if (auto p = get_env("HOME")) return to_path(*p);
        return fs::path(L"C:\\");
    }

    int process_id() { return static_cast<int>(GetCurrentProcessId()); }

    bool is_hidden(const fs::path& p, const std::string& name) {
        if (!name.empty() && name[0] == '.') return true;
        const DWORD a = GetFileAttributesW(p.c_str());
        return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_HIDDEN);
    }

    bool is_executable(const fs::path& p) {
        const std::wstring ext = lower(p.extension().wstring());
        if (ext.empty()) return false;
        const auto exts = executable_extensions();
        return std::find(exts.begin(), exts.end(), ext) != exts.end();
    }

    std::optional<fs::path> find_executable(const std::string& name) {
        if (name.empty()) return std::nullopt;
        const auto exts = executable_extensions();
        auto resolve = [&](const fs::path& base) -> std::optional<fs::path> {
            const std::wstring ext = lower(base.extension().wstring());
            if (!ext.empty() && std::find(exts.begin(), exts.end(), ext) != exts.end() && is_file(base)) return base;
            for (const auto& e : exts) {
                fs::path p = base;
                p += e;
                if (is_file(p)) return p;
            }
            return std::nullopt;
        };
        if (name.find_first_of("/\\:") != std::string::npos) return resolve(to_path(name));
        const std::string path = get_env("PATH").value_or("");
        size_t start = 0;
        while (start <= path.size()) {
            size_t end = path.find(';', start);
            if (end == std::string::npos) end = path.size();
            std::string dir = path.substr(start, end - start);
            dir.erase(std::remove(dir.begin(), dir.end(), '"'), dir.end());
            if (!dir.empty()) {
                if (auto r = resolve(to_path(dir) / to_path(name))) return r;
            }
            start = end + 1;
        }
        return std::nullopt;
    }

    int run_process(const fs::path& exe, const std::vector<std::string>& argv, const StdioSpec& io) {
        const std::wstring ext = lower(exe.extension().wstring());
        std::wstring cmdline;
        auto append = [&](const std::wstring& arg) {
            if (!cmdline.empty()) cmdline += L' ';
            cmdline += quote_arg(arg);
        };
        if (ext == L".bat" || ext == L".cmd") {
            std::wstring inner = quote_arg(exe.wstring());
            for (size_t i = 1; i < argv.size(); ++i) inner += L" " + quote_arg(widen(argv[i]));
            const std::wstring comspec = widen(get_env("ComSpec").value_or("C:\\Windows\\System32\\cmd.exe"));
            cmdline = quote_arg(comspec) + L" /d /s /c \"" + inner + L"\"";
        } else if (ext == L".ps1") {
            auto ps = find_executable("pwsh");
            if (!ps) ps = find_executable("powershell");
            if (!ps) return -1;
            append(ps->wstring());
            append(L"-NoProfile");
            append(L"-ExecutionPolicy");
            append(L"Bypass");
            append(L"-File");
            append(exe.wstring());
            for (size_t i = 1; i < argv.size(); ++i) append(widen(argv[i]));
        } else {
            append(exe.wstring());
            for (size_t i = 1; i < argv.size(); ++i) append(widen(argv[i]));
        }

        std::vector<HANDLE> owned;
        struct Closer {
            std::vector<HANDLE>& handles;
            ~Closer() { for (HANDLE h : handles) CloseHandle(h); }
        } closer{owned};

        SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
        auto open = [&](const std::string& path, DWORD access, DWORD disposition) {
            HANDLE h = CreateFileW(widen(path).c_str(), access, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                   &sa, disposition, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (h == INVALID_HANDLE_VALUE) {
                const DWORD e = GetLastError();
                throw std::runtime_error(path + ": " + (e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND
                                                            ? "No such file or directory"
                                                            : e == ERROR_ACCESS_DENIED ? "Permission denied" : "cannot open"));
            }
            owned.push_back(h);
            return h;
        };
        auto open_out = [&](const std::string& path, bool append_mode) {
            return append_mode ? open(path, FILE_APPEND_DATA | SYNCHRONIZE, OPEN_ALWAYS)
                               : open(path, GENERIC_WRITE, CREATE_ALWAYS);
        };
        auto inherit_std = [&](DWORD which) {
            HANDLE src = GetStdHandle(which);
            HANDLE dup = nullptr;
            if (src && src != INVALID_HANDLE_VALUE &&
                DuplicateHandle(GetCurrentProcess(), src, GetCurrentProcess(), &dup, 0, TRUE, DUPLICATE_SAME_ACCESS)) {
                owned.push_back(dup);
                return dup;
            }
            return src;
        };

        STARTUPINFOW si{};
        si.cb = sizeof(si);
        const bool redirect = !io.in_path.empty() || !io.out_path.empty() || !io.err_path.empty() || io.err_to_out || io.out_to_err;
        if (redirect) {
            HANDLE in = io.in_path.empty() ? inherit_std(STD_INPUT_HANDLE) : open(io.in_path, GENERIC_READ, OPEN_EXISTING);
            HANDLE out = io.out_path.empty() ? inherit_std(STD_OUTPUT_HANDLE) : open_out(io.out_path, io.out_append);
            HANDLE err = io.err_path.empty() ? inherit_std(STD_ERROR_HANDLE) : open_out(io.err_path, io.err_append);
            if (io.err_to_out) err = out;
            if (io.out_to_err) out = err;
            si.dwFlags = STARTF_USESTDHANDLES;
            si.hStdInput = in;
            si.hStdOutput = out;
            si.hStdError = err;
        }

        PROCESS_INFORMATION pi{};
        if (!CreateProcessW(nullptr, cmdline.data(), nullptr, nullptr, redirect ? TRUE : FALSE, 0, nullptr, nullptr, &si, &pi)) {
            return -1;
        }
        CloseHandle(pi.hThread);
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(pi.hProcess, &code);
        CloseHandle(pi.hProcess);
        if (code == 0xC000013A) return 130; // STATUS_CONTROL_C_EXIT, bash reports 128+SIGINT
        return static_cast<int>(code);
    }

#else // POSIX

    fs::path to_path(const std::string& s) { return fs::path(s); }
    std::string from_path(const fs::path& p) { return p.string(); }

    std::vector<std::string> command_line_args(int argc, char** argv) {
        return std::vector<std::string>(argv, argv + argc);
    }

    std::optional<std::string> get_env(const std::string& name) {
        const char* v = std::getenv(name.c_str());
        if (!v) return std::nullopt;
        return std::string(v);
    }

    void set_env(const std::string& name, const std::string& value) { ::setenv(name.c_str(), value.c_str(), 1); }
    void unset_env(const std::string& name) { ::unsetenv(name.c_str()); }

    std::vector<std::pair<std::string, std::string>> environment() {
        std::vector<std::pair<std::string, std::string>> env;
        for (char** e = environ; e && *e; ++e) {
            std::string s(*e);
            const size_t eq = s.find('=');
            if (eq == std::string::npos) continue;
            env.emplace_back(s.substr(0, eq), s.substr(eq + 1));
        }
        return env;
    }

    void init_console() {}
    void ignore_interrupts() { std::signal(SIGINT, SIG_IGN); }
    bool stdin_is_console() { return ::isatty(0) != 0; }
    bool stdout_is_console() { return ::isatty(1) != 0; }

    int terminal_width() {
        winsize ws{};
        if (::ioctl(1, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) return ws.ws_col;
        if (auto c = get_env("COLUMNS")) return std::max(20, std::atoi(c->c_str()));
        return 80;
    }

    bool read_line(std::string& line) {
        if (!std::getline(std::cin, line)) return false;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        return true;
    }

    std::string user_name() {
        if (auto u = get_env("USER")) return *u;
        if (passwd* pw = ::getpwuid(::getuid())) return pw->pw_name;
        return "user";
    }

    std::string host_name() {
        char buf[256] = {};
        if (::gethostname(buf, sizeof buf - 1) == 0) return buf;
        return "localhost";
    }

    fs::path home_dir() {
        if (auto h = get_env("HOME")) return *h;
        return "/";
    }

    int process_id() { return static_cast<int>(::getpid()); }

    bool is_hidden(const fs::path&, const std::string& name) { return !name.empty() && name[0] == '.'; }

    bool is_executable(const fs::path& p) { return ::access(p.c_str(), X_OK) == 0; }

    std::optional<fs::path> find_executable(const std::string& name) {
        if (name.empty()) return std::nullopt;
        auto ok = [](const fs::path& p) {
            std::error_code ec;
            return fs::is_regular_file(p, ec) && is_executable(p);
        };
        if (name.find('/') != std::string::npos) {
            if (ok(name)) return fs::path(name);
            return std::nullopt;
        }
        const std::string path = get_env("PATH").value_or("/usr/bin:/bin");
        size_t start = 0;
        while (start <= path.size()) {
            size_t end = path.find(':', start);
            if (end == std::string::npos) end = path.size();
            const std::string dir = path.substr(start, end - start);
            const fs::path p = fs::path(dir.empty() ? "." : dir) / name;
            if (ok(p)) return p;
            start = end + 1;
        }
        return std::nullopt;
    }

    int run_process(const fs::path& exe, const std::vector<std::string>& argv, const StdioSpec& io) {
        std::vector<char*> cargv;
        for (const auto& s : argv) cargv.push_back(const_cast<char*>(s.c_str()));
        cargv.push_back(nullptr);

        std::vector<int> fds;
        auto open_fd = [&](const std::string& p, int flags) {
            const int fd = ::open(p.c_str(), flags | O_CLOEXEC, 0644);
            if (fd < 0) {
                const std::string msg = p + ": " + std::strerror(errno);
                for (int f : fds) ::close(f);
                throw std::runtime_error(msg);
            }
            fds.push_back(fd);
            return fd;
        };
        const int in = io.in_path.empty() ? -1 : open_fd(io.in_path, O_RDONLY);
        const int out = io.out_path.empty() ? -1 : open_fd(io.out_path, O_WRONLY | O_CREAT | (io.out_append ? O_APPEND : O_TRUNC));
        const int err = io.err_path.empty() ? -1 : open_fd(io.err_path, O_WRONLY | O_CREAT | (io.err_append ? O_APPEND : O_TRUNC));

        const pid_t pid = ::fork();
        if (pid == 0) {
            std::signal(SIGINT, SIG_DFL);
            if (in >= 0) ::dup2(in, 0);
            if (out >= 0) ::dup2(out, 1);
            if (err >= 0) ::dup2(err, 2);
            if (io.err_to_out) ::dup2(1, 2);
            if (io.out_to_err) ::dup2(2, 1);
            ::execv(exe.c_str(), cargv.data());
            ::_exit(126);
        }
        for (int f : fds) ::close(f);
        if (pid < 0) return -1;
        int st = 0;
        while (::waitpid(pid, &st, 0) < 0 && errno == EINTR) {}
        if (WIFEXITED(st)) return WEXITSTATUS(st);
        if (WIFSIGNALED(st)) return 128 + WTERMSIG(st);
        return 1;
    }

#endif

}
