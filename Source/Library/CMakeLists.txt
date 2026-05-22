cmake_minimum_required(VERSION 3.28...3.30)

def_library(Core)

add_subdirectory(${LIBRARY_HOME}/ImGui)

# Executor: build all sources except Console.cpp, which needs language deps
project(Executor)
set(exec_root ${LIBRARY_HOME}/Executor)
set(kai_exec_includes ${KAI_HOME}/Include/KAI/Executor)
include_directories(${kai_exec_includes})
file(GLOB_RECURSE exec_srcs ${exec_root}/Source/*.cpp)
list(FILTER exec_srcs EXCLUDE REGEX ".*Console\\.cpp$")
add_library(Executor ${exec_srcs})
set_target_properties(Executor PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${BIN_HOME}"
    LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}"
)

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
add_library(ConsoleLib ${exec_root}/Source/Console.cpp)
target_include_directories(ConsoleLib PUBLIC
    ${kai_exec_includes}
    ${KAI_HOME}/Include/KAI/Language/Rho
    ${KAI_HOME}/Include/KAI/Language/Pi
    ${KAI_HOME}/Include/KAI/Language/Common
    ${LIBRARY_HOME}/Language/Rho/Include
    ${LIBRARY_HOME}/Language/Pi/Include
)
target_link_libraries(ConsoleLib PUBLIC Executor PiLang RhoLang CommonLang Core)
set_target_properties(ConsoleLib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${BIN_HOME}"
    LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}"
)
