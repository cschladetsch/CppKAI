cmake_minimum_required(VERSION 3.6)

# Project for testing proxy generation
project(Test_ProxyGeneration)

# Source files
set(SOURCES
    TestGenerateProxy.cpp
)

# Create the test executable
add_executable(Test_ProxyGeneration ${SOURCES})

# Add include directories
target_include_directories(Test_ProxyGeneration PRIVATE
    ${KAI_HOME}/Include
    ${TEST_HOME}/Include
    ${GTEST_INCLUDE_DIRS}
)

# Set compile flags
set_target_properties(Test_ProxyGeneration PROPERTIES COMPILE_FLAGS "-g -Wno-deprecated")

# Link libraries - include only the libraries needed for proxy generation testing
target_link_libraries(Test_ProxyGeneration 
    Network
    Core
    ${GTEST_BOTH_LIBRARIES}
    ${Boost_LIBRARIES}
    gtest_main
    pthread
)

# Set output directory
set_target_properties(Test_ProxyGeneration PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}/Test"
)

# Add the test to CTest
add_test(NAME Test_ProxyGeneration COMMAND Test_ProxyGeneration)