cmake_minimum_required(VERSION 3.28...3.30)

# Core, CommonLang and Executor come from the CppKaiCore submodule. They form a
# dependency cycle and are reused on other platforms (including Android), so
# they live together in one module rather than as separate libraries here.
add_subdirectory(${KAI_HOME}/Ext/CppKaiCore ${CMAKE_BINARY_DIR}/CppKaiCore)

# ImGui core lib (desktop only; the GL/GLFW frontend lives in App/Window).
if(NOT KAI_ANDROID)
    add_subdirectory(${LIBRARY_HOME}/ImGui)
endif()

if(KAI_BUILD_LLM)
    def_library(LLM)
    target_link_libraries(LLM PUBLIC ModelStore)
endif()

# Console.cpp ships in the CppKaiCore submodule's Executor sources but is glue
# that depends on the Pi/Rho translators, so the submodule excludes it from its
# Executor library and it is compiled into ConsoleLib (below) instead.
set(kai_console_src
    ${KAI_HOME}/Ext/CppKaiCore/Source/Library/Executor/Source/Console.cpp)

if(KAI_NETWORKING)
    def_library(Network)
    add_subdirectory(${LIBRARY_HOME}/Network)
endif()

# language libs are a bit different
include_directories(${LIBRARY_HOME}/Language/Include)
add_subdirectory(${LIBRARY_HOME}/Language)

# ConsoleLib: Console.cpp sits above Executor and language libs so it can
# include language translators directly without circular dependencies.
project(ConsoleLib)
add_library(ConsoleLib ${kai_console_src})
# Pi/Rho headers reach Console.cpp via the linked PiLang/RhoLang targets (their
# include dirs are PUBLIC) and the global CppKaiLanguage include path.
target_link_libraries(ConsoleLib PUBLIC Executor PiLang RhoLang CommonLang Core)
set_target_properties(ConsoleLib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${BIN_HOME}"
    LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}"
)
