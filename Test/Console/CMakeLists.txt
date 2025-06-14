cmake_minimum_required(VERSION 3.6)

set(name TestConsole)
project(${name})

set(source_path ${CMAKE_CURRENT_SOURCE_DIR})
set(header_path ${TEST_HOME}/Include)

# Console test files
set(sources
    ${source_path}/TestConsoleZshFeatures.cpp
)

# Include test headers
include_directories(${header_path})

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}/Test")

# Create test executable
add_executable(${name} ${sources})

# Link with required libraries
target_link_libraries(${name} 
    CommonLang
    Executor 
    Core 
    ${GTEST_BOTH_LIBRARIES} 
    gtest_main
)

# Add test
add_test(NAME ${name} COMMAND ${name})