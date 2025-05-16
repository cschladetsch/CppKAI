cmake_minimum_required(VERSION 3.10)

# Create a test executable
add_executable(LogTest main.cpp)

# Include the necessary directories
target_include_directories(LogTest PUBLIC
    ${CMAKE_SOURCE_DIR}/Include
)

# Link against the required libraries
target_link_libraries(LogTest
    Core
    Network
)

# Add the executable to the build
add_test(NAME LogTest COMMAND LogTest)