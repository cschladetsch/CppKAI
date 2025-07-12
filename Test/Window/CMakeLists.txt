cmake_minimum_required(VERSION 3.10)

project(TestWindow)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(GTest REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Source/App/Window/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Ext/imgui
    ${GTEST_INCLUDE_DIRS}
)

# Add library directory
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Bin)

# Simple mock test executable (no KAI dependencies)
add_executable(TestSimpleMock
    TestSimpleMock.cpp
)

# Set output directory for TestSimpleMock
set_target_properties(TestSimpleMock PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(TestSimpleMock
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)

# Window test executable
add_executable(TestWindow
    TestExecutorWindow.cpp
    TestWindowUI.cpp
    TestWindowPerformance.cpp
)

# Set output directory for TestWindow
set_target_properties(TestWindow PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin/Test
)

# Link libraries (using the same order as other tests)
target_link_libraries(TestWindow
    PiLang
    RhoLang
    TauLang
    Executor
    CommonLang
    Core
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)

# Add test
add_test(NAME TestWindow COMMAND TestWindow)

# Enable testing
enable_testing()