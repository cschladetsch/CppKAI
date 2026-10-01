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

if(KAI_NETWORKING)
    def_library(Network)
    add_subdirectory(${LIBRARY_HOME}/Network)
endif()

# language libs are a bit different
include_directories(${LIBRARY_HOME}/Language/Include)
add_subdirectory(${LIBRARY_HOME}/Language)

# ConsoleLib comes from the CppKaiConsoleLib submodule. It sits above Executor
# and the language libraries, so it is added after them.
add_subdirectory(${KAI_HOME}/Ext/CppKaiConsoleLib ${CMAKE_BINARY_DIR}/CppKaiConsoleLib)
