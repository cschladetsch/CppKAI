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

enum class style {
    reset=0,bold=1,dim=2,italic=3,underline=4,blink=5,reversed=7,conceal=8,
    Reset=0,Bold=1,Dim=2,Italic=3,Underline=4,Blink=5,Reversed=7,Conceal=8
};
enum class fg {
    black=30,red=31,green=32,yellow=33,blue=34,magenta=35,cyan=36,gray=37,reset=39,
    Black=30,Red=31,Green=32,Yellow=33,Blue=34,Magenta=35,Cyan=36,Gray=37,Reset=39
};
enum class bg {
    black=40,red=41,green=42,yellow=43,blue=44,magenta=45,cyan=46,gray=47,reset=49,
    Black=40,Red=41,Green=42,Yellow=43,Blue=44,Magenta=45,Cyan=46,Gray=47,Reset=49
};
enum class fgB{black=90,red=91,green=92,yellow=93,blue=94,magenta=95,cyan=96,gray=97};
enum class bgB{black=100,red=101,green=102,yellow=103,blue=104,magenta=105,cyan=106,gray=107};
enum class control{Auto=0,Off=1,Force=2,autoColor=0,off=1,force=2};

using Fg=fg; using Bg=bg; using Style=style; using Control=control;

namespace rang_impl {
    inline std::atomic<control>& mode() noexcept {
        static std::atomic<control> v(control::Auto); return v;
    }
    inline bool useColor(std::ostream& os) noexcept {
        auto m=mode().load();
        if(m==control::Force) return true;
        if(m==control::Off)   return false;
#ifdef _WIN32
        HANDLE h=INVALID_HANDLE_VALUE;
        if(&os==&std::cout)       h=GetStdHandle(STD_OUTPUT_HANDLE);
        else if(&os==&std::cerr||&os==&std::clog) h=GetStdHandle(STD_ERROR_HANDLE);
        if(h==INVALID_HANDLE_VALUE) return false;
        DWORD dm=0; GetConsoleMode(h,&dm);
        if(!(dm&ENABLE_VIRTUAL_TERMINAL_PROCESSING))
            SetConsoleMode(h,dm|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        GetConsoleMode(h,&dm);
        return (dm&ENABLE_VIRTUAL_TERMINAL_PROCESSING)!=0;
#else
        return true;
#endif
    }
    inline void emit(std::ostream& os, int v) { os<<"\033["<<v<<"m"; }
}

inline std::ostream& operator<<(std::ostream& os,fg v)    {if(rang_impl::useColor(os))rang_impl::emit(os,(int)v);return os;}
inline std::ostream& operator<<(std::ostream& os,bg v)    {if(rang_impl::useColor(os))rang_impl::emit(os,(int)v);return os;}
inline std::ostream& operator<<(std::ostream& os,fgB v)   {if(rang_impl::useColor(os))rang_impl::emit(os,(int)v);return os;}
inline std::ostream& operator<<(std::ostream& os,bgB v)   {if(rang_impl::useColor(os))rang_impl::emit(os,(int)v);return os;}
inline std::ostream& operator<<(std::ostream& os,style v) {if(rang_impl::useColor(os))rang_impl::emit(os,(int)v);return os;}

inline void setControlMode(control v) noexcept { rang_impl::mode()=v; }
inline void SetControlMode(control v) noexcept { rang_impl::mode()=v; }

} // namespace rang
