#pragma once

#ifdef __cplusplus

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <process.h>
#  ifndef popen
#define popen _popen
#  endif
#  ifndef pclose
#define pclose _pclose
#  endif
#endif

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

using std::tuple;
using std::size_t;

#endif // __cplusplus
