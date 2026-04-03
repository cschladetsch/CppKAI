cmake_minimum_required(VERSION 3.15)

# Build the do-while test executable
add_executable(DoWhileTestBasic DoWhileTestBasic.cpp)
target_link_libraries(DoWhileTestBasic KAICore)
set_target_properties(DoWhileTestBasic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin/Test"
)
target_include_directories(DoWhileTestBasic PRIVATE ${CMAKE_SOURCE_DIR}/Include)