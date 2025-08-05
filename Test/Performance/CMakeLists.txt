cmake_minimum_required(VERSION 3.28...3.30)

# Performance benchmark tests
project(PerformanceTests)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin/Test")

# Performance test sources
set(PERF_SOURCES
    SimplifiedPerformanceTests.cpp
)

# Create the performance test executable
add_executable(PerformanceTests ${PERF_SOURCES})

# Set the runtime output directory for the test executable
set_target_properties(PerformanceTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin/Test"
)

# Link libraries - include all language libraries for comprehensive testing
target_link_libraries(PerformanceTests 
    TauLang
    RhoLang 
    PiLang
    Executor
    Core 
    ${GTEST_BOTH_LIBRARIES}
    ${Boost_LIBRARIES}
    pthread
)

# Add include directories
target_include_directories(PerformanceTests PRIVATE
    ${KAI_HOME}/Include
    ${TEST_HOME}/Include
    ${GTEST_INCLUDE_DIRS}
)

# Set compile flags for optimal performance testing
set_target_properties(PerformanceTests PROPERTIES 
    COMPILE_FLAGS "-g -O2 -Wno-deprecated"
)

# Add the test to CTest
add_test(NAME PerformanceTests COMMAND PerformanceTests)

# Set test properties for longer timeout (performance tests can take time)
set_tests_properties(PerformanceTests PROPERTIES TIMEOUT 300)