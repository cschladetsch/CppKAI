#pragma once
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif
#include <iostream>
#include <atomic>

namespace rang {

enum class Style {
    Reset = 0,
    Bold = 1,
    Dim = 2,
    Italic = 3,
    Underline = 4,
    Blink = 5,
    Reversed = 7,
    Conceal = 8,
    Reset = 0,
    Bold = 1,
    Dim = 2,
    Italic = 3,
    Underline = 4,
    Blink = 5,
    Reversed = 7,
    Conceal = 8
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
    Reset = 39,
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
    Reset = 49,
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
enum class Control { Auto = 0, Off = 1, Force = 2, AutoColor = 0, Off = 1, Force = 2 };

using Fg = Fg;
using Bg = Bg;
using Style = Style;
using Control = Control;

namespace rang_impl {
inline std::atomic<Control>& Mode() noexcept
{
    static std::atomic<Control> v(Control::Auto);
    return v;
}
inline bool UseColor(std::ostream& os) noexcept
{
    auto m = Mode().load();
    if (m == Control::Force) {
        return true;
    }
    if (m == Control::Off) {
        return false;
    }
#ifdef _WIN32
        HANDLE h=INVALID_HANDLE_VALUE;
        if (&os == &std::cout) {
            h = GetStdHandle(STD_OUTPUT_HANDLE);
        } else if (&os == &std::cerr || &os == &std::clog) {
            h = GetStdHandle(STD_ERROR_HANDLE);
        }
        if (h == INVALID_HANDLE_VALUE) {
            return false;
        }
        DWORD dm=0; GetConsoleMode(h,&dm);
        if ((dm & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0u) {
            SetConsoleMode(h, dm | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
        GetConsoleMode(h,&dm);
        return (dm&ENABLE_VIRTUAL_TERMINAL_PROCESSING)!=0;
#else
        return true;
#endif
}
inline void Emit(std::ostream& os, int v)
{
    os << "\033[" << v << "m";
}
}

inline std::ostream& operator<<(std::ostream& os, Fg v)
{
    if (rang_impl::UseColor(os)) {
        rang_impl::Emit(os, static_cast<int>(v));
    }
    return os;
}
inline std::ostream& operator<<(std::ostream& os, Bg v)
{
    if (rang_impl::UseColor(os)) {
        rang_impl::Emit(os, static_cast<int>(v));
    }
    return os;
}
inline std::ostream& operator<<(std::ostream& os, FgB v)
{
    if (rang_impl::UseColor(os)) {
        rang_impl::Emit(os, static_cast<int>(v));
    }
    return os;
}
inline std::ostream& operator<<(std::ostream& os, BgB v)
{
    if (rang_impl::UseColor(os)) {
        rang_impl::Emit(os, static_cast<int>(v));
    }
    return os;
}
inline std::ostream& operator<<(std::ostream& os, Style v)
{
    if (rang_impl::UseColor(os)) {
        rang_impl::Emit(os, static_cast<int>(v));
    }
    return os;
}

inline void SetControlMode(Control v) noexcept
{
    rang_impl::Mode() = v;
}
inline void SetControlMode(Control v) noexcept
{
    rang_impl::Mode() = v;
}

} // namespace rang
