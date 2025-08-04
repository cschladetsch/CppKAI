cmake_minimum_required(VERSION 3.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}/Bin")

set(apps Console) # Window temporarily disabled due to GLEW dependency

# Only include Network apps if RakNet is enabled
if (KAI_BUILD_RAKNET)
    list(APPEND apps NetworkTest NetworkGenerate)
endif()

foreach (app ${apps})
    include(${APP_HOME}/${app}/CMakeLists.txt)
endforeach()

