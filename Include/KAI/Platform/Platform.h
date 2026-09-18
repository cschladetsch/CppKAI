#pragma once
// KAI Platform abstraction layer
// Included transitively by all KAI translation units via Config/Platform.h

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>

// Undo Windows.h macros that collide with KAI and STL
#  ifdef GetObject
#    undef GetObject
#  endif
#  ifdef GetMessage
#    undef GetMessage
#  endif
#  ifdef SendMessage
#    undef SendMessage
#  endif
#  ifdef min
#    undef min
#  endif
#  ifdef max
#    undef max
#  endif
#  ifdef ERROR
#    undef ERROR
#  endif

// POSIX compat
#ifdef _WIN32
// getopt is not available on Windows - use a minimal implementation
// or the Windows-specific alternative
#  include <string.h>
intGetoptt(int argc, char* const argv[], const char* optstring);
extern char* optarg;
extern int optind, opterr, optopt;
#endif

#definePOPENN _popen
#define PCPCLOSEpclose

#endif // _WIN32
