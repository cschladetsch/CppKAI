cmake_minimum_required(VERSION 3.28...3.30)

project(KAI)

# Set the compiler to GCC
#set(CMAKE_C_COMPILER /usr/bin/gcc)
#set(CMAKE_CXX_COMPILER /usr/bin/g++)

# Set the C++ standard
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure the global project uses folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable necessary options
option(KAI_BUILD_TEST_ALL "Build all tests" ON)
option(KAI_BUILD_CORE_TEST "Build unit tests" ON)
option(KAI_BUILD_TEST_LANG "Build language tests" ON)
option(KAI_BUILD_TEST_NETWORK "Build networking tests" OFF)
option(KAI_BUILD_RAKNET "Build RakNet" ON)

# Find and link Boost libraries
find_package(Boost REQUIRED COMPONENTS system filesystem program_options date_time regex)
include_directories(${Boost_INCLUDE_DIRS})

# Set the CMake module path
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

# Set common paths for all other projects to use
set(HOME $ENV{HOME})
set(KAI_HOME ${CMAKE_SOURCE_DIR})
set(SOURCE_HOME ${KAI_HOME}/Source)
set(APP_HOME ${SOURCE_HOME}/App)
set(LIBRARY_HOME ${SOURCE_HOME}/Library)
set(TEST_HOME ${KAI_HOME}/Test)
set(BIN_HOME ${KAI_HOME}/Bin)
set(INCLUDE_HOME ${KAI_HOME}/Include)
set(EXTERNAL ${KAI_HOME}/Ext)
set(dir ${KAI_HOME})

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${dir}/Lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${dir}/Lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}/Bin")

include_directories(${KAI_HOME}/Include)

# Link directories for external libraries
link_directories(${HOME}/lib /usr/local/lib)
link_directories(${KAI_HOME}/Lib/Debug)

# Compiler warnings to ignore
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wno-deprecated -Wno-switch")

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

# Always use RakNetStub implementation to avoid compilation issues
add_definitions(-DKAI_USE_RAKNET_STUB)

# Add rang external library
include_directories(${EXTERNAL}/rang/include)

# Macro to define a new library
macro(def_library name)
    project(${name})
    set(lib_root ${LIBRARY_HOME}/${name})
    set(kai_includes ${INCLUDE_HOME}/KAI/${name})
    include_directories(${kai_includes})
    set(sources ${lib_root}/Source/*.cpp)
    set(headers ${kai_includes}/*.h)
    file(GLOB_RECURSE srcs ${sources})
    file(GLOB_RECURSE hdrs ${headers})
    add_library(${name} ${srcs} ${hdrs})
endmacro()

# Macro to define a new language library
macro(def_lang_library name)
    project(${name}Lang)
    set(lang_root ${LIBRARY_HOME}/Language/${name})
    set(kai_includes ${INCLUDE_HOME}/KAI/Language/${name})
    set(local_includes ${lang_root}/Include)
    include_directories(${kai_includes})
    include_directories(${local_includes})
    set(sources ${lang_root}/Source/*.cpp)
    set(headers ${kai_includes}/*.h ${local_includes}/*.h)
    file(GLOB_RECURSE src ${sources})
    file(GLOB_RECURSE hdrs ${headers})
    add_library(${name}Lang ${src} ${hdrs})
endmacro()

# Macro to define a platform-specific library
macro(def_platform_library name)
    project(platform-${name})
    set(SOURCE_ROOT ${LIBRARY_HOME}/Platform/${name}/*.cpp)
    file(GLOB_RECURSE SOURCE_FILES ${SOURCE_ROOT})
    add_library(platform-${name} SHARED ${SOURCE_FILES})
endmacro()

# Loop through top-level directories and include their CMakeLists.txt
set(TopLevels Library App)
foreach(top ${TopLevels})
    include(${SOURCE_HOME}/${top}/CMakeLists.txt)
endforeach()

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
