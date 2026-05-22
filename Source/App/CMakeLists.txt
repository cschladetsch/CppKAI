cmake_minimum_required(VERSION 3.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}/Bin")

set(apps Console)

if(KAI_BUILD_WINDOW)
    list(APPEND apps Window)
endif()

if(KAI_BUILD_LLM)
    list(APPEND apps RepoIndex)
endif()

foreach (app ${apps})
    include(${APP_HOME}/${app}/CMakeLists.txt)
endforeach()

if(KAI_NETWORKING)
    include(${APP_HOME}/NetworkGenerate/CMakeLists.txt)
endif()
