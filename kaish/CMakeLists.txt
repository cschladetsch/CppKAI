cmake_minimum_required(VERSION 3.20)
project(kaish CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# kaish builds standalone (ps mode only) or inside CppKAI, where it links KAI's
# Console/Executor so pi and rho share the stack with the shell.
add_library(kaish_core
    src/syntax.cpp
    src/shell.cpp
    src/builtins.cpp
    src/platform.cpp
    src/config.cpp
    src/text_backend.cpp
)
target_include_directories(kaish_core PUBLIC include)
if(WIN32)
    target_compile_definitions(kaish_core PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(kaish_core PUBLIC shell32)
endif()
if(MSVC)
    target_compile_options(kaish_core PUBLIC /utf-8)
endif()

add_executable(kaish src/main.cpp)
target_link_libraries(kaish PRIVATE kaish_core)

if(TARGET ConsoleLib)
    message(STATUS "kaish: linking KAI (pi and rho modes enabled)")
    target_sources(kaish PRIVATE src/kai_backend.cpp)
    target_compile_definitions(kaish PRIVATE KAISH_WITH_KAI)
    target_link_libraries(kaish PRIVATE ConsoleLib PiLang RhoLang Executor CommonLang Core)
    if(WIN32)
        # Same reason as the KAI Console: deep Pi recursion needs more than 1MB.
        target_link_options(kaish PRIVATE -Xlinker /STACK:16777216)
    endif()
else()
    message(STATUS "kaish: KAI not found, building ps mode only")
endif()

add_executable(kaish_tests tests/test_kaish.cpp)
target_link_libraries(kaish_tests PRIVATE kaish_core)

# Inside CppKAI, put the binaries in Bin/ next to Console. Set explicitly because
# Source/App/CMakeLists.txt leaves CMAKE_RUNTIME_OUTPUT_DIRECTORY as "${dir}/Bin".
if(DEFINED BIN_HOME)
    set_target_properties(kaish kaish_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}")
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} config)
        set_target_properties(kaish kaish_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${config} "${BIN_HOME}")
    endforeach()
endif()

enable_testing()
add_test(NAME KaiShUnitTests COMMAND kaish_tests)
