cmake_minimum_required(VERSION 3.28...3.30)

# Set OpenGL policy to NEW to avoid warnings
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

# KAI Project Build Configuration
# -------------------------------
# - Uses Ninja generator by default for faster builds
# - Uses Clang/Clang++ by default (preferred compiler)
# - Override to use GCC with -DBUILD_GCC=ON
# - Defaults to the C++23 standard with modern C++ features
# - Supports out-of-source builds with output to build/Bin directory
# - Includes Clang-specific optimizations and colored diagnostics

# Enable clang-tidy for static analysis when using Clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*,performance-*,modernize-*")
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    else()
        message(STATUS "clang-tidy not found, skipping static analysis")
    endif()
endif()

# Set the default generator to Ninja and compiler to Clang unless BUILD_GCC is set
if(NOT DEFINED BUILD_GCC)
    # Ensure Ninja is used as the default generator (faster builds)
    if(NOT CMAKE_GENERATOR)
        set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "Generator used by CMake")
        message(STATUS "Using Ninja generator by default (override with -DBUILD_GCC=ON)")
    endif()
    
    # Set default compiler to Clang++ if not explicitly set
    if(NOT CMAKE_CXX_COMPILER)
        find_program(CLANG_CXX_COMPILER clang++)
        if(CLANG_CXX_COMPILER)
            set(CMAKE_CXX_COMPILER "${CLANG_CXX_COMPILER}" CACHE STRING "C++ compiler" FORCE)
            message(STATUS "Using Clang++ compiler by default at: ${CLANG_CXX_COMPILER}")
        else()
            message(WARNING "Clang++ not found, falling back to system default compiler")
        endif()
    endif()
    
    # Set default C compiler to Clang if not explicitly set
    if(NOT CMAKE_C_COMPILER)
        find_program(CLANG_C_COMPILER clang)
        if(CLANG_C_COMPILER)
            set(CMAKE_C_COMPILER "${CLANG_C_COMPILER}" CACHE STRING "C compiler" FORCE)
            message(STATUS "Using Clang compiler by default at: ${CLANG_C_COMPILER}")
        endif()
    endif()
else()
    message(STATUS "Using GCC compiler toolchain (BUILD_GCC is set)")
    # Mark BUILD_GCC as used
    option(BUILD_GCC "Use GCC compiler instead of Clang++" ${BUILD_GCC})
    
    # Ensure GCC is used when BUILD_GCC is set
    if(NOT CMAKE_CXX_COMPILER)
        find_program(GCC_CXX_COMPILER g++)
        if(GCC_CXX_COMPILER)
            set(CMAKE_CXX_COMPILER "${GCC_CXX_COMPILER}" CACHE STRING "C++ compiler" FORCE)
            message(STATUS "Using GCC++ compiler at: ${GCC_CXX_COMPILER}")
        endif()
    endif()
    
    if(NOT CMAKE_C_COMPILER)
        find_program(GCC_C_COMPILER gcc)
        if(GCC_C_COMPILER)
            set(CMAKE_C_COMPILER "${GCC_C_COMPILER}" CACHE STRING "C compiler" FORCE)
            message(STATUS "Using GCC compiler at: ${GCC_C_COMPILER}")
        endif()
    endif()
endif()

project(KAI)

# Warn loudly if we didn't actually land on Clang, despite the "prefer Clang"
# logic above (lines 28-54).
#
# That logic is a no-op in the exact situation it's meant to fix: CMake picks
# its generator (from -G, or its own per-platform default when -G is
# omitted - on Windows, the newest installed Visual Studio generator) before
# this file is ever read, and the chosen generator is then locked in via
# this build directory's CMakeCache.txt for as long as that directory
# exists. So `if(NOT CMAKE_GENERATOR)` above is always false by the time it
# runs - CMAKE_GENERATOR is already populated with whatever was picked - and
# `set(CMAKE_GENERATOR "Ninja" ...)` has no effect even when it does run,
# since a project cannot change its own generator mid-configure. On top of
# that, a Visual Studio generator ignores CMAKE_CXX_COMPILER for toolset
# selection (it always drives cl.exe/MSVC) unless the ClangCL toolset is
# requested explicitly with `-T ClangCL`.
#
# The practical effect: on a fresh clone, a plain `cmake -S . -B build` on
# Windows silently configures MSVC + a Visual Studio/MSBuild generator
# (producing .vcxproj files) while this file's messages still claim
# "Using Clang++ compiler by default". Anyone relying on that message
# without checking the actual compiler can be surprised much later - e.g. by
# MSVC-only flag conflicts such as `/O2` + `/RTC1` in a Debug config (see
# Test/Performance/CMakeLists.txt's comment on that).
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT DEFINED BUILD_GCC)
    message(WARNING
        "Configured with ${CMAKE_CXX_COMPILER_ID} (generator: ${CMAKE_GENERATOR}), "
        "not Clang, even though Clang is this project's intended default "
        "compiler. This almost always means either:\n"
        "  (a) this build directory's CMakeCache.txt already locked in a "
        "generator/compiler from an earlier configure - delete the build "
        "directory and reconfigure, or\n"
        "  (b) no -G was passed and CMake fell back to its own platform "
        "default (the Visual Studio generator on Windows), which drives "
        "cl.exe/MSVC regardless of CMAKE_CXX_COMPILER.\n"
        "To actually build with Clang, reconfigure from a clean build "
        "directory with the generator/compiler set explicitly, e.g.:\n"
        "  cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang\n"
        "or, to keep MSBuild/.vcxproj output but use Clang's toolset:\n"
        "  cmake -S . -B build -G \"Visual Studio 17 2022\" -T ClangCL\n"
        "Pass -DBUILD_GCC=ON instead if GCC is what you actually want, which "
        "silences this warning."
    )
endif()

# Set the C++ standard to C++23 as default for all targets
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Announce the C++ standard being used
message(STATUS "Using C++ standard: C++${CMAKE_CXX_STANDARD}")

# Define global log directory relative to KAI_HOME
set(KAI_HOME ${CMAKE_CURRENT_SOURCE_DIR})
set(KAI_LOG_DIR "${KAI_HOME}/Logs")
# Create the directory at configure time if it doesn't exist
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${KAI_LOG_DIR})
# Define as a compile-time definition using relative path (resolved at runtime)
add_definitions(-DKAI_LOG_DIR="./Logs")

# Ensure the global project uses folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable necessary options
option(KAI_BUILD_TEST_ALL "Build all tests" ON)
option(KAI_BUILD_CORE_TEST "Build unit tests" ON)
option(KAI_BUILD_TEST_LANG "Build language tests" ON)
option(KAI_BUILD_IMGUI "Build the ImGui client" OFF)
option(KAI_BUILD_WINDOW "Deprecated alias for KAI_BUILD_IMGUI" OFF)
option(KAI_BUILD_LLM "Build the local LLM support libraries" OFF)
option(KAI_NETWORKING "Build networking library, Tau IDL, and network tests" ON)

# Android target: build only the reusable libraries (Core, Language, Executor,
# Network), skipping desktop apps, ImGui, LLM, Tau and the NetworkGenerate tool.
# The KAI_ANDROID define is also exposed to C++.
option(KAI_ANDROID "Build the Android-reusable library subset" OFF)
if(KAI_ANDROID)
    add_definitions(-DKAI_ANDROID)
    # The static libs are linked into the shared libkai.so, so build them PIC.
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    message(STATUS "KAI_ANDROID: building the reusable library subset "
        "(CppKaiCore, CppKaiLanguage, Network); skipping ImGui, LLM, Tau, "
        "desktop apps, demos and tests")
    set(KAI_BUILD_IMGUI OFF CACHE BOOL "" FORCE)
    set(KAI_BUILD_WINDOW OFF CACHE BOOL "" FORCE)
    set(KAI_BUILD_LLM OFF CACHE BOOL "" FORCE)
    set(KAI_BUILD_TEST_ALL OFF CACHE BOOL "" FORCE)
    set(KAI_BUILD_CORE_TEST OFF CACHE BOOL "" FORCE)
    set(KAI_BUILD_TEST_LANG OFF CACHE BOOL "" FORCE)
endif()

if(KAI_BUILD_WINDOW AND NOT KAI_BUILD_IMGUI)
    message(STATUS "KAI_BUILD_WINDOW is deprecated; enabling KAI_BUILD_IMGUI")
    set(KAI_BUILD_IMGUI ON CACHE BOOL "Build the ImGui client" FORCE)
endif()

# Security option: Enable shell syntax (backtick operations)
#
# The backtick feature shells out to a POSIX shell (sh/echo/printf/grep/wc/
# cut/sed/tr/uname/date/expr, etc.) and captures its stdout. On Linux/macOS,
# and on any build actually done *inside* WSL2, this works natively: popen()
# already spawns a real /bin/sh there. On native Windows, Console.cpp routes
# the same command text through `wsl.exe -e bash -lc "..."` (see
# ToPlatformShellCommand in Console.cpp) instead of relying on cmd.exe, so
# the exact same backtick expressions behave identically on all three -
# the only extra requirement on native Windows is a working WSL2 distro
# with bash/coreutils installed and `wsl` on PATH. Override with
# -DENABLE_SHELL_SYNTAX=ON/OFF to force either behaviour on any platform.
option(ENABLE_SHELL_SYNTAX "Enable shell command execution via backtick syntax" OFF)

# Diagnostic-only: compiles in the KAI_TRACE()/KAI_TRACE_ERROR() calls
# scattered through Core/Debug.cpp and the Executor (Store/Suspend/EvalIdent/
# IfElse) which otherwise compile to no-ops. Off by default because it's
# very noisy and adds a Logger call to every traced instruction; note the
# calls still land in Logs/kai.log rather than the console unless
# Logger::s_consoleOutputForInfoDebug is also set at runtime (Info-level
# traces are file-only by design; only Warn/Error/Fatal print to stdout).
option(KAI_ENABLE_TRACE "Enable KAI_TRACE()/KAI_DEBUG_TRACE diagnostic logging" OFF)
if(KAI_ENABLE_TRACE)
    add_compile_definitions(KAI_DEBUG_TRACE)
endif()

# Boost has been fully removed from KAI. (CommonLang's optional arena allocator
# uses std::pmr, not boost.monotonic.)

# Set the CMake module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

# Set common paths for all other projects to use
set(HOME $ENV{HOME})
set(KAI_HOME ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCE_HOME ${KAI_HOME}/Source)
set(APP_HOME ${SOURCE_HOME}/App)
set(LIBRARY_HOME ${SOURCE_HOME}/Library)
set(TEST_HOME ${KAI_HOME}/Test)

# Support both in-source and out-of-source builds
if(NOT DEFINED BIN_HOME)
    # Always use KAI_HOME/Bin for output
    set(BIN_HOME ${KAI_HOME}/Bin)
endif()

message(STATUS "Binary output directory: ${BIN_HOME}")

# Set output directories - use BIN_HOME which was set above
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BIN_HOME}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}") 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}")

# Also set per-configuration output directories
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${BIN_HOME}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${BIN_HOME}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${BIN_HOME}")
endforeach()

include_directories(${KAI_HOME}/Include)
# Core/Executor/CommonLang headers now live in the CppKaiCore submodule, and
# the Pi/Rho languages in the CppKaiLanguage submodule.
include_directories(${KAI_HOME}/Ext/CppKaiCore/Include)
include_directories(${KAI_HOME}/Ext/CppKaiLanguage/Include)

# Link directories for external libraries
link_directories(${KAI_HOME}/Lib/Debug)
link_directories(/usr/local/lib)
# Don't use HOME/lib, it might not exist or have permission issues

# Modern C++23 compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Enable modern C++ features and warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
    
    # Disable specific warnings that are too noisy in legacy code
    if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-switch -Wno-comment -Wno-reorder")
endif()
    
    # Disable pedantic warnings for now as the codebase has many legacy issues
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-missing-field-initializers")
    
    # Disable additional warnings seen during build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-unused-value -Wno-unused-but-set-variable")
    
    # Add backward compatibility for the format library shim.
    add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
    
    # Add format-like behavior even in older compilers
    add_definitions(-DKAI_FORMAT_COMPATIBLE)
    
    # Compiler-specific optimizations and features
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # Clang optimizations
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
        
        # Clang-specific warnings that are useful
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument")
        
        # Enable colored diagnostics when using Ninja or Make
        if(CMAKE_GENERATOR STREQUAL "Ninja" OR CMAKE_GENERATOR STREQUAL "Unix Makefiles")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
        endif()
        
        # Modern C++ features - Clang has native concepts support.
        
        message(STATUS "Using Clang-specific optimizations and C++23 features")
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC-specific flags for modern C++ diagnostics.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts-diagnostics-depth=3")
        
        # GCC optimizations
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
        
        # Enable colored diagnostics when using Ninja or Make
        if(CMAKE_GENERATOR STREQUAL "Ninja" OR CMAKE_GENERATOR STREQUAL "Unix Makefiles")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
        endif()
        
        message(STATUS "Using GCC-specific optimizations and C++23 features")
    endif()
    
elseif(MSVC)
    # MSVC-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
    add_definitions(-DKAI_FORMAT_COMPATIBLE)
    message(STATUS "Using MSVC compiler with C++23-compatible mode")
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Add common defines
add_definitions(-DKAI_USE_NAMESPACES)
add_definitions(-DKAI_NAMESPACE_NAME=kai)
add_definitions(-DKAI_USE_EXCEPTIONS)
add_definitions(-DKAI_BOOST_UNORDERED_REGISTRY)
add_definitions(-DKAI_DEBUG)
# add_definitions(-DKAI_DEBUG_TRACE)
add_definitions(-DKAI_VERSION_MAJOR=0)
add_definitions(-DKAI_VERSION_MINOR=3)
add_definitions(-DKAI_VERSION_PATCH=0)

# Add shell syntax define if enabled
if(ENABLE_SHELL_SYNTAX)
    add_definitions(-DENABLE_SHELL_SYNTAX)
    if(WIN32)
        message(WARNING "Shell syntax (backtick operations) is ENABLED. Commands are routed through wsl.exe - a WSL2 distro with bash/coreutils must be installed and `wsl` must be on PATH, or backtick expressions will fail at runtime. This may pose security risks.")
    else()
        message(WARNING "Shell syntax (backtick operations) is ENABLED. This may pose security risks.")
    endif()
else()
    message(STATUS "Shell syntax (backtick operations) is DISABLED for security. Pass -DENABLE_SHELL_SYNTAX=ON to enable.")
endif()

# Add rang external library
set(EXTERNAL ${KAI_HOME}/Ext)

# Set up external dependencies like ENet and GoogleTest
if(KAI_NETWORKING)
    set(ENET_ROOT ${EXTERNAL}/ENet)
    include_directories(${ENET_ROOT}/include)
    add_subdirectory(${ENET_ROOT})
    add_definitions(-DKAI_USE_ENET)
    message(STATUS "Networking enabled: ENet, Network lib, Tau, and network tests will be built")
else()
    message(STATUS "Networking disabled (enable with -DKAI_NETWORKING=ON)")
endif()

if(KAI_BUILD_LLM)
    set(MODELSTORE_BUILD_TESTS OFF CACHE BOOL "Build ModelStore tests" FORCE)
    set(MODELSTORE_AUTO_ENSURE_MODELS OFF CACHE BOOL "Run ensure_models as part of the default build" FORCE)
    add_subdirectory(${EXTERNAL}/CppLmmModelStore)
    message(STATUS "LLM support enabled: CppLmmModelStore")
else()
    message(STATUS "LLM support disabled (enable with -DKAI_BUILD_LLM=ON)")
endif()

include_directories(${EXTERNAL}/rang/include)

# Macro to define a new library
macro(def_library name)
    project(${name})
    set(lib_root ${LIBRARY_HOME}/${name})
    set(kai_includes ${KAI_HOME}/Include/KAI/${name})
    include_directories(${kai_includes})
    set(sources ${lib_root}/Source/*.cpp)
    set(headers ${kai_includes}/*.h)
    file(GLOB_RECURSE srcs ${sources})
    file(GLOB_RECURSE hdrs ${headers})
    add_library(${name} ${srcs} ${hdrs})
    
    # Ensure output directory is correct
    set_target_properties(${name} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${BIN_HOME}"
        LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}"
    )
endmacro()

# Macro to define a new language library
macro(def_lang_library name)
    project(${name}Lang)
    set(lang_root ${LIBRARY_HOME}/Language/${name})
    set(kai_includes ${KAI_HOME}/Include/KAI/Language/${name})
    set(local_includes ${lang_root}/Include)
    include_directories(${kai_includes})
    include_directories(${local_includes})
    set(sources ${lang_root}/Source/*.cpp)
    set(headers ${kai_includes}/*.h ${local_includes}/*.h)
    file(GLOB_RECURSE src ${sources})
    file(GLOB_RECURSE hdrs ${headers})
    add_library(${name}Lang ${src} ${hdrs})
    
    # Ensure output directory is correct
    set_target_properties(${name}Lang PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${BIN_HOME}"
        LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}"
    )
endmacro()

# Macro to define a platform-specific library
macro(def_platform_library name)
    project(platform-${name})
    set(SOURCE_ROOT ${LIBRARY_HOME}/Platform/${name}/*.cpp)
    file(GLOB_RECURSE SOURCE_FILES ${SOURCE_ROOT})
    add_library(platform-${name} SHARED ${SOURCE_FILES})
    
    # Ensure output directory is correct
    set_target_properties(platform-${name} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${BIN_HOME}"
        LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}"
    )
endmacro()

# Loop through top-level directories and include their CMakeLists.txt
# Android builds only the libraries; desktop apps are skipped.
if(KAI_ANDROID)
    set(TopLevels Library)
else()
    set(TopLevels Library App)
endif()
foreach(top ${TopLevels})
    include(${SOURCE_HOME}/${top}/CMakeLists.txt)
endforeach()

# Standalone runnable demos (desktop only)
if(NOT KAI_ANDROID)
    add_subdirectory(Demo/ContinuationMobilityDemo)
endif()

# Set up main library dependencies
if(TARGET Executor)
    target_link_libraries(Executor PUBLIC CommonLang Core)
    if(TARGET Network)
        target_link_libraries(Executor PUBLIC Network)
    endif()
    if(KAI_NETWORKING AND TARGET enet)
        target_link_libraries(Executor PUBLIC enet)
    endif()
endif()

# CommonLang -> Core is declared inside the CppKaiCore submodule.

if(TARGET Network)
    target_link_libraries(Network PUBLIC Core)
    if(KAI_NETWORKING AND TARGET enet)
        target_link_libraries(Network PUBLIC enet)
    endif()
endif()

# PiLang / RhoLang link dependencies are declared inside CppKaiLanguage.

if(TARGET TauLang)
    target_link_libraries(TauLang Core CommonLang)
endif()

# Android: aggregate the runtime into a single loadable libkai.so with a JNI
# entry point, so an app can System.loadLibrary("kai"). See Android/ and
# Doc/Android.md. (Built only for the KAI_ANDROID subset.)
if(KAI_ANDROID)
    add_library(kai SHARED ${KAI_HOME}/Android/jni/Kai_jni.cpp)
    target_link_libraries(kai PRIVATE
        ConsoleLib RhoLang PiLang Executor CommonLang Core)
    if(TARGET Network)
        target_link_libraries(kai PRIVATE Network)
    endif()
    if(TARGET enet)
        target_link_libraries(kai PRIVATE enet)
    endif()
    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
        # jni.h comes from the NDK sysroot; link the Android system libraries.
        target_link_libraries(kai PRIVATE log android)
    else()
        # Host link-sanity build: pull JNI headers from an installed JDK.
        find_package(JNI REQUIRED)
        target_include_directories(kai PRIVATE ${JNI_INCLUDE_DIRS})
    endif()
    set_target_properties(kai PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}")
endif()

# Find or download Google Test if needed for tests
if(NOT KAI_ANDROID AND (KAI_BUILD_TEST_ALL OR KAI_BUILD_CORE_TEST OR KAI_BUILD_TEST_LANG OR KAI_NETWORKING))
  find_package(GTest QUIET)
  if(NOT GTEST_FOUND)
    message(STATUS "GTest not found - fetching via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG        v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    set(GTEST_INCLUDE_DIRS "${googletest_SOURCE_DIR}/googletest/include")
    set(GTEST_BOTH_LIBRARIES gtest gtest_main)
    set(GTEST_LIBRARIES gtest)
    set(GTEST_MAIN_LIBRARIES gtest_main)
  else()
    message(STATUS "Using system GTest")
  endif()
  include_directories(${GTEST_INCLUDE_DIRS})
endif()
  

# Add tests only when explicitly requested or when networking tests are enabled.
if(NOT KAI_ANDROID AND (KAI_BUILD_TEST_ALL OR KAI_BUILD_CORE_TEST OR KAI_BUILD_TEST_LANG OR KAI_NETWORKING))
    # enable_testing() must be called here (an ancestor of every Test/*
    # subdirectory) rather than down in Test/Window/CMakeLists.txt where it
    # used to live alone - CTestTestfile.cmake generation only threads
    # subdirs() down from the directory where testing was enabled, so a
    # leaf-only call left every other test directory invisible to `ctest`,
    # which is why `ctest --test-dir build-clang` reported "No tests were
    # found!!!" even though the test executables built and linked fine.
    enable_testing()
    add_subdirectory(Test)
endif()
