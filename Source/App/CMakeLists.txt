cmake_minimum_required(VERSION 3.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}/Bin")

set(apps Console) # Window NetworkTest

# Only include NetworkGenerate if RakNet is enabled
if (KAI_BUILD_RAKNET)
    list(APPEND apps NetworkGenerate)
endif()

foreach (app ${apps})
    include(${APP_HOME}/${app}/CMakeLists.txt)
endforeach()

