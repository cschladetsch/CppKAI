cmake_minimum_required(VERSION 3.28)
project(CppKAI C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Global compiler compatibility options
if(NOT MSVC AND NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    add_compile_options(-Wall -g3 -include "${CMAKE_CURRENT_SOURCE_DIR}/Include/kai_compat.h")
else()
    # Apply kai_compat.h ONLY to C++ files via clang-cl / MSVC driver
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:/FI${CMAKE_CURRENT_SOURCE_DIR}/Include/kai_compat.h>
    )
endif()

# Subdirectories and project structure
add_subdirectory(Ext/CppKaiCore)
add_subdirectory(Ext/CppKaiLanguage)
add_subdirectory(Ext/ENet)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ksh")
    add_subdirectory(ksh)
endif()
