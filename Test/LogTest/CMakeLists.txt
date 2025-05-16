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
    Executor
)

# Define KAI_DEBUG_TRACE to enable trace macros
target_compile_definitions(LogTest PRIVATE KAI_DEBUG_TRACE)

# Add the executable to the build
add_test(NAME LogTest COMMAND LogTest)