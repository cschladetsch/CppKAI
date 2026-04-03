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

# Set the output directory explicitly for this target
set_target_properties(LogTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin/Test"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Bin/Test"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Bin/Test"
)

# Add the executable to the build
add_test(NAME LogTest COMMAND LogTest)