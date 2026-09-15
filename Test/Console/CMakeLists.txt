cmake_minimum_required(VERSION 3.28...3.30)

set(name TestConsole)
project(${name})

set(source_path ${CMAKE_CURRENT_SOURCE_DIR})
set(header_path ${TEST_HOME}/Include)

# Console test files
set(sources
    ${source_path}/ConsoleRegressionTests.cpp
    ${source_path}/TestConsoleNetworking.cpp
    ${TEST_HOME}/Language/TestLangCommon.cpp
    ${TEST_HOME}/Common/TestCommon.cpp
    ${TEST_HOME}/Common/MyTestStruct.cpp
)

# Include test headers
include_directories(${header_path} ${TEST_HOME}/Language)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}/Test")

# Create test executable
add_executable(${name} ${sources})

# Link with required libraries
target_link_libraries(${name}
    ConsoleLib
    CommonLang
    Executor
    Core
    PiLang
    RhoLang
    ${GTEST_BOTH_LIBRARIES}
)

# Add test
add_test(NAME ${name} COMMAND ${name})
