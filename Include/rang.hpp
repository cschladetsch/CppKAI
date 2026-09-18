#ifndef RANG_DOT_HPP
#define RANG_DOT_HPP

#if defined(__unix__) || defined(__unix) || defined(__linux__)
#define OS_LINUX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define OS_WIN
#elif defined(__APPLE__) || defined(__MACH__)
#define OS_MAC
#else
#error Unknown Platform
#endif

#if defined(OS_LINUX) || defined(OS_MAC)
#include <unistd.h>

#elifdef OS_WIN

#if defined(_WIN32_WINNT) && (_WIN32_WINNT < 0x0600)
#error \
    "Please include rang.hpp before any windows system headers or set _WIN32_WINNT at least to _WIN32_WINNT_VISTA"
#elifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

#include <io.h>
#include <windows.h>

#include <memory>

// Only defined in windows 10 onwards, redefining in lower windows since it
// doesn't gets used in lower versions
// https://docs.microsoft.com/en-us/windows/console/getconsolemode
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#endif

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace rang {

/* For better compability with most of terminals do not use any style settings
 * except of reset, bold and reversed.
 * Note that on Windows terminals bold style is same as fgB color.
 */
enum class Style {
    Reset = 0,
    Bold = 1,
    Dim = 2,
    Italic = 3,
    Underline = 4,
    Blink = 5,
    Rblink = 6,
    Reversed = 7,
    Conceal = 8,
    Crossed = 9
};

enum class Fg {
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    Gray = 37,
    Reset = 39
};

enum class Bg {
    Black = 40,
    Red = 41,
    Green = 42,
    Yellow = 43,
    Blue = 44,
    Magenta = 45,
    Cyan = 46,
    Gray = 47,
    Reset = 49
};

enum class FgB { Black = 90, Red = 91, Green = 92, Yellow = 93, Blue = 94, Magenta = 95, Cyan = 96, Gray = 97 };

enum class BgB { Black = 100, Red = 101, Green = 102, Yellow = 103, Blue = 104, Magenta = 105, Cyan = 106, Gray = 107 };

enum class Control { // Behaviour of rang function calls
    Off = 0,         // toggle off rang style/color calls
    Auto = 1,        // (Default) autodect terminal and colorize if needed
    Force = 2        // force ansi color output to non terminal streams
};
// Use rang::setControlMode to set rang control mode

enum class WinTerm { // Windows Terminal Mode
    Auto = 0,        // (Default) automatically detects wheter Ansi or Native API
    Ansi = 1,        // Force use Ansi API
    Native = 2       // Force use Native API
};
// Use rang::setWinTermMode to explicitly set terminal API for Windows
// Calling rang::setWinTermMode have no effect on other OS

namespace rang_implementation {

inline std::atomic<Control>& ControlMode() noexcept
{
    static std::atomic<Control> value(Control::Auto);
    return value;
}

inline std::atomic<WinTerm>& WinTermMode() noexcept
{
    static std::atomic<WinTerm> termMode(WinTerm::Auto);
    return termMode;
}

inline bool SupportsColor() noexcept
{
#if defined(OS_LINUX) || defined(OS_MAC)

    static const bool result = [] {
        const char *Terms[] = {"ansi",  "color",   "console", "cygwin",
                               "gnome", "konsole", "kterm",   "linux",
                               "msys",  "putty",   "rxvt",    "screen",
                               "vt100", "xterm"};

        const char *env_p = std::getenv("TERM");
        if (env_p == nullptr) {
            return false;
        }
        return std::any_of(std::begin(Terms), std::end(Terms),
                           [&](const char *term) {
                               return std::strstr(env_p, term) != nullptr;
                           });
    }();

#elifdef OS_WIN
    // All windows versions support colors through native console methods
    static constexpr bool kResult = true;
#endif
    return kResult;
}

#ifdef OS_WIN

inline bool isMsysPty(int fd) noexcept {
    // Dynamic load for binary compability with old Windows
    const auto ptrGetFileInformationByHandleEx =
        reinterpret_cast<decltype(&GetFileInformationByHandleEx)>(
            GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
                           "GetFileInformationByHandleEx"));
    if (ptrGetFileInformationByHandleEx == nullptr) {
        return false;
    }

    auto h = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Check that it's a pipe:
    if (GetFileType(h) != FILE_TYPE_PIPE) {
        return false;
    }

    // POD type is binary compatible with FILE_NAME_INFO from WinBase.h
    // It have the same alignment and used to avoid UB in caller code
    struct MyFileNameInfo {
        DWORD fileNameLength;
        WCHAR fileName[MAX_PATH];
    };

    auto pNameInfo = std::unique_ptr<MyFileNameInfo>(new (std::nothrow) MyFileNameInfo());
    if (!pNameInfo) {
        return false;
    }

    // Check pipe name is template of
    // {"cygwin-","msys-"}XXXXXXXXXXXXXXX-ptyX-XX
    if (ptrGetFileInformationByHandleEx(h, FileNameInfo, pNameInfo.get(), sizeof(MyFileNameInfo)) == 0) {
        return false;
    }
    std::wstring name(pNameInfo->fileName, pNameInfo->fileNameLength / sizeof(WCHAR));
    return !((name.find(L"msys-") == std::wstring::npos && name.find(L"cygwin-") == std::wstring::npos) ||
             name.find(L"-pty") == std::wstring::npos);
}

#endif

inline bool IsTerminal(const std::streambuf* osbuf) noexcept
{
    using std::cerr;
    using std::clog;
    using std::cout;
#if defined(OS_LINUX) || defined(OS_MAC)
    if (osbuf == cout.rdbuf()) {
        static const bool cout_term = isatty(fileno(stdout)) != 0;
        return cout_term;
    } else if (osbuf == cerr.rdbuf() || osbuf == clog.rdbuf()) {
        static const bool cerr_term = isatty(fileno(stderr)) != 0;
        return cerr_term;
    }
#elifdef OS_WIN
    if (osbuf == cout.rdbuf()) {
        static const bool kCoutTerm = ((_isatty(_fileno(stdout)) != 0) || isMsysPty(_fileno(stdout)));
        return kCoutTerm;
    }
    if (osbuf == cerr.rdbuf() || osbuf == clog.rdbuf()) {
        static const bool cerr_term = (_isatty(_fileno(stderr)) || isMsysPty(_fileno(stderr)));
        return cerr_term;
    }
#endif
    return false;
}

template <typename T>
using enableStd =
    std::enable_if_t<std::is_same_v<T, rang::Style> || std::is_same_v<T, rang::Fg> || std::is_same_v<T, rang::Bg> ||
                         std::is_same_v<T, rang::FgB> || std::is_same_v<T, rang::BgB>,
                     std::ostream&>;

#ifdef OS_WIN

struct SGR {          // Select Graphic Rendition parameters for Windows console
    BYTE fgColor;     // foreground color (0-15) lower 3 rgb bits + intense bit
    BYTE bgColor;     // background color (0-15) lower 3 rgb bits + intense bit
    BYTE bold;        // emulated as FOREGROUND_INTENSITY bit
    BYTE underline;   // emulated as BACKGROUND_INTENSITY bit
    BOOLEAN inverse;  // swap foreground/bold & background/underline
    BOOLEAN conceal;  // set foreground/bold to background/underline
};

enum class AttrColor : BYTE { // Color attributes for console screen buffer
    Black = 0,
    Red = 4,
    Green = 2,
    Yellow = 6,
    Blue = 1,
    Magenta = 5,
    Cyan = 3,
    Gray = 7
};

inline HANDLE GetConsoleHandle(const std::streambuf* osbuf) noexcept
{
    if (osbuf == std::cout.rdbuf()) {
        static const HANDLE kHStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        return kHStdout;
    }
    if (osbuf == std::cerr.rdbuf() || osbuf == std::clog.rdbuf()) {
        static const HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE);
        return hStderr;
    }
    return INVALID_HANDLE_VALUE;
}

inline bool setWinTermAnsiColors(const std::streambuf *osbuf) noexcept {
    HANDLE h = GetConsoleHandle(osbuf);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD dwMode = 0;
    if (GetConsoleMode(h, &dwMode) == 0) {
        return false;
    }
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(h, dwMode) != 0;
}

inline bool SupportsAnsi(const std::streambuf* osbuf) noexcept
{
    using std::cerr;
    using std::clog;
    using std::cout;
    if (osbuf == cout.rdbuf()) {
        static const bool kCoutAnsi = (isMsysPty(_fileno(stdout)) || setWinTermAnsiColors(osbuf));
        return kCoutAnsi;
    }
    if (osbuf == cerr.rdbuf() || osbuf == clog.rdbuf()) {
        static const bool cerr_ansi = (isMsysPty(_fileno(stderr)) || setWinTermAnsiColors(osbuf));
        return cerr_ansi;
    }
    return false;
}

inline const SGR& DefaultState() noexcept
{
    static const SGR kDefaultSgr = []() -> SGR {
        CONSOLE_SCREEN_BUFFER_INFO info;
        WORD attrib = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                                       &info) ||
            GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &info)) {
            attrib = info.wAttributes;
        }
        SGR sgr = {.fgColor = 0, .bgColor = 0, .bold = 0, .underline = 0, .inverse = FALSE, .conceal = FALSE};
        sgr.fgColor = attrib & 0x0F;
        sgr.bgColor = (attrib & 0xF0) >> 4;
        return sgr;
    }();
    return kDefaultSgr;
}

inline BYTE Ansi2attr(BYTE rgb) noexcept
{
    static const AttrColor kRev[8] = {AttrColor::Black, AttrColor::Red,     AttrColor::Green, AttrColor::Yellow,
                                      AttrColor::Blue,  AttrColor::Magenta, AttrColor::Cyan,  AttrColor::Gray};
    return static_cast<BYTE>(kRev[rgb]);
}

inline void SetWinSgr(rang::Bg col, SGR& state) noexcept
{
    if (col != rang::Bg::Reset) {
        state.bgColor = Ansi2attr(static_cast<BYTE>(col) - 40);
    } else {
        state.bgColor = DefaultState().bgColor;
    }
}

inline void SetWinSgr(rang::Fg col, SGR& state) noexcept
{
    if (col != rang::Fg::Reset) {
        state.fgColor = Ansi2attr(static_cast<BYTE>(col) - 30);
    } else {
        state.fgColor = DefaultState().fgColor;
    }
}

inline void SetWinSgr(rang::BgB col, SGR& state) noexcept
{
    state.bgColor = (BACKGROUND_INTENSITY >> 4) | Ansi2attr(static_cast<BYTE>(col) - 100);
}

inline void SetWinSgr(rang::FgB col, SGR& state) noexcept
{
    state.fgColor = FOREGROUND_INTENSITY | Ansi2attr(static_cast<BYTE>(col) - 90);
}

inline void SetWinSgr(rang::Style style, SGR& state) noexcept
{
    switch (style) {
        case rang::Style::Reset:
            state = DefaultState();
            break;
        case rang::Style::Bold:
            state.bold = FOREGROUND_INTENSITY;
            break;
        case rang::Style::Underline:
        case rang::Style::Blink:
            state.underline = BACKGROUND_INTENSITY;
            break;
        case rang::Style::Reversed:
            state.inverse = TRUE;
            break;
        case rang::Style::Conceal:
            state.conceal = TRUE;
            break;
        default:
            break;
    }
}

inline SGR& CurrentState() noexcept
{
    static SGR state = DefaultState();
    return state;
}

inline WORD SGR2Attr(const SGR &state) noexcept {
    WORD attrib = 0;
    if (state.conceal != 0u) {
        if (state.inverse != 0u) {
            attrib = (state.fgColor << 4) | state.fgColor;
            if (state.bold != 0u) {
                attrib |= FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
            }
        } else {
            attrib = (state.bgColor << 4) | state.bgColor;
            if (state.underline != 0u) {
                attrib |= FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
            }
        }
    } else if (state.inverse != 0u) {
        attrib = (state.fgColor << 4) | state.bgColor;
        if (state.bold != 0u) {
            attrib |= BACKGROUND_INTENSITY;
        }
        if (state.underline != 0u) {
            attrib |= FOREGROUND_INTENSITY;
        }
    } else {
        attrib =
            state.fgColor | (state.bgColor << 4) | state.bold | state.underline;
    }
    return attrib;
}

template <typename T> inline void SetWinColorAnsi(std::ostream& os, T const value)
{
    os << "\033[" << static_cast<int>(value) << "m";
}

template <typename T> inline void SetWinColorNative(std::ostream& os, T const value)
{
    const HANDLE h = GetConsoleHandle(os.rdbuf());
    if (h != INVALID_HANDLE_VALUE) {
        setWinSGR(value, CurrentState());
        // Out all buffered text to console with previous settings:
        os.flush();
        SetConsoleTextAttribute(h, SGR2Attr(CurrentState()));
    }
}

template <typename T> inline enableStd<T> SetColor(std::ostream& os, T const value)
{
    if (WinTermMode() == WinTerm::Auto) {
        if (SupportsAnsi(os.rdbuf())) {
            setWinColorAnsi(os, value);
        } else {
            setWinColorNative(os, value);
        }
    } else if (WinTermMode() == WinTerm::Ansi) {
        setWinColorAnsi(os, value);
    } else {
        setWinColorNative(os, value);
    }
    return os;
}
#else
template <typename T>
inline enableStd<T> setColor(std::ostream &os, T const value) {
    return os << "\033[" << static_cast<int>(value) << "m";
}
#endif
}  // namespace rang_implementation

template <typename T>
inline rang_implementation::enableStd<T> operator<<(std::ostream &os,
                                                    const T value) {
    const Control option = rang_implementation::ControlMode();
    switch (option) {
        case Control::Auto:
            return rang_implementation::SupportsColor() && rang_implementation::IsTerminal(os.rdbuf())
                       ? rang_implementation::setColor(os, value)
                       : os;
        case Control::Force:
            return rang_implementation::setColor(os, value);
        default:
            return os;
    }
}

inline void SetWinTermMode(const rang::WinTerm value) noexcept
{
    rang_implementation::WinTermMode() = value;
}

inline void SetControlMode(const Control value) noexcept
{
    rang_implementation::ControlMode() = value;
}

}  // namespace rang

#undef OS_LINUX
#undef OS_WIN
#undef OS_MAC

#endif /* ifndef RANG_DOT_HPP */
