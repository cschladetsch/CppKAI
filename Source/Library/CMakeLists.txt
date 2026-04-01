cmake_minimum_required(VERSION 3.28...3.30)

set(libs Core Executor)

foreach(lib ${libs})
    def_library(${lib})
endforeach()

if(KAI_NETWORKING)
    def_library(Network)
    add_subdirectory(${LIBRARY_HOME}/Network)
endif()

# language libs are a bit different
include_directories(${LIBRARY_HOME}/Language/Include)
add_subdirectory(${LIBRARY_HOME}/Language)

# ImGui library disabled due to missing external dependencies
