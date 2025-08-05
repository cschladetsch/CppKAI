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
# - Defaults to C++20 standard with modern C++ features
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
# Define as a compile-time definition with proper escaping for C++20
add_definitions(-DKAI_LOG_DIR="${KAI_LOG_DIR}")

# Ensure the global project uses folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable necessary options
option(KAI_BUILD_TEST_ALL "Build all tests" ON)
option(KAI_BUILD_CORE_TEST "Build unit tests" ON)
option(KAI_BUILD_TEST_LANG "Build language tests" ON)
option(KAI_BUILD_TEST_NETWORK "Build networking tests" OFF)
option(KAI_BUILD_RAKNET "Build RakNet" OFF)
option(KAI_BUILD_NETWORK_APPS "Build network applications" OFF)

# Security option: Enable shell syntax (backtick operations)
# Default to OFF for security reasons
option(ENABLE_SHELL_SYNTAX "Enable shell command execution via backtick syntax" OFF)

# Find and link Boost libraries
find_package(Boost REQUIRED COMPONENTS system filesystem program_options date_time regex)
include_directories(${Boost_INCLUDE_DIRS})

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

# Link directories for external libraries
link_directories(${KAI_HOME}/Lib/Debug)
link_directories(/usr/local/lib)
# Don't use HOME/lib, it might not exist or have permission issues

# Modern C++20 compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Enable modern C++ features and warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
    
    # Disable specific warnings that are too noisy in legacy code
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-switch -Wno-comment -Wno-reorder")
    
    # Disable pedantic warnings for now as the codebase has many legacy issues
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-missing-field-initializers")
    
    # Disable additional warnings seen during build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-unused-value -Wno-unused-but-set-variable")
    
    # Add backward compatibility for format library - use C++20 std::format
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
        
        # Modern C++20 features - Clang has native C++20 support
        # No need for -fconcepts flags as they're built into C++20 support
        
        message(STATUS "Using Clang-specific optimizations and C++20 features")
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC-specific flags for C++20
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts-diagnostics-depth=3")
        
        # GCC optimizations
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
        
        # Enable colored diagnostics when using Ninja or Make
        if(CMAKE_GENERATOR STREQUAL "Ninja" OR CMAKE_GENERATOR STREQUAL "Unix Makefiles")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
        endif()
        
        message(STATUS "Using GCC-specific optimizations and C++20 features")
    endif()
    
elseif(MSVC)
    # MSVC-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
    add_definitions(-DKAI_FORMAT_COMPATIBLE)
    message(STATUS "Using MSVC compiler with C++20")
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Add common defines
add_definitions(-DKAI_USE_NAMESPACES)
add_definitions(-DKAI_NAMESPACE_NAME=kai)
add_definitions(-DKAI_USE_EXCEPTIONS)
add_definitions(-DKAI_BOOST_UNORDERED_REGISTRY)
add_definitions(-DKAI_DEBUG)
add_definitions(-DKAI_DEBUG_TRACE)
add_definitions(-DKAI_VERSION_MAJOR=0)
add_definitions(-DKAI_VERSION_MINOR=3)
add_definitions(-DKAI_VERSION_PATCH=0)

# Add shell syntax define if enabled
if(ENABLE_SHELL_SYNTAX)
    add_definitions(-DENABLE_SHELL_SYNTAX)
    message(WARNING "Shell syntax (backtick operations) is ENABLED. This may pose security risks.")
else()
    message(STATUS "Shell syntax (backtick operations) is DISABLED for security.")
endif()

# Set up external dependencies like Boost, RakNet, and GoogleTest
if (KAI_BUILD_RAKNET)
    set(RAKNET_ROOT ${EXTERNAL}/raknet)
    include_directories(${RAKNET_ROOT})
    # Disable crypto in RakNet due to OpenSSL compatibility issues
    add_definitions(-DSLIBCFG_DISABLE_SSL)
    add_definitions(-DSLIBCFG_DISABLE_CRYPTOGRAPHY)
    # Skip building RakNet for now, we'll use our stub implementation
    # add_subdirectory(${RAKNET_ROOT})
    add_definitions(-DKAI_USE_RAKNET)
endif()

# Only use RakNetStub implementation when KAI_BUILD_RAKNET is OFF
if (NOT KAI_BUILD_RAKNET)
    add_definitions(-DKAI_USE_RAKNET_STUB)
endif()

# Add rang external library
set(EXTERNAL ${KAI_HOME}/Ext)
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
set(TopLevels Library App)
foreach(top ${TopLevels})
    include(${SOURCE_HOME}/${top}/CMakeLists.txt)
endforeach()

# Set up main library dependencies
if(TARGET Executor)
    target_link_libraries(Executor CommonLang Core)
endif()

if(TARGET CommonLang)
    target_link_libraries(CommonLang Core)
endif()

if(TARGET Network)
    target_link_libraries(Network Core)
endif()

if(TARGET PiLang)
    target_link_libraries(PiLang Core CommonLang)
endif()

if(TARGET RhoLang)
    target_link_libraries(RhoLang Core CommonLang PiLang)
endif()

if(TARGET TauLang)
    target_link_libraries(TauLang Core CommonLang)
endif()

# Find or download Google Test if needed for tests
if(KAI_BUILD_TEST_ALL OR KAI_BUILD_CORE_TEST OR KAI_BUILD_TEST_LANG)
  # First try to find the system's gtest
  find_package(GTest QUIET)
  
  if(NOT GTEST_FOUND)
    message(STATUS "GTest not found in system, downloading it...")
    
    # Download and unpack googletest at configure time
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
    execute_process(COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

    # Prevent overriding compiler/linker options 
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        
    # Add googletest directly to our build
    add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
                     "${CMAKE_BINARY_DIR}/googletest-build"
                     EXCLUDE_FROM_ALL)
                     
    # Define needed variables
    set(GTEST_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/googletest-src/googletest/include")
    set(GTEST_BOTH_LIBRARIES gtest gtest_main)
    set(GTEST_LIBRARIES gtest)
    set(GTEST_MAIN_LIBRARIES gtest_main)
  else()
    message(STATUS "Using system GTest")
  endif()
  
  # Always add the include directories for tests
  include_directories(${GTEST_INCLUDE_DIRS})
endif()

# Add tests if requested
add_subdirectory(Test)
