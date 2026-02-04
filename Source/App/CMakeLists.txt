cmake_minimum_required(VERSION 3.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${dir}/Bin")

set(apps Console) # Window temporarily disabled due to GLEW dependency

foreach (app ${apps})
    include(${APP_HOME}/${app}/CMakeLists.txt)
endforeach()
