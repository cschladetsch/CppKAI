cmake_minimum_required(VERSION 3.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}/Bin")

set(apps Console)

if(KAI_BUILD_IMGUI)
    list(APPEND apps ImGui)
endif()

if(KAI_BUILD_LLM)
    list(APPEND apps RepoIndex RhoDataset)
endif()

foreach (app ${apps})
    if(app STREQUAL ImGui)
        include(${APP_HOME}/Window/CMakeLists.txt)
    else()
        include(${APP_HOME}/${app}/CMakeLists.txt)
    endif()
endforeach()

if(KAI_NETWORKING)
    include(${APP_HOME}/NetworkGenerate/CMakeLists.txt)
endif()
